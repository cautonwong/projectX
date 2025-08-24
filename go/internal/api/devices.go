package api

import (
	"encoding/json"
	"net/http"
	"strconv"

	"projectX.com/internal/store"

	"github.com/go-chi/chi/v5"
)

func (h *Handlers) HandleCreateDevice(w http.ResponseWriter, r *http.Request) {
	var device store.Device
	if err := json.NewDecoder(r.Body).Decode(&device); err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

	if err := h.dbStore.CreateDevice(&device); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusCreated)
	json.NewEncoder(w).Encode(device)
}

func (h *Handlers) HandleGetDevice(w http.ResponseWriter, r *http.Request) {
	id, err := strconv.Atoi(chi.URLParam(r, "id"))
	if err != nil {
		http.Error(w, "Invalid device ID", http.StatusBadRequest)
		return
	}

	device, err := h.dbStore.GetDevice(id)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(device)
}

func (h *Handlers) HandleGetDevices(w http.ResponseWriter, r *http.Request) {
	devices, err := h.dbStore.GetDevices()
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(devices)
}

func (h *Handlers) HandleUpdateDevice(w http.ResponseWriter, r *http.Request) {
	id, err := strconv.Atoi(chi.URLParam(r, "id"))
	if err != nil {
		http.Error(w, "Invalid device ID", http.StatusBadRequest)
		return
	}

	var device store.Device
	if err := json.NewDecoder(r.Body).Decode(&device); err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

	device.ID = id
	if err := h.dbStore.UpdateDevice(&device); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusNoContent)
}

func (h *Handlers) HandleDeleteDevice(w http.ResponseWriter, r *http.Request) {
	id, err := strconv.Atoi(chi.URLParam(r, "id"))
	if err != nil {
		http.Error(w, "Invalid device ID", http.StatusBadRequest)
		return
	}

	if err := h.dbStore.DeleteDevice(id); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusNoContent)
}

func (h *Handlers) HandleCreateDeviceParameter(w http.ResponseWriter, r *http.Request) {
	var parameter store.DeviceParameter
	if err := json.NewDecoder(r.Body).Decode(&parameter); err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

	if err := h.dbStore.CreateDeviceParameter(&parameter); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusCreated)
	json.NewEncoder(w).Encode(parameter)
}

func (h *Handlers) HandleGetDeviceParameters(w http.ResponseWriter, r *http.Request) {
	deviceId, err := strconv.Atoi(chi.URLParam(r, "id"))
	if err != nil {
		http.Error(w, "Invalid device ID", http.StatusBadRequest)
		return
	}

	parameters, err := h.dbStore.GetDeviceParameters(deviceId)
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.Header().Set("Content-Type", "application/json")
	json.NewEncoder(w).Encode(parameters)
}

func (h *Handlers) HandleUpdateDeviceParameter(w http.ResponseWriter, r *http.Request) {
	paramId, err := strconv.Atoi(chi.URLParam(r, "paramId"))
	if err != nil {
		http.Error(w, "Invalid parameter ID", http.StatusBadRequest)
		return
	}

	var parameter store.DeviceParameter
	if err := json.NewDecoder(r.Body).Decode(&parameter); err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

	parameter.ID = paramId
	if err := h.dbStore.UpdateDeviceParameter(&parameter); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusNoContent)
}

func (h *Handlers) HandleDeleteDeviceParameter(w http.ResponseWriter, r *http.Request) {
	paramId, err := strconv.Atoi(chi.URLParam(r, "paramId"))
	if err != nil {
		http.Error(w, "Invalid parameter ID", http.StatusBadRequest)
		return
	}

	if err := h.dbStore.DeleteDeviceParameter(paramId); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusNoContent)
}
