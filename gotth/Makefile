.PHONY: dev build generate tailwind-install tailwind-watch tailwind-build run-all

# ==============================================================================
# Go Commands
# ==============================================================================

# 启动开发服务器 (带热重载)
dev:
	@echo "🔥 Starting Go dev server with Air..."
	@air

# 构建生产环境二进制文件
build: tailwind-build
	@echo "📦 Building Go binary for production..."
	@templ generate
	@go build -o ./bin/app ./cmd/app

# 生成 Templ 和 sqlc (如果需要)
generate:
	@echo "✨ Generating Templ components..."
	@templ generate
	@echo "✨ Generating sqlc code..."
	@sqlc generate

# ==============================================================================
# Frontend Commands
# ==============================================================================

# 安装 npm 开发依赖
npm-install:
	@echo "Installing npm dev dependencies..."
	@npm install

# (开发) 编译并监听 CSS
tailwind-watch:
	@echo "🎨 Watching for CSS changes..."
	@npm run css:dev

# (生产) 构建并压缩 CSS
tailwind-build:
	@echo "🎨 Building and minifying CSS..."
	@npm run css:build

# ==============================================================================
# Helper Commands
# ==============================================================================

# 运行所有开发进程 (需要 concurrently)
# npm install -g concurrently
run-all-dev:
	@concurrently "make dev" "make tailwind-watch"

# 初始化项目依赖
setup: npm-install
	@echo "✅ Project setup complete. Run 'make dev' and 'make tailwind-watch' in separate terminals."

install-tools:
	@echo "🛠️  Installing Go development tools..."
	@go install github.com/air-verse/air@latest
	@go install github.com/a-h/templ/cmd/templ@latest
	@go install github.com/sqlc-dev/sqlc/cmd/sqlc@latest
	@go install -tags 'sqlite' github.com/golang-migrate/migrate/v4/cmd/migrate@latest
	@echo "✅ Go tools installed."

# ==============================================================================
# Database Commands
# ==============================================================================

# 创建新的迁移文件
db-migration:
    @read -p "Enter migration name: " name; \
    migrate create -ext sql -dir db/migrations -seq $$name

# 执行数据库迁移
db-migrate:
	@echo "🔄 Running database migrations..."
	@migrate -path db/migrations -database "sqlite3://app.db" up

# 回滚最后一次迁移
db-rollback:
	@echo "⏪ Rolling back the last migration..."
	@migrate -path db/migrations -database "sqlite3://app.db" down 1

# 回滚所有迁移
db-reset:
	@echo "🗑️  Rolling back all migrations..."
	@migrate -path db/migrations -database "sqlite3://app.db" down

db-seed:
	@echo "🌱 Seeding database..."
	@go run ./scripts/seed/main.go