package main

import (
	"log"
	"net/http"
	"os"

	"gotth/internal/auth"
	"gotth/internal/data"
	"gotth/internal/handler"
	"gotth/internal/middleware"

	"github.com/go-chi/chi/v5"
	chiMiddleware "github.com/go-chi/chi/v5/middleware"
	"github.com/joho/godotenv"
)

func main() {
	// 1. 加载 .env 文件
	err := godotenv.Load()
	if err != nil {
		log.Println("Warning: .env file not found, using default environment variables")
	}

	// 2. 从环境变量中获取数据库路径
	dbPath := os.Getenv("DATABASE_PATH")
	if dbPath == "" {
		dbPath = "app.db" // 如果没有设置，则默认为 app.db
	}
	// 1. 初始化服务
	db, err := data.InitDB(dbPath)
	if err != nil {
		log.Fatalf("❌ 数据库初始化失败: %v", err)
	}
	defer db.Close()
	queries := data.New(db)
	sessionManager := auth.NewSessionManager()
	sseBroker := handler.NewBroker()
	defer sseBroker.Shutdown()

	// 2. 初始化 Handlers
	homeHandler := &handler.HomeHandler{}
	authHandler := &handler.AuthHandler{DB: queries, Session: sessionManager}
	dashboardHandler := &handler.DashboardHandler{}
	todoHandler := &handler.TodoHandler{DB: queries}
	uiLabHandler := &handler.UILabHandler{DB: queries}
	// 3. 设置路由
	router := chi.NewRouter()
	router.Use(chiMiddleware.Logger)
	router.Use(chiMiddleware.Recoverer)
	router.Use(sessionManager.LoadAndSave)
	router.Get("/sse-stream", sseBroker.ServeHTTP)

	fs := http.FileServer(http.Dir("./public"))
	router.Handle("/public/*", http.StripPrefix("/public/", fs))

	// 公共路由 (任何人都可以访问)
	router.Get("/", homeHandler.HandleHomeShow)
	router.Get("/login", authHandler.HandleLoginShow)
	router.Post("/login", authHandler.HandleLoginPost)
	router.Get("/register", authHandler.HandleRegisterShow)
	router.Post("/register", authHandler.HandleRegisterPost)

	// === 受保护的路由组 ===
	router.Group(func(r chi.Router) {
		// 应用我们的“认证守卫”中间件，并传入数据库连接
		r.Use(middleware.RequireAuth(sessionManager, queries))

		// 所有在这个组里的路由都将受到保护
		r.Get("/dashboard", dashboardHandler.HandleDashboardShow)
		r.Post("/logout", authHandler.HandleLogoutPost)
		r.Get("/todos", todoHandler.HandleTodosShow)
		r.Post("/todos", todoHandler.HandleTodoCreate)
		r.Patch("/todos/{id}", todoHandler.HandleTodoUpdate)
		r.Delete("/todos/{id}", todoHandler.HandleTodoDelete)

		r.Get("/ui-lab", uiLabHandler.HandleUILabShow)
		r.Get("/ui-lab/buttons-forms", uiLabHandler.HandleButtonsFormsShow)
		r.Post("/ui-lab/loading-button", uiLabHandler.HandleButtonLoadingExample)
		r.Get("/ui-lab/feedback", uiLabHandler.HandleFeedbackShow)
		r.Get("/ui-lab/modal-content", uiLabHandler.HandleModalContent)
		r.Post("/ui-lab/show-toast", uiLabHandler.HandleShowToast)
		r.Get("/ui-lab/table", uiLabHandler.HandleTableShow)
		r.Get("/ui-lab/table-content", uiLabHandler.HandleTableContent)

		r.Get("/ui-lab/sse", uiLabHandler.HandleSseShow)
	})
	//router.Get("/sse-stream", uiLabHandler.HandleSSE)

	log.Println("✅ 服务器已启动于 http://localhost:3000")
	if err := http.ListenAndServe(":3000", router); err != nil {
		log.Fatalf("❌ 无法启动服务器: %v", err)
	}
}
