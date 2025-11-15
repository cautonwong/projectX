package handler

import (
	"fmt"
	"log"
	"net/http"
	"sync"
	"time"
)

type Client struct {
	id     string
	events chan []byte
}

type Broker struct {
	clients     map[*Client]bool
	clientsMux  sync.RWMutex
	register    chan *Client
	unregister  chan *Client
	events      chan []byte
	shutdown    chan struct{}
	headersSent bool
}

func NewBroker() *Broker {
	broker := &Broker{
		clients:    make(map[*Client]bool),
		register:   make(chan *Client),
		unregister: make(chan *Client),
		events:     make(chan []byte),
		shutdown:   make(chan struct{}),
	}

	// 启动广播协程
	go broker.run()
	// 启动生成事件的协程
	go broker.generateEvents()

	return broker
}

func (b *Broker) run() {
	for {
		select {
		case client := <-b.register:
			b.clientsMux.Lock()
			b.clients[client] = true
			b.clientsMux.Unlock()
			log.Printf("Client %s connected", client.id)

		case client := <-b.unregister:
			b.clientsMux.Lock()
			if _, ok := b.clients[client]; ok {
				delete(b.clients, client)
				close(client.events)
			}
			b.clientsMux.Unlock()
			log.Printf("Client %s disconnected", client.id)

		case event := <-b.events:
			b.clientsMux.RLock()
			for client := range b.clients {
				select {
				case client.events <- event:
				default:
					// 如果客户端处理太慢，关闭连接
					close(client.events)
					delete(b.clients, client)
				}
			}
			b.clientsMux.RUnlock()

		case <-b.shutdown:
			b.clientsMux.Lock()
			for client := range b.clients {
				close(client.events)
				delete(b.clients, client)
			}
			b.clientsMux.Unlock()
			return
		}
	}
}

func (b *Broker) generateEvents() {
	counter := 0
	ticker := time.NewTicker(1 * time.Second)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
			counter++
			b.events <- []byte(fmt.Sprintf("event: message\ndata: %d\n\n", counter))
		case <-b.shutdown:
			return
		}
	}
}

func (b *Broker) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	// 立即设置 headers
	w.Header().Set("Content-Type", "text/event-stream")
	w.Header().Set("Cache-Control", "no-cache")
	w.Header().Set("Connection", "keep-alive")
	w.Header().Set("X-Accel-Buffering", "no")
	w.Header().Set("Access-Control-Allow-Origin", "*") // 如果需要 CORS

	flusher, ok := w.(http.Flusher)
	if !ok {
		log.Printf("Streaming not supported")
		http.Error(w, "Streaming not supported", http.StatusInternalServerError)
		return
	}

	// 创建新的客户端
	clientChan := make(chan []byte, 1)
	client := &Client{
		id:     r.RemoteAddr,
		events: clientChan,
	}

	// 注册客户端
	b.register <- client
	defer func() {
		b.unregister <- client
		close(clientChan)
	}()

	// 发送一个初始消息并立即刷新
	fmt.Fprintf(w, "data: Connected\n\n")
	flusher.Flush()

	// 监听连接关闭
	notify := r.Context().Done()
	go func() {
		<-notify
		b.unregister <- client
	}()

	// 发送消息循环
	for {
		select {
		case <-notify:
			return
		case msg, ok := <-client.events:
			if !ok {
				return
			}
			_, err := fmt.Fprintf(w, "data: %s\n\n", msg)
			if err != nil {
				log.Printf("Error writing to client: %v", err)
				return
			}
			flusher.Flush()
		}
	}
}

func (b *Broker) Shutdown() {
	close(b.shutdown)
}
