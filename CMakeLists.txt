cmake_minimum_required(VERSION 3.14)
project(uv_plus 
    VERSION 1.0.0
    LANGUAGES C
    DESCRIPTION "Asynchronous Database and Network Framework"
)
# 定义平台和架构变量
if(WIN32)
    set(PLATFORM "windows")
else()
    set(PLATFORM "linux")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH "x64")
else()
    set(ARCH "x86")
endif()

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTING "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)
option(ENABLE_SANITIZERS "Enable sanitizers in debug mode" ON)
set(SANITIZER_TYPE "address" CACHE STRING "Type of sanitizer (address, undefined)")

# 获取顶层目录的绝对路径
get_filename_component(PROJECT_SOURCE_DIR ${CMAKE_SOURCE_DIR} ABSOLUTE)
# 定义平台和架构变量
if(WIN32)
    set(PLATFORM "windows")
else()
    set(PLATFORM "linux")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH "x64")
else()
    set(ARCH "x86")
endif()
set(LOCAL_PREFIX_PATH "${PROJECT_SOURCE_DIR}/.local")
#set(OPENSSL_ROOT_DIR "/usr")
#set(OPENSSL_CRYPTO_LIBRARY "/usr/lib/x86_64-linux-gnu/libcrypto.so")
#set(OPENSSL_SSL_LIBRARY "/usr/lib/x86_64-linux-gnu/libssl.so")

set(ZLIB_ROOT_DIR "/usr")
set(ZLIB_LIBRARY "/usr/lib/x86_64-linux-gnu/libz.so")
set(ZLIB_INCLUDE_DIR "/usr/include")
#set(CMAKE_PREFIX_PATH 
    #${LOCAL_PREFIX_PATH}/${PLATFORM}-${ARCH}/libyaml
    #${LOCAL_PREFIX_PATH}/${PLATFORM}-${ARCH}/Libevent
   #${LOCAL_PREFIX_PATH}/${PLATFORM}-${ARCH}/libuv
    #${CMAKE_PREFIX_PATH}
#)
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}/.local/${PLATFORM}-${ARCH}")
message(STATUS ${PROJECT_SOURCE_DIR})
#add_subdirectory(cmd)
#add_subdirectory(third-party/libmsx)
#add_subdirectory(cmd/firmware/boilerplate)
add_subdirectory(microservice)