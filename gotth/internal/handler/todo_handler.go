package handler

import (
	"log"
	"net/http"

	"gotth/internal/data"
	"gotth/internal/middleware"
	"gotth/view/pages"
	"gotth/view/shared"

	"github.com/go-chi/chi/v5"
	"github.com/google/uuid"
)

type TodoHandler struct {
	DB data.Querier
}

func (h *TodoHandler) HandleTodosShow(w http.ResponseWriter, r *http.Request) {
	// 1. Get the current user from the context.
	// This is guaranteed to exist because this handler will be protected by our auth middleware.
	user, _ := middleware.CtxGetUser(r.Context())

	// 2. Fetch the todos for this user from the database.
	todos, err := h.DB.ListTodosByUser(r.Context(), user.ID)
	if err != nil {
		log.Printf("Error fetching todos for user %s: %v", user.ID, err)
		http.Error(w, "Failed to load todos", http.StatusInternalServerError)
		return
	}

	// 3. Render the Todos page component with the fetched data.
	pages.Todos(todos).Render(r.Context(), w)
}

func (h *TodoHandler) HandleTodoCreate(w http.ResponseWriter, r *http.Request) {
	// 1. Parse the task from the form submission.
	task := r.FormValue("task")
	if task == "" {
		// In a real app, you might return an error message to the user.
		http.Error(w, "Task cannot be empty", http.StatusBadRequest)
		return
	}

	// 2. Get the current user from the context.
	user, _ := middleware.CtxGetUser(r.Context())

	// 3. Create the new todo in the database.
	params := data.CreateTodoParams{
		ID:     uuid.NewString(),
		UserID: user.ID,
		Task:   task,
	}
	newTodo, err := h.DB.CreateTodo(r.Context(), params)
	if err != nil {
		log.Printf("Error creating todo: %v", err)
		http.Error(w, "Failed to create todo", http.StatusInternalServerError)
		return
	}

	// 4. Important: Instead of redirecting or returning JSON, we return an HTML fragment.
	// We render just the single TodoItem component with the newly created data.
	// HTMX will take this HTML and place it exactly where we told it to.
	shared.TodoItem(newTodo).Render(r.Context(), w)
}

func (h *TodoHandler) HandleTodoUpdate(w http.ResponseWriter, r *http.Request) {
	// 1. Get the ID of the todo from the URL path.
	id := chi.URLParam(r, "id")
	user, _ := middleware.CtxGetUser(r.Context())

	// 2. Fetch the current state of the todo to toggle its 'completed' status.
	currentTodo, err := h.DB.GetTodoByID(r.Context(), id)
	if err != nil {
		http.Error(w, "Todo not found", http.StatusNotFound)
		return
	}
	// Security check
	if currentTodo.UserID != user.ID {
		http.Error(w, "Unauthorized", http.StatusForbidden)
		return
	}

	// 3. Update the todo with the toggled status.
	params := data.UpdateTodoStatusParams{
		ID:        id,
		Completed: !currentTodo.Completed, // The toggle logic!
		UserID:    user.ID,
	}
	updatedTodo, err := h.DB.UpdateTodoStatus(r.Context(), params)
	if err != nil {
		log.Printf("Error updating todo: %v", err)
		http.Error(w, "Failed to update todo", http.StatusInternalServerError)
		return
	}

	// 4. Return the updated TodoItem component as an HTML fragment.
	shared.TodoItem(updatedTodo).Render(r.Context(), w)
}

func (h *TodoHandler) HandleTodoDelete(w http.ResponseWriter, r *http.Request) {
	// 1. Get the ID and user.
	id := chi.URLParam(r, "id")
	user, _ := middleware.CtxGetUser(r.Context())

	// You could add a security check here similar to the update handler if desired.

	// 2. Delete the todo from the database.
	err := h.DB.DeleteTodo(r.Context(), data.DeleteTodoParams{
		ID:     id,
		UserID: user.ID,
	})
	if err != nil {
		log.Printf("Error deleting todo: %v", err)
		http.Error(w, "Failed to delete todo", http.StatusInternalServerError)
		return
	}

	// 3. For a delete operation, we want the item to disappear.
	// We return an empty response, and HTMX will swap the element with nothing, effectively removing it.
	w.WriteHeader(http.StatusOK)
}
