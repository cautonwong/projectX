# ==============================================================================
# windows_clang_toolchain.cmake
#
# A CMake toolchain file for building with Clang on Windows.
# To use this file, run CMake from a Visual Studio developer command prompt:
# cmake -DCMAKE_TOOLCHAIN_FILE=windows_clang_toolchain.cmake ..
# ==============================================================================

# 指定系统名称和编译器语言
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER "clang-cl.exe" CACHE STRING "The C compiler to use" FORCE)
set(CMAKE_CXX_COMPILER "clang-cl.exe" CACHE STRING "The C++ compiler to use" FORCE)

# 设置默认构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose build type" FORCE)
endif()

# 强制 CMake 重新检测编译器
unset(CMAKE_C_COMPILER_WORKS CACHE)
unset(CMAKE_CXX_COMPILER_WORKS CACHE)

# ------------------------------------------------------------------------------
# 编译器和链接器选项
# ------------------------------------------------------------------------------
# 共享 Visual Studio 的头文件和库
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

# 确保在 Visual Studio 的环境中运行 CMake，以便找到 vc runtime 和库
# 链接器使用 lld-link，它通常更快
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES> -fuse-ld=lld-link")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES> -fuse-ld=lld-link")

# 编译器选项
set(CMAKE_C_FLAGS_INIT "/W4 /WX /std:c11 /diagnostics:caret")
set(CMAKE_CXX_FLAGS_INIT "/W4 /WX /std:c++17 /diagnostics:caret")

# ------------------------------------------------------------------------------
# 可选：Sanitizers for finding bugs
# ------------------------------------------------------------------------------
# Clang on Windows supports sanitizers like AddressSanitizer.
# Uncomment the following lines to enable it.
# You need to manually link the Asan runtime library.
# if(USE_ASAN)
#    message(STATUS "Enabling Address Sanitizer")
#    add_compile_options(-fsanitize=address)
#    add_link_options(-fsanitize=address)
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
# endif()