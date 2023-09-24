# EASTL
set(EASTL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Common/External/EASTL")

list(APPEND CMAKE_MODULE_PATH "${EASTL_ROOT_DIR}/scripts/CMake")
add_subdirectory(${EASTL_ROOT_DIR})
add_subdirectory(${EASTL_ROOT_DIR}/test/packages/EAAssert)
add_subdirectory(${EASTL_ROOT_DIR}/test/packages/EAThread)
add_subdirectory(${EASTL_ROOT_DIR}/test/packages/EAStdC)