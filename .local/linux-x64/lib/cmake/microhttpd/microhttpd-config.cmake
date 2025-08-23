if(NOT TARGET microhttpd::microhttpd)
    add_library(microhttpd::microhttpd STATIC IMPORTED)
    set_target_properties(microhttpd::microhttpd PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../../../lib/libmicrohttpd.a"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../../../include"
    )
endif()
