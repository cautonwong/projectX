package handler

import (
	"encoding/json"
	"fmt"
	"log"
	"math"
	"net/http"
	"sort"
	"strconv"
	"time"

	"gotth/internal/data"
	"gotth/view/ui_lab"
	"gotth/view/ui_lab/sections"
)

type UILabHandler struct {
	DB data.Querier
}

func (h *UILabHandler) HandleUILabShow(w http.ResponseWriter, r *http.Request) {
	ui_lab.Index().Render(r.Context(), w)
}

// --- Buttons & Forms Section ---
func (h *UILabHandler) HandleButtonsFormsShow(w http.ResponseWriter, r *http.Request) {
	sections.ButtonsFormsExample().Render(r.Context(), w)
}

func (h *UILabHandler) HandleButtonLoadingExample(w http.ResponseWriter, r *http.Request) {
	// 1. 模拟一个耗时的操作，比如数据库查询或 API 调用
	time.Sleep(2 * time.Second)

	// 2. 操作完成后，返回一段简单的 HTML 片段
	// HTMX 会将这段 HTML 注入到 id="loading-response" 的 div 中
	w.WriteHeader(http.StatusOK)
	fmt.Fprintf(w, `<div class="text-success">✅ Request complete!</div>`)
}
func (h *UILabHandler) HandleFeedbackShow(w http.ResponseWriter, r *http.Request) {
	sections.FeedbackExample().Render(r.Context(), w)
}

// This handler returns ONLY the HTML fragment for the modal's content
func (h *UILabHandler) HandleModalContent(w http.ResponseWriter, r *http.Request) {
	// 模拟获取数据
	time.Sleep(1 * time.Second)

	// 直接在 handler 中写入一个简单的 HTML 片段作为响应
	// 在真实应用中，这里会调用一个 templ 组件
	html := `
        <h3 class="font-bold text-lg">Hello from the Server!</h3>
        <p class="py-4">This content was dynamically loaded into the modal.</p>
        <div class="modal-action">
            <form method="dialog">
                <button class="btn">Close</button>
            </form>
        </div>
    `
	w.WriteHeader(http.StatusOK)
	w.Write([]byte(html))
}

func (h *UILabHandler) HandleShowToast(w http.ResponseWriter, r *http.Request) {
	// 从请求中获取 toast 的类型和消息
	toastType := r.FormValue("type")
	message := r.FormValue("message")

	// 创建一个 map 来构造要发送的事件详情
	detail := map[string]string{
		"message": message,
		"type":    toastType,
	}

	// 将 map 序列化为 JSON 字符串
	detailJSON, err := json.Marshal(detail)
	if err != nil {
		http.Error(w, "Server error", http.StatusInternalServerError)
		return
	}

	// 设置 HX-Trigger 响应头
	// 我们触发一个名为 'show-toast' 的事件，并将 JSON 字符串作为其详情
	w.Header().Set("HX-Trigger", `{"show-toast": `+string(detailJSON)+`}`)
	w.WriteHeader(http.StatusOK)
}

const DefaultPageSize = 10

