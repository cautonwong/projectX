# (需要先安装 mosquitto-clients)
mosquitto_pub -h localhost -t "sensors/factory1/machineA/temperature" -m '{"value": 85.5, "unit": "C"}'

# 查询最新值
curl http://localhost:8080/api/v1/latest/sensors/factory1/machineA/temperature

# 查询历史记录
curl "http://localhost:8080/api/v1/history?topic=sensors/factory1/machineA/temperature&limit=5"

CGO_ENABLED=1 GOOS=linux go build -a -installsuffix  cgo  -ldflags="-s -w" -o /edge-processor ./cmd/main.go
upx --best --lzma /edge-processor

# 启动服务
sudo systemctl start mosquitto

# 检查服务状态
sudo systemctl status mosquitto

# 设置为开机自启 (可选)
sudo systemctl enable mosquitto

mosquitto -c /etc/mosquitto/mosquitto.conf