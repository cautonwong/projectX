add_library(Services
    LoginService.cpp
)

target_link_libraries(Services PRIVATE
    Core
    Qt6::Core
)