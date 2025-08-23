# --- CPM.cmake setup ---
# Include CPM.cmake script. You can either add it to your repository or download it on the fly.
# This example downloads it if not found.
set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cpm/CPM.cmake")
if(NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    message(STATUS "Downloading CPM.cmake...")
    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/get_cpm.cmake
        ${CPM_DOWNLOAD_LOCATION}
    )
endif()
include(${CPM_DOWNLOAD_LOCATION})

# --- Fetch Zephyr using CPM ---
# Specify the Zephyr repository and the desired tag/commit.
CPMAddPackage(
    NAME zephyr
    GIT_REPOSITORY https://github.com/zephyrproject-rtos/zephyr.git
    GIT_TAG        v4.2.0 # Or any other tag/commit
)

# --- Set ZEPHYR_BASE ---
# Set the ZEPHYR_BASE to the path where CPM has downloaded Zephyr.
# The source directory is available in the variable `zephyr_SOURCE_DIR`.
set(ZEPHYR_BASE ${zephyr_SOURCE_DIR})
message(STATUS "ZEPHYR_BASE set to: ${ZEPHYR_BASE}")