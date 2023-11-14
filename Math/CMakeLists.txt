project(Math)

add_library(${PROJECT_NAME})
add_library("Raptor::${PROJECT_NAME}" ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_sources(${PROJECT_NAME}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/Matrix.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Matrix.inl
    ${CMAKE_CURRENT_LIST_DIR}/Vector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Vector.inl
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Matrix.h
    ${CMAKE_CURRENT_LIST_DIR}/Vector.h
)

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
   "Raptor::Core"
   "Raptor::Debug"
)