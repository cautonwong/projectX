package handler

import (
	"net/http"

	"gotth/view/pages"

	"github.com/a-h/templ"
)

type HomeHandler struct{}

func NewHomeHandler() *HomeHandler {
	return &HomeHandler{}
}

func (h *HomeHandler) HandleHomeShow(w http.ResponseWriter, r *http.Request) {
	templ.Handler(pages.Home("Go + Templ Boilerplate")).ServeHTTP(w, r)
}
