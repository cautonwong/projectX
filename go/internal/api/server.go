package api

import (
	"net/http"
	"time"

	"projectX.com/internal/config"
	"projectX.com/internal/mqtt"
	"projectX.com/internal/store"
	"projectX.com/internal/web"

	"github.com/go-chi/chi/v5"
	"github.com/go-chi/chi/v5/middleware"
)

func NewServer(cfg *config.Config, mem *store.MemoryStore, db *store.Persistence, hub *web.Hub, mqttClient *mqtt.Client) *http.Server {
	apiHandlers := &Handlers{
		memStore: mem,
		dbStore:  db,
		wsHub:    hub,
	}

	webHandlers := web.NewHandlers(mem, db, mqttClient)

	r := chi.NewRouter()
	r.Use(middleware.RequestID)
	r.Use(middleware.RealIP)
	r.Use(middleware.Logger)
	r.Use(middleware.Recoverer)
	r.Use(middleware.Timeout(60 * time.Second))

	r.Get("/health", apiHandlers.HandleHealthCheck)

	// API routes
	r.Route("/api/v1", func(r chi.Router) {
		r.Get("/latest/{topic:*}", apiHandlers.HandleGetLatestValue)
		r.Get("/history", apiHandlers.HandleGetHistory)

		r.Route("/devices", func(r chi.Router) {
			r.Get("/", apiHandlers.HandleGetDevices)
			r.Post("/", apiHandlers.HandleCreateDevice)
			r.Get("/{id}", apiHandlers.HandleGetDevice)
			r.Put("/{id}", apiHandlers.HandleUpdateDevice)
			r.Delete("/{id}", apiHandlers.HandleDeleteDevice)

			r.Get("/{id}/parameters", apiHandlers.HandleGetDeviceParameters)
			r.Post("/{id}/parameters", apiHandlers.HandleCreateDeviceParameter)
			r.Put("/{id}/parameters/{paramId}", apiHandlers.HandleUpdateDeviceParameter)
			r.Delete("/{id}/parameters/{paramId}", apiHandlers.HandleDeleteDeviceParameter)
		})
	})

	// Web UI routes
	r.Get("/", webHandlers.HandleDashboard)
	r.Get("/ws", func(w http.ResponseWriter, r *http.Request) {
		web.ServeWs(apiHandlers.wsHub, w, r)
	})
	r.Get("/ui/sensor-cards", webHandlers.HandleGetSensorCards)
	r.Post("/ui/control/{topic:*}", webHandlers.HandleControl)

	// Device UI routes
	r.Route("/ui/devices", func(r chi.Router) {
		r.Get("/", webHandlers.HandleGetDevicesPage)
		r.Get("/list", webHandlers.HandleGetDeviceList)
		r.Get("/new", webHandlers.HandleNewDeviceForm)
		r.Post("/", webHandlers.HandleCreateDevice)
		r.Get("/{id}/edit", webHandlers.HandleEditDeviceForm)
		r.Put("/{id}", webHandlers.HandleUpdateDevice)
		r.Delete("/{id}", webHandlers.HandleDeleteDevice)
	})

	// Static files
	fs := http.FileServer(http.Dir("./go/web/static"))
	r.Handle("/static/*", http.StripPrefix("/static/", fs))


	return &http.Server{
		Addr:    cfg.Server.Address,
		Handler: r,
	}
}