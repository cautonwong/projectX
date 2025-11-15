package middleware

import (
	"context"
	"gotth/internal/data" // <-- 确保 "gotth" 是你的模块名
)

// 定义一个自定义类型作为 context 的 key，防止与其他包的 key 冲突
type contextKey string

// 这是我们将用来在 context 中存储用户信息的 key
const UserContextKey = contextKey("user")

// CtxGetUser 是一个帮助函数，可以方便地从 context 中获取用户信息
// 将 context.T 修改为 context.Context
func CtxGetUser(ctx context.Context) (data.User, bool) {
	user, ok := ctx.Value(UserContextKey).(data.User)
	return user, ok
}
