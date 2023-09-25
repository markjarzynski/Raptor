project(Graphics)

add_library(${PROJECT_NAME})
add_library("Raptor::${PROJECT_NAME}" ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Vulkan.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Window.cpp
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Vulkan.h
    ${CMAKE_CURRENT_LIST_DIR}/Window.h
)

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    EASTL
    EAStdC
    "Vulkan::Vulkan"
    imgui
    "Raptor::Core"
    "Raptor::Debug"
)