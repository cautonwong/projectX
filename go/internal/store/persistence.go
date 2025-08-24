package store

import (
	"database/sql"
	"fmt"
	"log/slog"
	"os"
	"path/filepath"
	"strings"
	"time"

	_ "github.com/mattn/go-sqlite3"
)

type Persistence struct {
	db         *sql.DB
	WriteQueue chan SensorReading
}

func NewPersistence(dbpath string) (*Persistence, error) {
	dir := filepath.Dir(dbpath)
	if err := os.MkdirAll(dir, 0755); err != nil {
		return nil, fmt.Errorf("failed to create database directory %s: %w", dir, err)
	}
	db, err := sql.Open("sqlite3", fmt.Sprintf("%s?_journal_mode=WAL", dbpath))
	if err != nil {
		return nil, err
	}

	createTableSQL := `
    CREATE TABLE IF NOT EXISTS devices (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL UNIQUE,
        description TEXT,
        topic_prefix TEXT NOT NULL UNIQUE,
        created_at DATETIME NOT NULL
    );
    CREATE TABLE IF NOT EXISTS device_parameters (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        device_id INTEGER NOT NULL,
        name TEXT NOT NULL,
        value TEXT NOT NULL,
        attributes TEXT,
        created_at DATETIME NOT NULL,
        updated_at DATETIME NOT NULL,
        FOREIGN KEY (device_id) REFERENCES devices (id) ON DELETE CASCADE
    );
    CREATE TABLE IF NOT EXISTS sensor_readings (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        topic TEXT NOT NULL,
        value REAL NOT NULL,
        unit TEXT,
        ts DATETIME NOT NULL
    );
    CREATE INDEX IF NOT EXISTS idx_topic_ts ON sensor_readings (topic, ts);
    `
	if _, err := db.Exec(createTableSQL); err != nil {
		return nil, err
	}

	p := &Persistence{
		db:         db,
		WriteQueue: make(chan SensorReading, 200),
	}

	go p.writer()
	return p, nil
}

func (p *Persistence) writer() {
	ticker := time.NewTicker(2 * time.Second)
	defer ticker.Stop()

	batch := make([]SensorReading, 0, 100)

	for {
		select {
		case reading, ok := <-p.WriteQueue:
			if !ok {
				if len(batch) > 0 {
					p.flushBatch(batch)
				}
				slog.Info("DB writer shutting down.")
				return
			}
			batch = append(batch, reading)
			if len(batch) >= 100 {
				p.flushBatch(batch)
				batch = make([]SensorReading, 0, 100)
			}
		case <-ticker.C:
			if len(batch) > 0 {
				p.flushBatch(batch)
				batch = make([]SensorReading, 0, 100)
			}
		}
	}
}

func (p *Persistence) flushBatch(batch []SensorReading) {
	if len(batch) == 0 {
		return
	}

	tx, err := p.db.Begin()
	if err != nil {
		slog.Error("failed to begin transaction", "error", err)
		return
	}

	stmt, err := tx.Prepare("INSERT INTO sensor_readings (topic, value, unit, ts) VALUES (?, ?, ?, ?)")
	if err != nil {
		slog.Error("failed to prepare statement", "error", err)
		tx.Rollback()
		return
	}
	defer stmt.Close()

	for _, reading := range batch {
		_, err := stmt.Exec(reading.Topic, reading.Value, reading.Unit, reading.Timestamp)
		if err != nil {
			slog.Error("failed to execute statement in batch", "error", err, "reading", reading)
			tx.Rollback()
			return
		}
	}

	if err := tx.Commit(); err != nil {
		slog.Error("failed to commit transaction", "error", err)
	} else {
		slog.Info("Successfully wrote batch to database", "count", len(batch))
	}
}

func (p *Persistence) QueryReadings(topic string, limit int, start, end time.Time) ([]SensorReading, error) {
	var queryBuilder strings.Builder
	args := make([]interface{}, 0)

	queryBuilder.WriteString("SELECT topic, value, unit, ts FROM sensor_readings WHERE topic = ?")
	args = append(args, topic)

	if !start.IsZero() {
		queryBuilder.WriteString(" AND ts >= ?")
		args = append(args, start)
	}
	if !end.IsZero() {
		queryBuilder.WriteString(" AND ts <= ?")
		args = append(args, end)
	}

	queryBuilder.WriteString(" ORDER BY ts DESC LIMIT ?")
	args = append(args, limit)

	rows, err := p.db.Query(queryBuilder.String(), args...)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var results []SensorReading
	for rows.Next() {
		var r SensorReading
		if err := rows.Scan(&r.Topic, &r.Value, &r.Unit, &r.Timestamp); err != nil {
			return nil, err
		}
		results = append(results, r)
	}
	return results, nil
}

