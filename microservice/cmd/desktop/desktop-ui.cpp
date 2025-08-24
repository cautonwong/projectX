#include <functional>
#include <stdio.h>
#include <string>

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include <uv.h>

// 全局变量
GLFWwindow *window = nullptr;
uv_loop_t *loop = nullptr;
bool needs_render = true;

// 渲染函数
void render_frame() {
  if (!needs_render) {
    return;
  }

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::Begin("Event-Driven Render with Chinese Font");
  ImGui::Text("Hello World!");
  ImGui::Text("你好，世界！"); // 显示中文字符

  // 如果需要，可以在这里手动切换字体
  // ImGui::PushFont(io.Fonts->Fonts[1]); // 假设第二个字体是中文
  // ImGui::Text("你好，世界！");
  // ImGui::PopFont();

  ImGui::End();

  ImGui::Render();
  int display_w, display_h;
  glfwGetFramebufferSize(window, &display_w, &display_h);
  glViewport(0, 0, display_w, display_h);
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
  glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwSwapBuffers(window);

  needs_render = false;
}

// 事件回调
void cursor_pos_callback(GLFWwindow *win, double xpos, double ypos) {
  ImGui_ImplGlfw_CursorPosCallback(win, xpos, ypos);
  needs_render = true;
  glfwPostEmptyEvent();
}
// ... 其他回调函数 ...
void mouse_button_callback(GLFWwindow *win, int button, int action, int mods) {
  ImGui_ImplGlfw_MouseButtonCallback(win, button, action, mods);
  needs_render = true;
  glfwPostEmptyEvent();
}
void key_callback(GLFWwindow *win, int key, int scancode, int action,
                  int mods) {
  ImGui_ImplGlfw_KeyCallback(win, key, scancode, action, mods);
  needs_render = true;
  glfwPostEmptyEvent();
}
void scroll_callback(GLFWwindow *win, double xoffset, double yoffset) {
  ImGui_ImplGlfw_ScrollCallback(win, xoffset, yoffset);
  needs_render = true;
  glfwPostEmptyEvent();
}
void framebuffer_size_callback(GLFWwindow *win, int width, int height) {
  ImGui_ImplGlfw_FramebufferSizeCallback(win, width, height);
  needs_render = true;
  glfwPostEmptyEvent();
}

int main(int, char **) {
  // ... 初始化 GLFW ...
  if (!glfwInit())
    return 1;
  window = glfwCreateWindow(1280, 720, "Event-Driven Render with Chinese Font",
                            nullptr, nullptr);
  if (!window)
    return 1;
  glfwMakeContextCurrent(window);

  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  // ===========================================
  // 加载字体
  // ===========================================
  // 1. 加载主字体 (JetBrains Mono)
  const char *jetbrains_mono_path = "JetBrainsMono-Regular.ttf";
  ImFont *jetbrains_font =
      io.Fonts->AddFontFromFileTTF(jetbrains_mono_path, 16.0f);

  // 2. 加载微软雅黑字体，并设置为备用（Fallback）字体
  // 使用 Windows 默认路径
  const char *msyh_path = "C:/Windows/Fonts/msyh.ttc";
  ImFontConfig font_config;
  font_config.MergeMode = true;  // 启用合并模式
  font_config.PixelSnapH = true; // 像素对齐

  static const ImWchar ranges[] = {
      0x0020, 0x00FF, // Basic Latin + Latin-1 Supplement
      0x4E00, 0x9FA5, // CJK Unified Ideographs
      0,
  };

  ImFont *chinese_font =
      io.Fonts->AddFontFromFileTTF(msyh_path, 16.0f, &font_config, ranges);

  if (jetbrains_font) {
    // 将 JetBrains Mono 设置为主字体
    io.FontDefault = jetbrains_font;
    // 如果 JetBrains Mono 加载失败，fallback 到中文字体
    if (chinese_font) {
      jetbrains_font->FallbackChar = '?'; // Fallback 字符，可选
    }
  } else {
    // 如果 JetBrains Mono 加载失败，直接使用中文字体
    if (chinese_font) {
      io.FontDefault = chinese_font;
    } else {
      fprintf(stderr,
              "Failed to load both JetBrains Mono and Chinese fonts.\n");
      io.Fonts->AddFontDefault();
    }
  }

  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 130");

  // ===========================================
  // 设置所有事件回调
  // ===========================================
  glfwSetKeyCallback(window, key_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetCursorPosCallback(window, cursor_pos_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // ===========================================
  // 主循环
  // ===========================================
  while (!glfwWindowShouldClose(window)) {
    glfwWaitEvents();
    render_frame();
  }

  // ... 清理代码 ...
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}