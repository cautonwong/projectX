#!/bin/bash

# ==============================================================================
# Go-Templ-HTMX Boilerplate Initialization Script v1.1
# ==============================================================================
#
# 这个脚本会创建完整的项目目录结构和所有必要的初始文件。
# 用法: ./init.sh <go_module_name>
# 例如: ./init.sh github.com/my-user/my-project
#

# 检查是否提供了模块名
if [ -z "$1" ]; then
  echo "错误: 请提供 Go 模块名作为参数。"
  echo "用法: ./init.sh <go_module_name>"
  exit 1
fi

MODULE_NAME=$1
PROJECT_NAME=$(basename "$MODULE_NAME")

echo "🚀 开始初始化项目 '$PROJECT_NAME'..."
echo "Go 模块名: $MODULE_NAME"

# --- 1. 创建目录结构 ---
echo "📂 正在创建目录结构..."
mkdir -p \
  assets/css \
  cmd/app \
  internal/config \
  internal/handler \
  internal/services \
  db/migrations \
  db/queries \
  view/layouts \
  view/pages \
  view/shared \
  public \
  scripts/seed

# --- 2. 创建 Go 相关文件 ---
echo "📝 正在创建 Go 文件..."

# go.mod
cat << EOF > go.mod
module $MODULE_NAME

go 1.25

require (
	github.com/a-h/templ v0.3.943
)
EOF

# .air.toml
cat << EOF > .air.toml
root = "."
tmp_dir = "tmp"

[build]
  cmd = "/root/go/bin/templ generate && go build -o ./tmp/main ./cmd/app"
  bin = "./tmp/main"
  full_bin = ""
  delay = 1000
  stop_on_error = true
  kill_delay = 500
  log = "air.log"
  send_interrupt = false
  exclude_dir = ["assets", "tmp", "vendor", "public", "scripts"]
  include_ext = ["go", "templ"]

[log]
  time = true

[color]
  main = "yellow"
  watcher = "cyan"
  build = "green"
  runner = "magenta"
EOF

# cmd/app/main.go
cat << EOF > cmd/app/main.go
package main

import (
	"log"
	"net/http"

	"github.com/go-chi/chi/v5"
	"github.com/go-chi/chi/v5/middleware"

	"$(MODULE_NAME)/internal/handler"
)

func main() {
	router := chi.NewRouter()
	router.Use(middleware.Logger)
	router.Use(middleware.Recoverer)

	// 托管静态文件
	fs := http.FileServer(http.Dir("./public"))
	router.Handle("/public/*", http.StripPrefix("/public/", fs))

	// 注册路由
	homeHandler := handler.NewHomeHandler()
	router.Get("/", homeHandler.HandleHomeShow)

	log.Println("✅ 服务器已启动于 http://localhost:3000")
	if err := http.ListenAndServe(":3000", router); err != nil {
		log.Fatalf("❌ 无法启动服务器: %v", err)
	}
}
EOF

# internal/handler/home_handler.go
cat << EOF > internal/handler/home_handler.go
package handler

import (
	"net/http"

	"$(MODULE_NAME)/view/pages"
	"github.com/a-h/templ"
)

type HomeHandler struct{}

func NewHomeHandler() *HomeHandler {
	return &HomeHandler{}
}

func (h *HomeHandler) HandleHomeShow(w http.ResponseWriter, r *http.Request) {
	templ.Handler(pages.Home("Go + Templ Boilerplate")).ServeHTTP(w, r)
}
EOF

# --- 3. 创建视图 (Templ) 文件 ---
echo "🎨 正在创建 Templ 视图文件..."

# view/layouts/base.templ
cat << 'EOF' > view/layouts/base.templ
package layouts

templ Base(title string) {
	<!DOCTYPE html>
	<html lang="en" data-theme="light">
		<head>
			<meta charset="UTF-8"/>
			<meta name="viewport" content="width=device-width, initial-scale=1.0"/>
			<title>{ title }</title>
			<link rel="stylesheet" href="/public/styles.css"/>
			<script src="https://unpkg.com/htmx.org@1.9.12" defer></script>
			<script src="https://unpkg.com/alpinejs@3.14.0" defer></script>
		</head>
		<body class="bg-base-200">
			{ children... }
		</body>
	</html>
}
EOF

# view/pages/home.templ
cat << 'EOF' > view/pages/home.templ
package pages

import "$(MODULE_NAME)/view/layouts"

templ Home(name string) {
	@layouts.Base(name) {
		<div class="hero min-h-screen bg-base-200">
			<div class="hero-content text-center">
				<div class="max-w-md">
					<h1 class="text-5xl font-bold">Hello there 👋</h1>
					<p class="py-6">
						Welcome to your new app: <span class="font-bold">{ name }</span>
					</p>
					<button class="btn btn-primary">Get Started</button>
				</div>
			</div>
		</div>
	}
}
EOF

# --- 4. 创建前端构建相关文件 ---
echo "💄 正在创建前端构建文件..."

# package.json
cat << EOF > package.json
{
  "name": "$PROJECT_NAME",
  "version": "1.0.0",
  "scripts": {
    "css:dev": "npx @tailwindcss/cli -i ./assets/css/main.css -o ./public/styles.css --watch",
    "css:build": "npx @tailwindcss/cli -i ./assets/css/main.css -o ./public/styles.css --minify"
  },
  "devDependencies": {
    "@tailwindcss/cli": "^4.0.0",
    "daisyui": "^4.10.1",
    "tailwindcss-animate": "^1.0.7",
    "tailwindcss": "^4.0.0"
  }
}
EOF

# tailwind.config.js
cat << 'EOF' > tailwind.config.js
import daisyui from 'daisyui';

/** @type {import('tailwindcss').Config} */
export default {
  content: [
    './view/**/*.templ',
    './static/**/*.html',
  ], // v4 中 content 是自动的
  plugins: [
    daisyui,
  ],
  daisyui: {
    themes: ["light", "dark", "cupcake"],
  },
};
EOF

# assets/css/main.css
cat << 'EOF' > assets/css/main.css
@import "tailwindcss";
EOF

# --- 5. 创建 Makefile 和其他配置文件 ---
echo "🛠️ 正在创建 Makefile 和 .gitignore..."

# Makefile
cat << 'EOF' > Makefile
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
	# @echo "✨ Generating sqlc code..."
	# @sqlc generate

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
EOF

# .gitignore
#cat << EOF > .gitignore
# Go
#bin/
#tmp/
#vendor/
#*.log

# Environment
#.env*
#!/.env.example

# Node
#node_modules/
#package-lock.json
#EOF

# --- 6. 完成 ---
echo ""
echo "✅ 项目初始化成功!"
echo ""
echo "下一步:"
echo "1. 运行 'go mod tidy' 来同步依赖。"
echo "2. 运行 'make setup' 来安装 npm 依赖。"
echo "3. 在一个终端运行 'make tailwind-watch'。"
echo "4. 在另一个终端运行 'make dev'。"
echo "5. 打开浏览器访问 http://localhost:3000"
echo ""
echo "Happy Coding! 🎉"


go get github.com/alexedwards/scs/v2
go get github.com/google/uuid