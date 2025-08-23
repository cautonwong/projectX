if(NOT TARGET sqlite3::sqlite3)
    add_library(sqlite3::sqlite3 STATIC IMPORTED)
    set_target_properties(sqlite3::sqlite3 PROPERTIES
        IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/../../../lib/libsqlite3.a"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/../../../include"
    )
    
    # SQLite 可能需要的系统库
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set_property(TARGET sqlite3::sqlite3 PROPERTY
            INTERFACE_LINK_LIBRARIES "dl;pthread;m"
        )
    endif()
endif()
