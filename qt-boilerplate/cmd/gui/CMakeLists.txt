file(GLOB_RECURSE QML_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/qml/*.qml"
    "${CMAKE_CURRENT_SOURCE_DIR}/qml/*.qmldir"
)

qt_add_qml_module(MainGuiAppModule
    URI "MainGuiApp"
    VERSION 1.0
    QML_FILES ${QML_FILES}
)

qt_add_executable(MainGuiApp main.cpp)

target_link_libraries(MainGuiApp PRIVATE
    Core
    Services
    MainGuiAppModule
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

# Deployment script
qt_generate_deploy_app_script(
    TARGET MainGuiApp
    OUTPUT_SCRIPT deploy_script.cmake
    NO_UNSUPPORTED_PLATFORM_ERROR
)

add_custom_command(
    TARGET MainGuiApp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/deploy_script.cmake"
    COMMENT "Deploying Qt & QML dependencies..."
)