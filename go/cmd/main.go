package main

import (
	"context"
	"log/slog"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"

	"projectX.com/internal/api"
	"projectX.com/internal/config"
	"projectX.com/internal/core"
	"projectX.com/internal/mqtt"
	"projectX.com/internal/store"

	paho "github.com/eclipse/paho.mqtt.golang"
)

func main() {
	logger := slog.New(slog.NewJSONHandler(os.Stdout, nil))
	slog.SetDefault(logger)

	cfg, err := config.LoadConfig(".")
	if err != nil {
		slog.Error("cannot load config", "error", err)
		os.Exit(1)
	}

	pStore, err := store.NewPersistence(cfg.Database.Filepath)
	if err != nil {
		slog.Error("failed to initialize persistent store", "error", err)
		os.Exit(1)
	}
	defer pStore.Close()

	memStore := store.NewMemoryStore()

	core.MainWorkQueue = make(chan paho.Message, cfg.Processor.QueueSize)
	core.StartConcurrentProcessor(cfg, pStore, memStore)
	slog.Info("Core processor workers started")

	mqttClient := mqtt.NewClient(cfg, core.MainWorkQueue)
	if err := mqttClient.Connect(); err != nil {
		slog.Error("failed to connect to MQTT broker", "error", err)
		os.Exit(1)
	}
	mqttClient.Subscribe()

	apiServer := api.NewServer(cfg, memStore, pStore)
	go func() {
		slog.Info("Starting API server", "address", cfg.Server.Address)
		if err := apiServer.ListenAndServe(); err != nil && err != http.ErrServerClosed {
			slog.Error("API server crashed", "error", err)
			os.Exit(1)
		}
	}()

	quit := make(chan os.Signal, 1)
	signal.Notify(quit, syscall.SIGINT, syscall.SIGTERM)
	<-quit
	slog.Info("Shutting down server...")

	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	if err := apiServer.Shutdown(ctx); err != nil {
		slog.Error("Server forced to shutdown", "error", err)
	}

	mqttClient.Disconnect(250)
	slog.Info("MQTT client disconnected.")
	slog.Info("Server exiting.")
}
