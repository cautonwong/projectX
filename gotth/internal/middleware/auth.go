package middleware

import (
	"context"
	"net/http"

	"gotth/internal/data" // <-- 新增导入

	"github.com/alexedwards/scs/v2"
)

// RequireAuth 现在需要数据库连接来获取用户信息
func RequireAuth(session *scs.SessionManager, db data.Querier) func(http.Handler) http.Handler {
	return func(next http.Handler) http.Handler {
		return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
			userID := session.GetString(r.Context(), "userID")
			if userID == "" {
				// 用户未登录，重定向
				http.Redirect(w, r, "/login", http.StatusSeeOther)
				return
			}

			// 从数据库中获取完整的用户信息
			user, err := db.GetUserByID(r.Context(), userID) // <-- 我们需要添加这个查询
			if err != nil {
				// 如果找不到用户（可能用户被删除了），销毁 session 并重定向
				session.Destroy(r.Context())
				http.Redirect(w, r, "/login", http.StatusSeeOther)
				return
			}

			// 将用户信息存入请求的 context 中
			ctx := context.WithValue(r.Context(), UserContextKey, user)

			// 使用带有用户信息的 context 继续处理请求
			next.ServeHTTP(w, r.WithContext(ctx))
		})
	}
}
