-- name: GetUserByEmail :one
SELECT * FROM users
WHERE email = ? LIMIT 1;

-- name: CreateUser :one
INSERT INTO users (id, email, password_hash)
VALUES (?, ?, ?)
RETURNING *;

-- name: GetUserByID :one
SELECT * FROM users
WHERE id = ? LIMIT 1;

-- name: GetAllUsersFiltered :many
-- This query fetches ALL users that match the search filter.
-- Sorting and pagination will be handled in the Go code.
SELECT * FROM users
WHERE
    -- The second '?' parameter is the same as the search query.
    -- If the search query is empty (''), the second part of the OR becomes TRUE,
    -- effectively returning all users.
    email LIKE ? OR ? = '';

-- name: CountUsers :one
-- This query counts ALL users that match the search filter.
-- This is used to calculate the total number of pages.
SELECT count(*) FROM users
WHERE
    email LIKE ? OR ? = '';