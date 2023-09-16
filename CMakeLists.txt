cmake_minimum_required(VERSION 3.20)

project(Raptor VERSION 1)

set(CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

add_executable(${PROJECT_NAME}
    main.cpp
)

# GLFW
set(GLFW_BUILD_D    OCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(Common/External/glfw)

# EASTL
set(EASTL_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/Common/External/EASTL")
include_directories (${EASTL_ROOT_DIR}/include)
include_directories (${EASTL_ROOT_DIR}/test/packages/EAAssert/include)
include_directories (${EASTL_ROOT_DIR}/test/packages/EABase/include/Common)
include_directories (${EASTL_ROOT_DIR}/test/packages/EAMain/include)
include_directories (${EASTL_ROOT_DIR}/test/packages/EAStdC/include)
include_directories (${EASTL_ROOT_DIR}/test/packages/EATest/include)
include_directories (${EASTL_ROOT_DIR}/test/packages/EAThread/include)

set(EASTL_LIBRARY debug ${EASTL_ROOT_DIR}/build/Debug/EASTL.lib optimized ${EASTL_ROOT_DIR}/build/Release/EASTL.lib)
add_custom_target(NatVis SOURCES ${EASTL_ROOT_DIR}/doc/EASTL.natvis)

# Vulkan
find_package(Vulkan REQUIRED)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    glfw
    ${EASTL_LIBRARY}
    "Vulkan::Vulkan"
)
