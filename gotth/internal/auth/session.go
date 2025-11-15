package auth

import (
	"net/http"
	"time"

	"github.com/alexedwards/scs/v2"
)

// NewSessionManager 创建并配置一个新的 session 管理器
func NewSessionManager() *scs.SessionManager {
	sessionManager := scs.New()
	sessionManager.Lifetime = 24 * time.Hour // Session 有效期
	sessionManager.Cookie.Persist = true
	sessionManager.Cookie.SameSite = http.SameSiteLaxMode
	sessionManager.Cookie.Secure = false // 在生产环境中应设为 true
	return sessionManager
}
