package main

import (
	"context"
	"database/sql"
	"log"
	"path/filepath"

	"gotth/internal/auth"
	"gotth/internal/data"

	"github.com/go-faker/faker/v4"
	"github.com/google/uuid"
	_ "github.com/mattn/go-sqlite3"
)

func main() {
	log.Println("开始填充数据库...")

	// 使用绝对路径
	dbPath, err := filepath.Abs("app.db")
	if err != nil {
		log.Fatalf("无法获取数据库路径: %v", err)
	}
	log.Printf("使用数据库路径: %s", dbPath)

	// 直接使用 sql.Open 连接数据库
	db, err := sql.Open("sqlite3", dbPath)
	if err != nil {
		log.Fatalf("无法连接数据库: %v", err)
	}
	defer db.Close()

	// 验证数据库连接
	if err := db.Ping(); err != nil {
		log.Fatalf("数据库连接测试失败: %v", err)
	}
	log.Println("✅ 数据库连接成功")

	queries := data.New(db)
	ctx := context.Background()

	// 清空现有用户表
	_, err = db.Exec("DELETE FROM users")
	if err != nil {
		log.Fatalf("清空用户表失败: %v", err)
	}
	log.Println("✅ 已清空用户表")

	// 创建 50 个虚拟用户
	successCount := 0
	for i := 0; i < 50; i++ {
		hashedPassword, err := auth.HashPassword("password123")
		if err != nil {
			log.Printf("生成密码哈希失败: %v", err)
			continue
		}

		params := data.CreateUserParams{
			ID:           uuid.NewString(),
			Email:        faker.Email(),
			PasswordHash: hashedPassword,
		}

		_, err = queries.CreateUser(ctx, params)
		if err != nil {
			log.Printf("创建用户失败 %s: %v", params.Email, err)
			continue
		}
		successCount++
	}

	// 验证插入的数据
	var count int
	err = db.QueryRow("SELECT COUNT(*) FROM users").Scan(&count)
	if err != nil {
		log.Fatalf("查询用户数量失败: %v", err)
	}

	log.Printf("✅ 数据库填充完成！成功创建 %d 个用户", count)
}