func (p *Persistence) Close() {
	close(p.WriteQueue)
	p.db.Close()
}

func (p *Persistence) CreateDevice(device *Device) error {
	stmt, err := p.db.Prepare("INSERT INTO devices (name, description, topic_prefix, created_at) VALUES (?, ?, ?, ?)")
	if err != nil {
		return err
	}
	defer stmt.Close()

	result, err := stmt.Exec(device.Name, device.Description, device.TopicPrefix, time.Now())
	if err != nil {
		return err
	}

	id, err := result.LastInsertId()
	if err != nil {
		return err
	}
	device.ID = int(id)
	return nil
}

func (p *Persistence) GetDevice(id int) (*Device, error) {
	row := p.db.QueryRow("SELECT id, name, description, topic_prefix, created_at FROM devices WHERE id = ?", id)
	device := &Device{}
	err := row.Scan(&device.ID, &device.Name, &device.Description, &device.TopicPrefix, &device.CreatedAt)
	if err != nil {
		return nil, err
	}
	return device, nil
}

func (p *Persistence) GetDevices() ([]Device, error) {
	rows, err := p.db.Query("SELECT id, name, description, topic_prefix, created_at FROM devices")
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var devices []Device
	for rows.Next() {
		device := Device{}
		err := rows.Scan(&device.ID, &device.Name, &device.Description, &device.TopicPrefix, &device.CreatedAt)
		if err != nil {
			return nil, err
		}
		devices = append(devices, device)
	}
	return devices, nil
}

func (p *Persistence) UpdateDevice(device *Device) error {
	stmt, err := p.db.Prepare("UPDATE devices SET name = ?, description = ?, topic_prefix = ? WHERE id = ?")
	if err != nil {
		return err
	}
	defer stmt.Close()

	_, err = stmt.Exec(device.Name, device.Description, device.TopicPrefix, device.ID)
	return err
}

func (p *Persistence) DeleteDevice(id int) error {
	stmt, err := p.db.Prepare("DELETE FROM devices WHERE id = ?")
	if err != nil {
		return err
	}
	defer stmt.Close()

	_, err = stmt.Exec(id)
	return err
}

func (p *Persistence) CreateDeviceParameter(parameter *DeviceParameter) error {
	stmt, err := p.db.Prepare("INSERT INTO device_parameters (device_id, name, value, attributes, created_at, updated_at) VALUES (?, ?, ?, ?, ?, ?)")
	if err != nil {
		return err
	}
	defer stmt.Close()

	result, err := stmt.Exec(parameter.DeviceID, parameter.Name, parameter.Value, parameter.Attributes, time.Now(), time.Now())
	if err != nil {
		return err
	}

	id, err := result.LastInsertId()
	if err != nil {
		return err
	}
	parameter.ID = int(id)
	return nil
}

func (p *Persistence) GetDeviceParameter(deviceId int, name string) (*DeviceParameter, error) {
	row := p.db.QueryRow("SELECT id, device_id, name, value, attributes, created_at, updated_at FROM device_parameters WHERE device_id = ? AND name = ?", deviceId, name)
	parameter := &DeviceParameter{}
	err := row.Scan(&parameter.ID, &parameter.DeviceID, &parameter.Name, &parameter.Value, &parameter.Attributes, &parameter.CreatedAt, &parameter.UpdatedAt)
	if err != nil {
		return nil, err
	}
	return parameter, nil
}

func (p *Persistence) GetDeviceParameters(deviceId int) ([]DeviceParameter, error) {
	rows, err := p.db.Query("SELECT id, device_id, name, value, attributes, created_at, updated_at FROM device_parameters WHERE device_id = ?", deviceId)
	if err != nil {
		return nil, err
	}
	defer rows.Close()

	var parameters []DeviceParameter
	for rows.Next() {
		parameter := DeviceParameter{}
		err := rows.Scan(&parameter.ID, &parameter.DeviceID, &parameter.Name, &parameter.Value, &parameter.Attributes, &parameter.CreatedAt, &parameter.UpdatedAt)
		if err != nil {
			return nil, err
		}
		parameters = append(parameters, parameter)
	}
	return parameters, nil
}

func (p *Persistence) UpdateDeviceParameter(parameter *DeviceParameter) error {
	stmt, err := p.db.Prepare("UPDATE device_parameters SET value = ?, attributes = ?, updated_at = ? WHERE id = ?")
	if err != nil {
		return err
	}
	defer stmt.Close()

	_, err = stmt.Exec(parameter.Value, parameter.Attributes, time.Now(), parameter.ID)
	return err
}

func (p *Persistence) DeleteDeviceParameter(id int) error {
	stmt, err := p.db.Prepare("DELETE FROM device_parameters WHERE id = ?")
	if err != nil {
		return err
	}
	defer stmt.Close()

	_, err = stmt.Exec(id)
	return err
}
