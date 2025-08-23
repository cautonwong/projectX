package api

import (
	"net/http"
	"time"

	"projectX.com/internal/config"
	"projectX.com/internal/store"

	"github.com/go-chi/chi/v5"
	"github.com/go-chi/chi/v5/middleware"
)

func NewServer(cfg *config.Config, mem *store.MemoryStore, db *store.Persistence) *http.Server {
	h := &Handlers{
		memStore: mem,
		dbStore:  db,
	}

	r := chi.NewRouter()
	r.Use(middleware.RequestID)
	r.Use(middleware.RealIP)
	r.Use(middleware.Logger)
	r.Use(middleware.Recoverer)
	r.Use(middleware.Timeout(60 * time.Second))

	r.Get("/health", h.HandleHealthCheck)
	r.Route("/api/v1", func(r chi.Router) {
		r.Get("/latest/{topic:*}", h.HandleGetLatestValue)
		r.Get("/history", h.HandleGetHistory)
	})

	return &http.Server{
		Addr:    cfg.Server.Address,
		Handler: r,
	}
}
