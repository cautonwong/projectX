package auth

import "golang.org/x/crypto/bcrypt"

// HashPassword 将明文密码转换为安全的 bcrypt 哈希值
func HashPassword(password string) (string, error) {
	// bcrypt.GenerateFromPassword 自动处理加盐和哈希过程
	// 14 是哈希的成本因子，数值越高越安全，但计算也越慢。12-14 是一个很好的平衡点。
	bytes, err := bcrypt.GenerateFromPassword([]byte(password), 14)
	return string(bytes), err
}

// CheckPasswordHash 比较明文密码和哈希值是否匹配
func CheckPasswordHash(password, hash string) bool {
	// bcrypt.CompareHashAndPassword 可以防止时序攻击 (timing attacks)
	err := bcrypt.CompareHashAndPassword([]byte(hash), []byte(password))
	return err == nil
}