// --- Advanced Table Section ---
/*
func (h *UILabHandler) fetchTableData(r *http.Request) (sections.TableData, error) {
	// --- Step 1: Parse and validate the page number ---
	pageStr := r.URL.Query().Get("page")
	page, err := strconv.Atoi(pageStr)
	if err != nil || page < 1 {
		page = 1 // Default to page 1 if invalid or not provided
	}
	log.Printf("Fetching data for page: %d", page)

	// --- Step 2: Get the total number of users ---
	totalUsers, err := h.DB.CountUsers(r.Context())
	if err != nil {
		return sections.TableData{}, fmt.Errorf("failed to count users: %w", err)
	}
	log.Printf("Total users in DB: %d", totalUsers)

	// --- Step 3: Calculate LIMIT and OFFSET ---
	// Ensure types are correct for the sqlc query (int64)
	limit := int64(DefaultPageSize)
	offset := int64((page - 1) * DefaultPageSize)
	log.Printf("Querying with LIMIT: %d, OFFSET: %d", limit, offset)

	// --- Step 4: Fetch the paginated user data ---
	users, err := h.DB.ListUsersPaginated(r.Context(), data.ListUsersPaginatedParams{
		Limit:  limit,
		Offset: offset,
	})
	if err != nil {
		return sections.TableData{}, fmt.Errorf("failed to list users paginated: %w", err)
	}
	log.Printf("Found %d users for the current page.", len(users))

	// --- Step 5: Calculate total pages ---
	totalPages := int(math.Ceil(float64(totalUsers) / float64(DefaultPageSize)))
	log.Printf("Calculated total pages: %d", totalPages)

	return sections.TableData{
		Users:       users,
		TotalUsers:  totalUsers,
		CurrentPage: page,
		TotalPages:  totalPages,
		PageSize:    DefaultPageSize,
	}, nil
}
*/
func (h *UILabHandler) fetchTableData(r *http.Request) (sections.TableData, error) {
	// 1. Parse all query parameters from the request URL
	q := r.URL.Query()
	page, err := strconv.Atoi(q.Get("page"))
	if err != nil || page < 1 {
		page = 1
	}

	searchQuery := q.Get("search")
	sortBy := q.Get("sort_by")
	sortOrder := q.Get("sort_order")

	// Provide safe default values for sorting
	if sortBy == "" {
		sortBy = "created_at"
	}
	if sortOrder != "asc" && sortOrder != "desc" {
		sortOrder = "desc"
	}

	log.Printf("Fetching data for page: %d, search: '%s', sortBy: %s, sortOrder: %s", page, searchQuery, sortBy, sortOrder)

	// 2. Get the total count of users that match the search filter
	searchPattern := "%" + searchQuery + "%"
	countParams := data.CountUsersParams{
		Email:   searchPattern,
		Column2: searchQuery,
	}
	totalUsers, err := h.DB.CountUsers(r.Context(), countParams)
	if err != nil {
		return sections.TableData{}, fmt.Errorf("failed to count users: %w", err)
	}

	// 3. Get ALL users that match the search filter from the database
	getParams := data.GetAllUsersFilteredParams{
		Email:   searchPattern,
		Column2: searchQuery,
	}
	allFilteredUsers, err := h.DB.GetAllUsersFiltered(r.Context(), getParams)
	if err != nil {
		return sections.TableData{}, fmt.Errorf("failed to get all filtered users: %w", err)
	}

	// 4. Sort the retrieved users in memory using Go's sort package
	sort.SliceStable(allFilteredUsers, func(i, j int) bool {
		switch sortBy {
		case "email":
			if sortOrder == "asc" {
				return allFilteredUsers[i].Email < allFilteredUsers[j].Email
			}
			return allFilteredUsers[i].Email > allFilteredUsers[j].Email
		default: // Default sort by CreatedAt
			if sortOrder == "asc" {
				return allFilteredUsers[i].CreatedAt.Before(allFilteredUsers[j].CreatedAt)
			}
			return allFilteredUsers[i].CreatedAt.After(allFilteredUsers[j].CreatedAt)
		}
	})

	// 5. Manually paginate the sorted slice in memory
	start := (page - 1) * DefaultPageSize
	end := start + DefaultPageSize
	if start > len(allFilteredUsers) {
		start = len(allFilteredUsers)
	}
	if end > len(allFilteredUsers) {
		end = len(allFilteredUsers)
	}
	paginatedUsers := allFilteredUsers[start:end]

	// 6. Calculate total pages based on the filtered count
	totalPages := int(math.Ceil(float64(totalUsers) / float64(DefaultPageSize)))

	// 7. Return the complete state to the view
	return sections.TableData{
		Users:       paginatedUsers,
		TotalUsers:  totalUsers,
		CurrentPage: page,
		TotalPages:  totalPages,
		PageSize:    DefaultPageSize,
		SearchQuery: searchQuery,
		SortBy:      sortBy,
		SortOrder:   sortOrder,
	}, nil
}

func (h *UILabHandler) HandleTableShow(w http.ResponseWriter, r *http.Request) {
	data, err := h.fetchTableData(r)
	if err != nil {
		log.Printf("ERROR: 获取表格数据失败: %v", err)
		http.Error(w, "Failed to fetch data: "+err.Error(), http.StatusInternalServerError)
		return
	}

	if err := sections.TableExample(data).Render(r.Context(), w); err != nil {
		log.Printf("ERROR: 渲染表格失败: %v", err)
		http.Error(w, "Failed to render table", http.StatusInternalServerError)
	}
}

func (h *UILabHandler) HandleTableContent(w http.ResponseWriter, r *http.Request) {
	data, err := h.fetchTableData(r)
	if err != nil {
		log.Printf("ERROR: 获取表格内容失败: %v", err)
		http.Error(w, "Failed to fetch data: "+err.Error(), http.StatusInternalServerError)
		return
	}

	if err := sections.TableComponent(data).Render(r.Context(), w); err != nil {
		log.Printf("ERROR: 渲染表格组件失败: %v", err)
		http.Error(w, "Failed to render table component", http.StatusInternalServerError)
	}
}

func (h *UILabHandler) HandleSseShow(w http.ResponseWriter, r *http.Request) {
	sections.SseExample().Render(r.Context(), w)
}

func (h *UILabHandler) HandleSSE(w http.ResponseWriter, r *http.Request) {
	// 首先设置正确的 headers
	w.Header().Set("Content-Type", "text/event-stream")
	w.Header().Set("Cache-Control", "no-cache")
	w.Header().Set("Connection", "keep-alive")
	w.Header().Set("X-Accel-Buffering", "no") // 禁用 nginx 缓冲（如果使用 nginx）

	// 确保在设置任何 header 之后才写入数据
	flusher, ok := w.(http.Flusher)
	if !ok {
		http.Error(w, "SSE not supported", http.StatusInternalServerError)
		return
	}

	// 立即发送一个初始消息并刷新
	fmt.Fprintf(w, "event: message\ndata: 0\n\n")
	flusher.Flush()

	// 设置定时器
	count := 0
	ticker := time.NewTicker(1 * time.Second)
	defer ticker.Stop()

	// 监听连接关闭
	ctx := r.Context()

	for {
		select {
		case <-ctx.Done():
			return
		case <-ticker.C:
			count++
			// 确保消息格式正确：每个消息必须以\n\n结尾
			fmt.Fprintf(w, "event: message\ndata: %d\n\n", count)
			flusher.Flush()
		}
	}
}
