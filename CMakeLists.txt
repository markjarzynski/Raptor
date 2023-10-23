cmake_minimum_required(VERSION 3.20)

# set the project name and version
project(Raptor VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQURIED True)

# set configure file
configure_file(Raptor.h.in Raptor.h @ONLY)

message(STATUS "Test: ${PROJECT_SOURCE_DIR}")

# cmake module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

find_package(Vulkan REQUIRED)
message(STATUS "Vulkan: ${Vulkan_LIBRARIES} ${Vulkan_INCLUDE_DIRS}")

find_package(VulkanMemoryAllocator REQUIRED)
message(STATUS "VulkanMemoryAllocator: ${VulkanMemoryAllocator_INCLUDE_DIR}")

#include(GLFW)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
find_package(GLFW REQUIRED)

include(EASTL)
include(ImGui)

add_executable(${PROJECT_NAME}
    main.cpp
)

# include_directories(${PROJECT_NAME}
# PUBLIC
#     ${CMAKE_CURRENT_BINARY_DIR}
#     ${CMAKE_CURRENT_SOURCE_DIR}/Graphics
#     ${CMAKE_CURRENT_SOURCE_DIR}/Debug
#     ${CMAKE_CURRENT_SOURCE_DIR}/Debug/UI
# )

add_subdirectory(Core)
add_subdirectory(Debug)
add_subdirectory(Graphics)
add_subdirectory(Debug/UI)

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${VulkanMemoryAllocator_INCLUDE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    EASTL
    EAStdC
    "Vulkan::Vulkan"
    #VulkanMemoryAllocator
    glfw
    imgui
    "Raptor::Core"
    "Raptor::Debug"
    "Raptor::Graphics"
    "Raptor::Debug::UI"
)
