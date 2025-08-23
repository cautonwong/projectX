package core

import (
	"encoding/json"
	"log/slog"
	"time"

	"projectX.com/internal/config"
	"projectX.com/internal/store"

	mqtt "github.com/eclipse/paho.mqtt.golang"
)

var MainWorkQueue chan mqtt.Message

func StartConcurrentProcessor(cfg *config.Config, db *store.Persistence, memStore *store.MemoryStore) {
	numWorkers := cfg.Processor.NumWorkers
	slog.Info("Starting processor with worker pool", "count", numWorkers)

	for i := 0; i < numWorkers; i++ {
		go worker(i+1, MainWorkQueue, db, memStore)
	}
}

func worker(id int, jobs <-chan mqtt.Message, db *store.Persistence, memStore *store.MemoryStore) {
	slog.Info("Worker started", "id", id)
	for msg := range jobs {
		topic := msg.Topic()

		var reading store.SensorReading
		if err := json.Unmarshal(msg.Payload(), &reading); err != nil {
			slog.Error("Failed to parse message payload", "worker_id", id, "topic", topic, "error", err)
			continue
		}
		reading.Topic = topic
		reading.Timestamp = time.Now().UTC()

		memStore.UpdateValue(topic, reading)

		if reading.Value > 90.0 {
			slog.Warn("CRITICAL: Overheating detected!", "worker_id", id, "topic", topic, "value", reading.Value)
		}

		select {
		case db.WriteQueue <- reading:
		default:
			slog.Warn("Database write queue is full. Dropping message.", "worker_id", id, "topic", topic)
		}
	}
}
