package handler

import (
	"log"
	"net/http"

	"gotth/internal/auth"
	"gotth/internal/data"
	"gotth/view/pages"

	"github.com/alexedwards/scs/v2"
	"github.com/google/uuid"
)

// AuthHandler 持有数据库查询接口和 Session 管理器
type AuthHandler struct {
	DB      data.Querier
	Session *scs.SessionManager
}

// --- GET 请求的处理方法 ---

// 修正：添加 (h *AuthHandler) 接收器
func (h *AuthHandler) HandleLoginShow(w http.ResponseWriter, r *http.Request) {
	pages.Login("").Render(r.Context(), w)
}

// 修正：添加 (h *AuthHandler) 接收器
func (h *AuthHandler) HandleRegisterShow(w http.ResponseWriter, r *http.Request) {
	pages.Register().Render(r.Context(), w)
}

// --- POST 请求的处理方法 ---

func (h *AuthHandler) HandleRegisterPost(w http.ResponseWriter, r *http.Request) {
	if err := r.ParseForm(); err != nil {
		http.Error(w, "Invalid form data", http.StatusBadRequest)
		return
	}
	email := r.FormValue("email")
	password := r.FormValue("password")

	// ... (其余 POST 方法代码保持不变，它们已经是正确的) ...
	// ... (HandleLoginPost, HandleLogoutPost 的代码也保持不变) ...

	if email == "" || password == "" {
		http.Error(w, "Email and password are required", http.StatusBadRequest)
		return
	}

	_, err := h.DB.GetUserByEmail(r.Context(), email)
	if err == nil {
		http.Error(w, "Email already exists", http.StatusConflict)
		return
	}

	hashedPassword, err := auth.HashPassword(password)
	if err != nil {
		log.Printf("无法哈希密码: %v", err)
		http.Error(w, "Server error", http.StatusInternalServerError)
		return
	}

	params := data.CreateUserParams{
		ID:           uuid.NewString(),
		Email:        email,
		PasswordHash: hashedPassword,
	}
	user, err := h.DB.CreateUser(r.Context(), params)
	if err != nil {
		log.Printf("无法创建用户: %v", err)
		http.Error(w, "Could not create user", http.StatusInternalServerError)
		return
	}

	_ = h.Session.RenewToken(r.Context())
	h.Session.Put(r.Context(), "userID", user.ID)

	w.Header().Set("HX-Redirect", "/dashboard")
	w.WriteHeader(http.StatusOK)
}

func (h *AuthHandler) HandleLoginPost(w http.ResponseWriter, r *http.Request) {
	if err := r.ParseForm(); err != nil {
		pages.Login("Invalid form data.").Render(r.Context(), w)
		return
	}
	email := r.FormValue("email")
	password := r.FormValue("password")

	user, err := h.DB.GetUserByEmail(r.Context(), email)
	if err != nil {
		w.WriteHeader(http.StatusUnauthorized)
		pages.Login("Invalid email or password.").Render(r.Context(), w)
		return
	}

	if !auth.CheckPasswordHash(password, user.PasswordHash) {
		pages.Login("Invalid email or password.").Render(r.Context(), w)
		return
	}

	err = h.Session.RenewToken(r.Context())
	if err != nil {
		pages.Login("An unexpected error occurred.").Render(r.Context(), w)
		return
	}
	h.Session.Put(r.Context(), "userID", user.ID)

	w.Header().Set("HX-Redirect", "/dashboard")
	w.WriteHeader(http.StatusOK)
}

func (h *AuthHandler) HandleLogoutPost(w http.ResponseWriter, r *http.Request) {
	_ = h.Session.Destroy(r.Context())
	w.Header().Set("HX-Redirect", "/login")
	w.WriteHeader(http.StatusOK)
}
