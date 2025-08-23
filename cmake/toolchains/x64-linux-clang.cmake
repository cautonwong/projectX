# ==============================================================================
# clang_toolchain.cmake
#
# A CMake toolchain file for building projects with Clang on Linux.
# To use this file, run CMake with the following command:
#   cmake -DCMAKE_TOOLCHAIN_FILE=clang_toolchain.cmake ..
# ==============================================================================

# ------------------------------------------------------------------------------
# Set the default build type to RelWithDebInfo
# ------------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose build type" FORCE)
endif()

# ------------------------------------------------------------------------------
# Specify the C and C++ compilers
# ------------------------------------------------------------------------------
set(CMAKE_C_COMPILER "clang" CACHE STRING "The C compiler to use" FORCE)
set(CMAKE_CXX_COMPILER "clang++" CACHE STRING "The C++ compiler to use" FORCE)

# Force CMake to re-detect the compilers
unset(CMAKE_C_COMPILER_WORKS CACHE)
unset(CMAKE_CXX_COMPILER_WORKS CACHE)

# ------------------------------------------------------------------------------
# Specify the linker
# ------------------------------------------------------------------------------
# Use lld linker for faster linking times.
set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_C_COMPILER> -fuse-ld=lld <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
set(CMAKE_CXX_LINK_EXECUTABLE "<CMAKE_CXX_COMPILER> -fuse-ld=lld <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")

# ------------------------------------------------------------------------------
# Enable some useful compiler flags for Clang
# ------------------------------------------------------------------------------
# The 'CMAKE_POLICY_DEFAULT_CMP0092' is required to correctly set the sanitizer flags.
cmake_policy(SET CMP0092 NEW)

# Common flags for warnings and strictness
set(CMAKE_C_FLAGS_INIT "-Wall -Wextra -Wpedantic -Wconversion")
set(CMAKE_CXX_FLAGS_INIT "-Wall -Wextra -Wpedantic -Wconversion")

# Linker flags
set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld")

# Release build flags
set(CMAKE_C_FLAGS_RELEASE_INIT "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O2 -DNDEBUG")

# Debug build flags
set(CMAKE_C_FLAGS_DEBUG_INIT "-O0 -g")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-O0 -g")

# RelWithDebInfo build flags
set(CMAKE_C_FLAGS_RELWITHDEBINFO_INIT "-O2 -g")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO_INIT "-O2 -g")

# ------------------------------------------------------------------------------
# Optional: Sanitizers for finding bugs
# ------------------------------------------------------------------------------
# These flags are commented out by default.
# To enable them, uncomment the lines below and adjust as needed.

# Set these flags via the command line:
# cmake -DCMAKE_TOOLCHAIN_FILE=clang_toolchain.cmake -DUSE_SANITIZERS=ON ..

# if(USE_SANITIZERS)
#     set(SANITIZER_FLAGS "-fsanitize=address,undefined,leak -fno-omit-frame-pointer")
#     message(STATUS "Enabled Sanitizers: address, undefined, leak")
# 
#     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
# 
#     # You may also need to set linker flags, but -fuse-ld=lld should handle most cases.
#     # set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
# endif()