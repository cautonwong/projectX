cmake_minimum_required(VERSION 3.10)
project(msx C CXX)

# --- Find all required packages at the top ---
find_package(OpenSSL REQUIRED)
find_package(eclipse-paho-mqtt-c REQUIRED)
#find_package(yaml REQUIRED) 
find_package(libuv REQUIRED) 
find_package(leveldb REQUIRED)
find_package(Threads REQUIRED)
find_package(cJSON REQUIRED)
find_package(Libevent REQUIRED)
#get_target_property(LIBEVENT_CORE_INCLUDE_DIRS Libevent::core INTERFACE_INCLUDE_DIRECTORIES)
#get_target_property(LIBEVENT_EXTRA_INCLUDE_DIRS Libevent::extra INTERFACE_INCLUDE_DIRECTORIES)

find_package(CycloneDDS REQUIRED)
find_package(microhttpd REQUIRED)
find_package(CURL REQUIRED)
find_package(sqlite3 REQUIRED)
find_package(Poco REQUIRED Foundation Net Util JSON XML)
# --- End of find_package calls ---

add_library(${PROJECT_NAME} STATIC 
    service.c
    leveldb_manager.c
    sqlite_manager.c
    mqtt_client_manager.c
    http_server_manager.c
    http_client_manager.c
    docker_manager.c
)

set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

target_include_directories(${PROJECT_NAME} PUBLIC
    ../include
    /workspaces/projectX/.local/linux-x64/include/curl
    $<TARGET_PROPERTY:libuv::uv_a,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:cjson,INTERFACE_INCLUDE_DIRECTORIES>
    #$<TARGET_PROPERTY:yaml,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:OpenSSL::SSL,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:OpenSSL::Crypto,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:leveldb::leveldb,INTERFACE_INCLUDE_DIRECTORIES>
    ${Libevent_INCLUDE_DIRS}
    $<TARGET_PROPERTY:microhttpd::microhttpd,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:sqlite3::sqlite3,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:eclipse-paho-mqtt-c::paho-mqtt3a-static,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Poco::Foundation,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Poco::Net,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Poco::Util,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Poco::JSON,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:Poco::XML,INTERFACE_INCLUDE_DIRECTORIES>
)

set(SANITIZER_FLAGS "-fsanitize=undefined")
target_compile_options(${PROJECT_NAME} PUBLIC ${SANITIZER_FLAGS}) # cjson依赖此
target_link_options(${PROJECT_NAME} PUBLIC ${SANITIZER_FLAGS})

target_link_libraries(${PROJECT_NAME} PUBLIC
    -Wl,--whole-archive
    /workspaces/projectX/.local/linux-x64/lib/libcurl.a
    libuv::uv_a
    cjson
    #yaml
    rt
    z
    psl
    OpenSSL::SSL
    OpenSSL::Crypto
    leveldb::leveldb
    CycloneDDS::ddsc
    #libevent::core
    #libevent::extra
    microhttpd::microhttpd
    sqlite3::sqlite3
    eclipse-paho-mqtt-c::paho-mqtt3c-static # Ensure this is linked
    Poco::Foundation
    Poco::Net
    Poco::Util
    Poco::JSON
    -Wl,--no-whole-archive
)