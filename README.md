# libmsx




[![CMake on multiple platforms](https://github.com/cautonwong/libmsx/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/cautonwong/libmsx/actions/workflows/cmake-multi-platform.yml)


微服务开发基础库
----

- 构建系统cmake
- 工作流github workflow
- 质量控制ASAN

--------

## 开始

1. 复制`CMakeUserPresets.json.template`并重命名为`CMakeUserPresets.json`
2. 修改自己的工具链位置
3. 编写代码
```
struct my_service{
    service_t *base;
    // my other fields;
};

int main()
{
    service_init(); // 解析配置文件 注册回调  添加http route
    service_run();
    service_destroy();
}

```


## 已有微服务

```
south: 南向设备的发现注册抄读和控制 调用南向设备通信协议 按照任务间隔采集设备数据,经IPC存入core-data
north: 北向主站 调用北向主站通信协议 IPC给core-data和core-command
core-data: 数据中心 注册中心
core-command: 从北往南的命令(点抄或者阻塞式的设备控制)
```

```json
"workspaceMount": "source=${localWorkspaceFolder},target=/workspaces/${localWorkspaceFolderBasename},type=bind,consistency=cached",
"workspaceFolder": "/workspaces/${localWorkspaceFolderBasename}"
```

## TODO

### firmware

1. dts + threadx
2. zephyr userspace
3. zephyr + loadable module

### microservice

1. c/c++/go
2. 容器化部署


---

### **Go-Templ HTMX Boilerplate: 详细设计文档 (v1.1 - 更新版)**

#### **1. 导言 (Introduction)**

##### **1.1 项目愿景**
本项目旨在创建一个功能完备、高度可定制且遵循最佳实践的全栈 Web 应用样板。开发者可基于此样板快速启动新项目，无需重复进行环境配置、认证系统搭建和基础 UI 组件开发。

##### **1.2 核心架构原则**
*   **服务器为中心 (Server-Centric):** UI 的状态和渲染主要由后端 Go 服务器通过 `go-templ` 控制。前端通过 `htmx` 发送请求并无缝替换页面局部内容。
*   **类型安全至上 (Type-Safety First):** 从数据库查询 (`sqlc`) 到后端逻辑 (Go) 再到 HTML 模板 (`go-templ`)，在编译阶段尽可能捕捉错误，提升代码健壮性。
*   **最小化 JavaScript (Minimal JavaScript):** 仅在无法通过 htmx 实现的纯客户端交互上使用 `Alpine.js`，避免引入大型前端框架的复杂性。
*   **声明式 UI (Declarative UI):** UI 组件作为可复用的 Go 函数，通过参数接收数据并渲染，逻辑清晰。
*   **配置优先 (Configuration-First):** 关键行为（如环境变量、主题样式）通过配置文件驱动，而非硬编码。

#### **2. 技术栈详情 (Technology Stack Deep Dive)**

| 类别 | 技术选型 | 版本/备注 | 设计集成要点 |
| :--- | :--- | :--- | :--- |
| **CSS 框架** | **Tailwind CSS** | **v4.x** | **不再使用 `postcss.config.js` 和 `content` 扫描。** 将采用独立的 Tailwind CLI 或 Vite 插件模式。核心配置文件 `tailwind.config.js` 将主要用于定义主题变量（颜色、字体等），并通过 `@theme` 指令在 CSS 中引用。 |
| **数据库 ORM** | **sqlc** | - | 严格遵循 "SQL-first" 原则。所有数据库交互都必须先在 `.sql` 文件中定义查询，再通过 `make generate` 生成类型安全的 Go 代码。 |
| **HTTP 路由**| **Chi Router** | - | 选择 `chi` 是因为它轻量、性能高，且拥有强大的中间件生态系统，与 Go 的 `http.Handler` 接口完美兼容。 |
| **Session 管理**| **alexedwards/scs** | - | 提供行业标准的安全 Session 管理。默认使用基于 SQLite 的存储引擎，便于开发，并可轻松切换至 Redis 等用于生产环境。 |
| **数据库迁移**| **golang-migrate** | - | 行业标准的数据库迁移工具，支持通过 CLI 管理数据库 schema 的版本。 |
| **其他** | *(如前述)* | - | *(如前述)* |

#### **3. 详细项目结构与文件职责**

```plaintext
/
├── assets/                     # 前端源文件
│   └── css/
│       └── main.css            # Tailwind CSS v4 的主入口文件
├── cmd/
│   └── app/
│       ├── main.go             # 应用入口: 初始化依赖、设置路由、启动服务器
│       └── routes.go           # 路由定义: 将 URL 路径映射到具体的 handler
├── internal/
│   ├── config/
│   │   └── config.go           # 使用 struct 定义配置，通过 Viper 加载
│   ├── core/
│   │   ├── user_service.go     # 示例: 用户相关的业务逻辑
│   │   └── tenant_service.go   # 示例: 租户相关的业务逻辑
│   ├── data/                   # 数据访问层
│   │   ├── db.go               # 数据库连接初始化与管理
│   │   ├── models.go           # sqlc 生成的模型代码 (gitignored)
│   │   ├── queries.sql.go      # sqlc 生成的查询代码 (gitignored)
│   │   └── querier.go          # sqlc 生成的 Querier 接口
│   ├── handler/                # HTTP 处理器
│   │   ├── auth_handler.go     # 处理登录、注册、登出
│   │   ├── dashboard_handler.go# 处理仪表盘页面逻辑
│   │   └── sse_handler.go      # 处理 SSE 连接
│   ├── middleware/             # HTTP 中-间件
│   │   ├── auth.go             # 认证检查与上下文注入
│   │   └── tenancy.go          # 租户隔离检查
│   ├── sse/
│   │   └── hub.go              # 管理所有 SSE 连接，并广播消息
│   └── services/
│       └── logger.go           # Slog 的初始化与配置
├── db/                         # 数据库 schema 与查询
│   ├── migrations/             # golang-migrate 的迁移文件 (e.g., 000001_create_users_table.up.sql)
│   ├── queries/                # sqlc 的查询文件 (e.g., user_queries.sql)
│   └── schema.sql              # 数据库的完整 schema 定义
├── view/                       # Go Templ 模板
│   ├── layouts/
│   │   └── base.templ          # 包含 <head> 和 <body> 的基础 HTML 框架
│   │   └── app.templ           # 包含侧边栏和导航栏的应用主布局
│   ├── pages/
│   │   ├── login.templ
│   │   └── dashboard/
│   │       └── index.templ
│   └── shared/                 # 跨页面共享的原子组件
│       ├── button.templ
│       ├── form_input.templ
│       ├── modal.templ
│       └── toast.templ
├── public/                     # 编译后的静态资源 (会被服务器托管)
│   └── styles.css              # 由 Tailwind CSS v4 生成的最终 CSS 文件
├── scripts/
│   └── seed/                   # 数据填充相关脚本
│       ├── main.go
│       └── seeders.go
├── .air.toml
├── .gitignore
├── Dockerfile
├── docker-compose.yml
├── go.mod
├── Makefile
└── tailwind.config.js          # Tailwind v4 配置文件 (主要用于主题定义)
```

#### **4. 核心模块设计详述**

#### **4. 核心模块设计详述 (已更新)**

##### **4.1 UI & 前端构建 (Tailwind CSS v4)**

*   **`package.json` 文件:**
    为了管理前端开发依赖，项目根目录将包含一个 `package.json` 文件。
    ```json
    {
      "name": "your-project-name",
      "version": "1.0.0",
      "devDependencies": {
        "@tailwindcss/cli": "^4.0.0-alpha.11", // 使用 v4 的 CLI
        "daisyui": "^4.10.1",
        "tailwindcss": "^4.0.0-alpha.11" // 核心引擎
      }
    }
    ```

*   **`assets/css/main.css` 文件内容 (无变化):**
    ```css
    @import "tailwindcss";

    @theme {
      --color-brand: #4f46e5;
    }
    ```

*   **`tailwind.config.js` 文件内容 (无变化):**
    ```javascript
    import daisyui from 'daisyui';

    /** @type {import('tailwindcss').Config} */
    export default {
      plugins: [
        daisyui,
      ],
      daisyui: {
        themes: ["light", "dark"],
      },
    };
    ```

*   **`Makefile` 集成 (已修正):**
    ```makefile
    # 安装 npm 开发依赖
    npm-install:
        npm install

    # (开发) 编译并监听 CSS 变化
    tailwind-watch:
        npx @tailwindcss/cli -i ./assets/css/main.css -o ./public/styles.css --watch

    # (生产) 构建并压缩 CSS
    tailwind-build:
        npx @tailwindcss/cli -i ./assets/css/main.css -o ./public/styles.css --minify
    ```

*   **开发工作流:**
    开发者需要打开两个终端窗口：
    1.  一个运行 `make dev` (启动 `air` 来热重载 Go 应用)。
    2.  另一个运行 `make tailwind-watch` (实时编译 CSS)。
    或者，可以使用一个工具如 `overmind` 或 `foreman` 来通过一个命令同时启动两个进程。我们可以在 `Makefile` 中添加这样一个目标。

---

#### **6. 项目初始化脚本 (新增)**

为了实现一键式项目创建，我们提供一个名为 `init.sh` 的 shell 脚本。此脚本将创建完整的目录结构和所有基础文件的初始内容。

##### **6.1 使用方法**
1.  将下面的脚本内容保存为 `init.sh` 文件。
2.  给予脚本执行权限: `chmod +x init.sh`
3.  执行脚本并传入您的项目模块名 (例如: `github.com/your-username/my-awesome-app`):
    ```bash
    ./init.sh github.com/your-username/my-awesome-app
    ```

##### **6.2 `init.sh` 脚本内容**

```bash
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

go 1.22

require (
	github.com/a-h/templ v0.2.680
	github.com/go-chi/chi/v5 v5.0.12
)
EOF

# .air.toml
cat << EOF > .air.toml
root = "."
tmp_dir = "tmp"

[build]
  cmd = "templ generate && go build -o ./tmp/main ./cmd/app"
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
    "@tailwindcss/cli": "^4.0.0-alpha.11",
    "daisyui": "^4.10.1",
    "tailwindcss": "^4.0.0-alpha.11"
  }
}
EOF

# tailwind.config.js
cat << 'EOF' > tailwind.config.js
import daisyui from 'daisyui';

/** @type {import('tailwindcss').Config} */
export default {
  content: [], // v4 中 content 是自动的
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
cat << EOF > .gitignore
# Go
bin/
tmp/
vendor/
*.log

# Environment
.env*
!/.env.example

# Node
node_modules/
package-lock.json
EOF

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
```
