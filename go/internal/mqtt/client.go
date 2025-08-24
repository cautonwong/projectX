package mqtt

import (
	"log/slog"
	"time"

	"projectX.com/internal/config"

	mqtt "github.com/eclipse/paho.mqtt.golang"
)

type Client struct {
	client    mqtt.Client
	cfg       *config.Config
	workQueue chan<- mqtt.Message
}

func NewClient(cfg *config.Config, workQueue chan<- mqtt.Message) *Client {
	return &Client{cfg: cfg, workQueue: workQueue}
}

func (c *Client) onMessageReceived(client mqtt.Client, msg mqtt.Message) {
	select {
	case c.workQueue <- msg:
	default:
		slog.Warn("Main work queue is full. Dropping MQTT message.", "topic", msg.Topic())
	}
}

func (c *Client) onConnect(client mqtt.Client) {
	slog.Info("Connected to MQTT Broker.")
	c.Subscribe()
}

func (c *Client) onConnectionLost(client mqtt.Client, err error) {
	slog.Error("MQTT connection lost", "error", err)
}

func (c *Client) Connect() error {
	opts := mqtt.NewClientOptions()
	opts.AddBroker(c.cfg.Mqtt.Broker)
	opts.SetClientID(c.cfg.Mqtt.ClientID)
	opts.SetDefaultPublishHandler(c.onMessageReceived)
	opts.OnConnect = c.onConnect
	opts.OnConnectionLost = c.onConnectionLost
	opts.SetPingTimeout(10 * time.Second)
	opts.SetKeepAlive(30 * time.Second)
	opts.SetAutoReconnect(true)
	opts.SetMaxReconnectInterval(10 * time.Second)

	c.client = mqtt.NewClient(opts)
	if token := c.client.Connect(); token.Wait() && token.Error() != nil {
		return token.Error()
	}
	return nil
}

func (c *Client) Subscribe() {
	topic := c.cfg.Mqtt.Topic
	if token := c.client.Subscribe(topic, 1, nil); token.Wait() && token.Error() != nil {
		slog.Error("Failed to subscribe to topic", "topic", topic, "error", token.Error())
	} else {
		slog.Info("Subscribed to topic", "topic", topic)
	}
}

func (c *Client) Disconnect(quiesce uint) {
	c.client.Disconnect(quiesce)
}

func (c *Client) Publish(topic string, payload interface{}) error {
	token := c.client.Publish(topic, 0, false, payload)
	return token.Error()
}
