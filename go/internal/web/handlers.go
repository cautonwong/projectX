package web

import (
	"net/http"
	"strconv"

	"projectX.com/internal/mqtt"
	"projectX.com/internal/store"
	"projectX.com/web/templates"

	"github.com/go-chi/chi/v5"
)

type Handlers struct {
	memStore   *store.MemoryStore
	dbStore    *store.Persistence
	mqttClient *mqtt.Client
}

func NewHandlers(memStore *store.MemoryStore, dbStore *store.Persistence, mqttClient *mqtt.Client) *Handlers {
	return &Handlers{memStore: memStore, dbStore: dbStore, mqttClient: mqttClient}
}

func (h *Handlers) HandleDashboard(w http.ResponseWriter, r *http.Request) {
	templates.Dashboard().Render(r.Context(), w)
}

func (h *Handlers) HandleGetSensorCards(w http.ResponseWriter, r *http.Request) {
	readings := h.memStore.GetAllValues()
	for _, reading := range readings {
		templates.SensorCard(reading).Render(r.Context(), w)
	}
}

func (h *Handlers) HandleControl(w http.ResponseWriter, r *http.Request) {
	topic := r.URL.Path[len("/ui/control/"):]
	// In a real application, you'd parse a command from the request body
	payload := "{\"command\": \"ping\"}"
	h.mqttClient.Publish(topic, payload)
	w.WriteHeader(http.StatusOK)
}

func (h *Handlers) HandleGetDevicesPage(w http.ResponseWriter, r *http.Request) {
	templates.Devices().Render(r.Context(), w)
}

func (h *Handlers) HandleGetDeviceList(w http.ResponseWriter, r *http.Request) {
	devices, err := h.dbStore.GetDevices()
	if err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	templates.DeviceList(devices).Render(r.Context(), w)
}

func (h *Handlers) HandleNewDeviceForm(w http.ResponseWriter, r *http.Request) {
	templates.DeviceForm(&store.Device{}).Render(r.Context(), w)
}

func (h *Handlers) HandleEditDeviceForm(w http.ResponseWriter, r *http.Request) {
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

	templates.DeviceForm(device).Render(r.Context(), w)
}

func (h *Handlers) HandleCreateDevice(w http.ResponseWriter, r *http.Request) {
	if err := r.ParseForm(); err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

	device := &store.Device{
		Name:        r.FormValue("name"),
		Description: r.FormValue("description"),
		TopicPrefix: r.FormValue("topic_prefix"),
	}

	if err := h.dbStore.CreateDevice(device); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	h.HandleGetDeviceList(w, r)
}

func (h *Handlers) HandleUpdateDevice(w http.ResponseWriter, r *http.Request) {
	id, err := strconv.Atoi(chi.URLParam(r, "id"))
	if err != nil {
		http.Error(w, "Invalid device ID", http.StatusBadRequest)
		return
	}

	if err := r.ParseForm(); err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

	device := &store.Device{
		ID:          id,
		Name:        r.FormValue("name"),
		Description: r.FormValue("description"),
		TopicPrefix: r.FormValue("topic_prefix"),
	}

	if err := h.dbStore.UpdateDevice(device); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}

	h.HandleGetDeviceList(w, r)
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

	h.HandleGetDeviceList(w, r)
}
