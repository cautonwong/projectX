cmake_minimum_required(VERSION 3.21)

project(MyQtProductSuite LANGUAGES CXX)

# --- Global Settings ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#list(APPEND CMAKE_MODULE_PATH "D:/Qt/6.10.0/msvc2022_64/lib/cmake/Qt6")
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Gui
    Qml
    Quick
    QuickControls2
    Svg
    Test
    LinguistTools
)
qt_standard_project_setup()

# --- Find Packages ---

add_executable(main main.cpp)

target_link_libraries(main PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)
# --- Add all sub-modules ---
message(STATUS "Configuring Libraries...")
#add_subdirectory(libs/core)
#add_subdirectory(libs/ui-controls)
#add_subdirectory(libs/services)

message(STATUS "Configuring Applications...")
#add_subdirectory(apps/MainGuiApp)
#add_subdirectory(apps/CliTool)

message(STATUS "Configuring Tests...")
enable_testing()
#add_subdirectory(tests)

# --- Packaging (CPack) ---
# include(cmake/Packaging.cmake) # Optional for now