/** @type {import('tailwindcss').Config} */
module.exports = {
  content: [
    "./**/*.templ",    // 扫描 Templ 文件
    "./**/*.html",     // 扫描 HTML 文件
    "./**/*.go",       // 扫描 Go 文件（如果在字符串中包含类名）
  ],
  theme: {
    extend: {},
  },
  // 添加 DaisyUI 插件
  plugins: [require("daisyui")],

  // 可选：配置 DaisyUI 主题
  daisyui: {
    themes: [
      "light",          // 默认主题
      "dark",
      "cupcake",
      "bumblebee",
      "emerald",
      "corporate",
      "synthwave",
      "retro",
      "cyberpunk",
      "valentine",
      "halloween",
      "garden",
      "forest",
      "aqua",
      "lofi",
      "pastel",
      "fantasy",
      "wireframe",
      "black",
      "luxury",
      "dracula",
      "cmyk",
      "autumn",
      "business",
      "acid",
      "lemonade",
      "night",
      "coffee",
      "winter"
    ], // true: 所有主题 | false: 仅 light + dark | array: 特定主题
    darkTheme: "dark", // 默认暗色主题
    base: true,        // 应用基础样式
    styled: true,      // 包含 DaisyUI 的样式
    utils: true,       // 添加响应式和工具类
    prefix: "",        // 类名前缀（可用于避免冲突）
    logs: true,        // 在构建时显示日志
    themeRoot: ":root", // 主题根元素选择器
  },
}