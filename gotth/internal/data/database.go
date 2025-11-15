package data

import (
	"database/sql"
	"fmt"

	"github.com/golang-migrate/migrate/v4"
	"github.com/golang-migrate/migrate/v4/database/sqlite"
	_ "github.com/golang-migrate/migrate/v4/source/file" // 必需的驱动
	_ "github.com/mattn/go-sqlite3"                      // 必需的驱动
)

// InitDB 初始化数据库连接并运行迁移
func InitDB(dsn string) (*sql.DB, error) {
	// 1. 使用 DSN (数据源名称) 连接到 SQLite 数据库。
	// 如果文件不存在，会自动创建。
	db, err := sql.Open("sqlite3", dsn)
	if err != nil {
		return nil, fmt.Errorf("无法打开数据库连接: %w", err)
	}

	// 2. 验证与数据库的连接是否真实可用
	if err := db.Ping(); err != nil {
		return nil, fmt.Errorf("无法 ping 通数据库: %w", err)
	}

	fmt.Println("✅ 数据库连接成功")

	// 3. 运行数据库迁移
	if err := runMigrations(db); err != nil {
		return nil, fmt.Errorf("数据库迁移失败: %w", err)
	}

	return db, nil
}

func runMigrations(db *sql.DB) error {
	// 使用数据库实例创建一个新的 migrate 驱动
	driver, err := sqlite.WithInstance(db, &sqlite.Config{})
	if err != nil {
		return fmt.Errorf("无法创建 migrate driver 实例: %w", err)
	}

	// 指定迁移文件的位置并创建 migrate 实例
	m, err := migrate.NewWithDatabaseInstance(
		"file://./db/migrations",
		"sqlite", // 数据库的名称
		driver,
	)
	if err != nil {
		return fmt.Errorf("无法创建 migrate 实例: %w", err)
	}

	// 执行 'up' 迁移。如果数据库已经是最新版本，它会返回 migrate.ErrNoChange，
	// 我们需要忽略这个特定的“错误”。
	if err := m.Up(); err != nil && err != migrate.ErrNoChange {
		return err
	}

	fmt.Println("✅ 数据库迁移完成")
	return nil
}
