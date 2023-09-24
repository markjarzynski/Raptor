cmake_minimum_required(VERSION 3.20)

# set the project name and version
project(Raptor VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQURIED True)

# set configure file
configure_file(Config.h.in Config.h)

# cmake module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

find_package(Vulkan REQUIRED)

#include(GLFW)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
find_package(GLFW REQUIRED)

include(EASTL)
include(ImGui)

#find_package(IMGUI REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    Graphics/Window.h Graphics/Window.cpp
    Graphics/Vulkan.h Graphics/Vulkan.cpp
    Debug/Log.h Debug/Log.cpp
    Debug/UI/DebugUI.h Debug/UI/DebugUI.cpp
)

include_directories(${PROJECT_NAME}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Graphics
    ${CMAKE_CURRENT_SOURCE_DIR}/Debug
    ${CMAKE_CURRENT_SOURCE_DIR}/Debug/UI
)

add_subdirectory(Debug)
add_subdirectory(Graphics)
add_subdirectory(Debug/UI)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    EASTL
    EAAssert
    EAThread
    EAStdC
    glfw
    imgui
    "Vulkan::Vulkan"
    "Raptor::Debug"
    "Raptor::Graphics"
    "Raptor::Debug::UI"
)
