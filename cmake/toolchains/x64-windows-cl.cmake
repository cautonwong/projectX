# ==============================================================================
# windows_cl_toolchain.cmake
#
# A CMake toolchain file for building with cl.exe on Windows.
# To use this file, run CMake from a Visual Studio developer command prompt:
# cmake -DCMAKE_TOOLCHAIN_FILE=windows_cl_toolchain.cmake ..
# ==============================================================================

# 指定系统名称和编译器语言
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_C_COMPILER "cl.exe" CACHE STRING "The C compiler to use" FORCE)
set(CMAKE_CXX_COMPILER "cl.exe" CACHE STRING "The C++ compiler to use" FORCE)

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
set(CMAKE_C_COMPILER_FORCED TRUE)
set(CMAKE_CXX_COMPILER_FORCED TRUE)

# 设置一些有用的警告标志
set(CMAKE_C_FLAGS_INIT "/W4 /WX /std:c11")
set(CMAKE_CXX_FLAGS_INIT "/W4 /WX /std:c++17")

# 设置调试和发布模式的额外标志
set(CMAKE_C_FLAGS_DEBUG_INIT "/Od /Zi /RTC1")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "/Od /Zi /RTC1")
set(CMAKE_C_FLAGS_RELEASE_INIT "/O2 /DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "/O2 /DNDEBUG")