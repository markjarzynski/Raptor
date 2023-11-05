# wyhash
set(WYHASH_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Common/External/wyhash")
set(WYHASH_INCLUDE_DIR ${WYHASH_ROOT_DIR})
include_directories (${WYHASH_INCLUDE_DIR})

add_library(wyhash INTERFACE)
add_library(wyhash::wyhash ALIAS wyhash)

target_include_directories(wyhash INTERFACE "${WYHASH_INCLUDE_DIR}")