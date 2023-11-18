project(Application)

add_library(${PROJECT_NAME})
add_library("Raptor::${PROJECT_NAME}" ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Window.cpp
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Window.h
)

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${Vulkan_INCLUDE_DIRS}
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    EASTL
    "Vulkan::Vulkan"
    glfw
    "Raptor::Core"
)