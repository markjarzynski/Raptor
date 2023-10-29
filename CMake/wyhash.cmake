# wyhash
set(WYHASH_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Common/External/wyhash")
include_directories (${WYHASH_ROOT_DIR})

add_library(wyhash INTERFACE)
add_library(wyhash::wyhash ALIAS wyhash)

target_include_directories(wyhash INTERFACE "${WYHASH_ROOT_DIR}")