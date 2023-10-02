project(DebugUI)

add_library(${PROJECT_NAME})
add_library("Raptor::Debug::UI" ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/DebugUI.cpp
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/DebugUI.h
)

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    EASTL
    EAStdC
    imgui
    "Raptor::Core"
    "Raptor::Debug"
    "Raptor::Graphics"
)