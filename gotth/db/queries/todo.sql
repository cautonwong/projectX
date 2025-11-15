-- name: ListTodosByUser :many
SELECT * FROM todos
WHERE user_id = ?
ORDER BY created_at DESC;

-- name: CreateTodo :one
INSERT INTO todos (id, user_id, task)
VALUES (?, ?, ?)
RETURNING *;

-- name: UpdateTodoStatus :one
UPDATE todos
SET completed = ?
WHERE id = ? AND user_id = ?
RETURNING *;

-- name: DeleteTodo :exec
DELETE FROM todos
WHERE id = ? AND user_id = ?;

-- name: GetTodoByID :one
SELECT * FROM todos
WHERE id = ? LIMIT 1;