# DearImGui
set(IMGUI_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Common/External/imgui")
include_directories (${IMGUI_ROOT_DIR})

add_library (imgui
    ${IMGUI_ROOT_DIR}/imgui.cpp
    ${IMGUI_ROOT_DIR}/imgui_draw.cpp
    ${IMGUI_ROOT_DIR}/imgui_tables.cpp
    ${IMGUI_ROOT_DIR}/imgui_widgets.cpp
    ${IMGUI_ROOT_DIR}/imgui_demo.cpp
    ${IMGUI_ROOT_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_ROOT_DIR}/backends/imgui_impl_vulkan.cpp
)

add_library(imgui::imgui ALIAS imgui)
target_include_directories(imgui PUBLIC "${IMGUI_ROOT_DIR}" "${IMGUI_ROOT_DIR}/backends")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Common/External/glfw)

message(STATUS "GLFW: " ${CMAKE_CURRENT_SOURCE_DIR}/Common/External/glfw)

target_link_libraries(imgui 
    glfw
    "Vulkan::Vulkan"
)