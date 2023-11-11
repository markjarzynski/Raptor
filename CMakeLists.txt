cmake_minimum_required(VERSION 3.20)

# set the project name and version
project(Raptor VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQURIED True)

# set configure file
configure_file(Raptor.h.in Raptor.h @ONLY)

# cmake module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

# Find required packages.
find_package(Vulkan REQUIRED)
find_package(VulkanMemoryAllocator REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
find_package(GLFW REQUIRED)

include(EASTL)
include(ImGui)
include(wyhash)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Common/External/tinygltf)

add_executable(${PROJECT_NAME}
    main.cpp
)

add_subdirectory(Core)
add_subdirectory(Debug)
add_subdirectory(Math)
add_subdirectory(Graphics)
add_subdirectory(Debug/UI)


target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR} # Required for Raptor.h to be found.
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    EASTL
    EAStdC
    "Vulkan::Vulkan"
    glfw
    imgui
    tinygltf
    "Raptor::Core"
    "Raptor::Debug"
    "Raptor::Math"
    "Raptor::Graphics"
    "Raptor::Debug::UI"
)
