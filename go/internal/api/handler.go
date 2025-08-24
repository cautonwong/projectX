package api

import (
	"encoding/json"
	"log/slog"
	"net/http"
	"strconv"
	"time"

	"projectX.com/internal/store"
	"projectX.com/internal/web"

	"github.com/go-chi/chi/v5"
)

type Handlers struct {
	memStore *store.MemoryStore
	dbStore  *store.Persistence
	wsHub    *web.Hub
}

func (h *Handlers) HandleHealthCheck(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	json.NewEncoder(w).Encode(map[string]string{"status": "ok"})
}

func (h *Handlers) HandleGetLatestValue(w http.ResponseWriter, r *http.Request) {
	topic := chi.URLParam(r, "*")
	if topic == "" {
		http.Error(w, "Topic cannot be empty", http.StatusBadRequest)
		return
	}

	latestValue, found := h.memStore.GetValue(topic)
	if !found {
		http.Error(w, "Not Found: No data for the specified topic", http.StatusNotFound)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(latestValue)
}

func (h *Handlers) HandleGetHistory(w http.ResponseWriter, r *http.Request) {
	topic := r.URL.Query().Get("topic")
	if topic == "" {
		http.Error(w, "Bad Request: 'topic' query parameter is required", http.StatusBadRequest)
		return
	}

	limitStr := r.URL.Query().Get("limit")
	limit, err := strconv.Atoi(limitStr)
	if err != nil || limit <= 0 {
		limit = 100 // Default limit
	}

	startTimeStr := r.URL.Query().Get("start_time")
	endTimeStr := r.URL.Query().Get("end_time")
	var startTime, endTime time.Time
	if startTimeStr != "" {
		startTime, err = time.Parse(time.RFC3339, startTimeStr)
		if err != nil {
			http.Error(w, "Bad Request: invalid 'start_time' format (must be RFC3339)", http.StatusBadRequest)
			return
		}
	}
	if endTimeStr != "" {
		endTime, err = time.Parse(time.RFC3339, endTimeStr)
		if err != nil {
			http.Error(w, "Bad Request: invalid 'end_time' format (must be RFC3339)", http.StatusBadRequest)
			return
		}
	}

	readings, err := h.dbStore.QueryReadings(topic, limit, startTime, endTime)
	if err != nil {
		slog.Error("failed to query history from db", "error", err)
		http.Error(w, "Internal Server Error", http.StatusInternalServerError)
		return
	}

	type historyResponse struct {
		QueryParams map[string]interface{} `json:"query_params"`
		Readings    []store.SensorReading  `json:"readings"`
	}

	resp := historyResponse{
		QueryParams: map[string]interface{}{
			"topic":      topic,
			"limit":      limit,
			"start_time": startTime,
			"end_time":   endTime,
		},
		Readings: readings,
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(resp)
}
