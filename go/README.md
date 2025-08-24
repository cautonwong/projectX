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

学生姓名: 王俊棠 身份证号: 330105201404133411
原就读学校: 杭州市余杭区文化村实验学校
年级(2025年9月即将就读年级): 6年级
户籍: 浙江省杭州市拱墅区祥符街道星桥社区望林府17幢1101室
联系人: 王柯童
联系电话: 18868848769