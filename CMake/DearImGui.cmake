# DearImGui
set(IMGUI_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Common/External/imgui")
include_directories (${IMGUI_ROOT_DIR})

add_library (imgui
    ${IMGUI_ROOT_DIR}/imgui.cpp
    ${IMGUI_ROOT_DIR}/imgui_draw.cpp
    ${IMGUI_ROOT_DIR}/imgui_tables.cpp
    ${IMGUI_ROOT_DIR}/imgui_widgets.cpp
    ${IMGUI_ROOT_DIR}/imgui_demo.cpp
)

add_library(imgui::imgui ALIAS imgui)
target_include_directories(imgui PUBLIC "${IMGUI_ROOT_DIR}")