project(Core)

add_library(${PROJECT_NAME})
add_library("Raptor::${PROJECT_NAME}" ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_sources(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Constants.h
    ${CMAKE_CURRENT_LIST_DIR}/Defines.h
    ${CMAKE_CURRENT_LIST_DIR}/Service.h
    ${CMAKE_CURRENT_LIST_DIR}/Types.h
)

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)