project(Debug)

add_library(${PROJECT_NAME})
add_library("Raptor::${PROJECT_NAME}" ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Log.cpp
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Debug.h
    ${CMAKE_CURRENT_LIST_DIR}/Log.h
)

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    EASTL
    EAStdC
)