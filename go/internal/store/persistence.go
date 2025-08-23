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
