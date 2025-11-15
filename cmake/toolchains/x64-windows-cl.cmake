# ==============================================================================
# windows_cl_toolchain.cmake
#
# A CMake toolchain file for building with cl.exe on Windows.
# To use this file, run CMake from a Visual Studio developer command prompt:
# cmake -DCMAKE_TOOLCHAIN_FILE=windows_cl_toolchain.cmake ..
# ==============================================================================

# 指定系统名称和编译器语言
#include(D:\env\BuildTools\VC\vcpkg\scripts\buildsystems\vcpkg.cmake)
set(__target x64)
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_FIND_ROOT_PATH ${BuildTools_ROOT}/${BuildTools_VERSION}/bin/Hostx64/${__target})
set(CMAKE_ASM_COMPILER ${CMAKE_FIND_ROOT_PATH}/cl.exe)
set(CMAKE_C_COMPILER ${CMAKE_FIND_ROOT_PATH}/cl.exe)
set(CMAKE_CXX_COMPILER ${CMAKE_FIND_ROOT_PATH}/cl.exe)
set(CMAKE_AR ${CMAKE_FIND_ROOT_PATH}/lib.exe)
set(CMAKE_LINKER ${CMAKE_FIND_ROOT_PATH}/linker.exe)
set(CMAKE_MT ${WindowsSDK_ROOT}/bin/${WindowsSDK_VERSION}/${__target}/mt.exe)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

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
#set(CMAKE_C_FLAGS_INIT "/W4 /WX /std:c11")
#set(CMAKE_CXX_FLAGS_INIT "/W4 /WX /std:c++17")

# 设置调试和发布模式的额外标志
set(CMAKE_C_FLAGS_DEBUG_INIT "/Od /Zi /RTC1")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "/Od /Zi /RTC1")
set(CMAKE_C_FLAGS_RELEASE_INIT "/O2 /DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "/O2 /DNDEBUG")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_C_FLAGS "/wd4819" CACHE INTERNAL "c compiler flags")
set(CMAKE_CXX_FLAGS "/wd4819 /EHsc" CACHE INTERNAL "cxx compiler flags")


set(TOOLCHAIN_INCLUDEPATH 
    "${BuildTools_ROOT}/${BuildTools_VERSION}/include"
    "${WindowsSDK_ROOT}/Include/${WindowsSDK_VERSION}/ucrt"
    "${WindowsSDK_ROOT}/Include/${WindowsSDK_VERSION}/shared"
    "${WindowsSDK_ROOT}/Include/${WindowsSDK_VERSION}/um"
    "${WindowsSDK_ROOT}/Include/${WindowsSDK_VERSION}/winrt"
    "${WindowsSDK_ROOT}/Include/${WindowsSDK_VERSION}/cppwinrt"
)

set(TOOLCHAIN_LIBRARYPATH 
    "${BuildTools_ROOT}/${BuildTools_VERSION}/lib/${__target}"
    "${WindowsSDK_ROOT}/Lib/${WindowsSDK_VERSION}/ucrt/${__target}"
    "${WindowsSDK_ROOT}/Lib/${WindowsSDK_VERSION}/um/${__target}"
)