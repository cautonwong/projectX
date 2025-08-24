cmake_minimum_required(VERSION 3.10)
project(core_data C CXX)

add_executable(core_data core-data.c)

target_link_libraries(core_data PRIVATE
    msx
)

# Add sanitizer flags for linking
target_link_options(core_data PRIVATE "-fsanitize=undefined")