package store

import (
	"sync"
)

type MemoryStore struct {
	mu     sync.RWMutex
	values map[string]SensorReading
}

func NewMemoryStore() *MemoryStore {
	return &MemoryStore{
		values: make(map[string]SensorReading),
	}
}

func (ms *MemoryStore) UpdateValue(topic string, reading SensorReading) {
	ms.mu.Lock()
	defer ms.mu.Unlock()
	ms.values[topic] = reading
}

func (ms *MemoryStore) GetValue(topic string) (SensorReading, bool) {
	ms.mu.RLock()
	defer ms.mu.RUnlock()
	val, ok := ms.values[topic]
	return val, ok
}

func (ms *MemoryStore) GetAllValues() []SensorReading {
	ms.mu.RLock()
	defer ms.mu.RUnlock()
	readings := make([]SensorReading, 0, len(ms.values))
	for _, reading := range ms.values {
		readings = append(readings, reading)
	}
	return readings
}
