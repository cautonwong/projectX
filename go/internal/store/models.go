package store

import "time"

type SensorReading struct {
	Topic     string    `json:"topic"`
	Value     float64   `json:"value"`
	Unit      string    `json:"unit"`
	Timestamp time.Time `json:"timestamp"`
}

type Device struct {
	ID          int       `json:"id"`
	Name        string    `json:"name"`
	Description string    `json:"description"`
	TopicPrefix string    `json:"topic_prefix"`
	CreatedAt   time.Time `json:"created_at"`
}

type DeviceParameter struct {
	ID         int       `json:"id"`
	DeviceID   int       `json:"device_id"`
	Name       string    `json:"name"`
	Value      string    `json:"value"`
	Attributes string    `json:"attributes"`
	CreatedAt  time.Time `json:"created_at"`
	UpdatedAt  time.Time `json:"updated_at"`
}
