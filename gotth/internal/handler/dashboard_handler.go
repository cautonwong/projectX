package handler

import (
	"net/http"

	"gotth/internal/middleware" // <-- 新增导入
	"gotth/view/pages"
)

type DashboardHandler struct{}

func (h *DashboardHandler) HandleDashboardShow(w http.ResponseWriter, r *http.Request) {
	// 从 context 中安全地获取用户信息
	user, ok := middleware.CtxGetUser(r.Context())
	if !ok {
		// 如果因为某些原因获取不到用户，这是一个服务端错误
		http.Error(w, "Unauthorized", http.StatusUnauthorized)
		return
	}

	// 将用户信息传递给视图
	pages.Dashboard(user.Email).Render(r.Context(), w)
}
