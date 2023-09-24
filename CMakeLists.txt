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

include(GLFW)
include(EASTL)
include(DearImGui)

find_package(Vulkan REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    Graphics/Window.h
    Graphics/Window.cpp
)

include_directories(${PROJECT_NAME}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    glfw
    ${EASTL_LIBRARY}
    "Vulkan::Vulkan"
    imgui
)
