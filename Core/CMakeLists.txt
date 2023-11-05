project(Core)

add_library(${PROJECT_NAME})
add_library("Raptor::${PROJECT_NAME}" ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

target_sources(${PROJECT_NAME}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/ResourceManager.cpp
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Constants.h
    ${CMAKE_CURRENT_LIST_DIR}/Defines.h
    ${CMAKE_CURRENT_LIST_DIR}/Hash.h
    ${CMAKE_CURRENT_LIST_DIR}/ResourceManager.h
    ${CMAKE_CURRENT_LIST_DIR}/Service.h
    ${CMAKE_CURRENT_LIST_DIR}/Types.h
)

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME}
PUBLIC
    EASTL
    wyhash
    "Raptor::Debug"
)