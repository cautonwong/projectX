package store

import "time"

type SensorReading struct {
	Topic     string    `json:"topic"`
	Value     float64   `json:"value"`
	Unit      string    `json:"unit"`
	Timestamp time.Time `json:"timestamp"`
}
