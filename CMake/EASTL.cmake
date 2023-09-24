# EASTL
#set(EASTL_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/Common/External/EASTL")
#include_directories (${EASTL_ROOT_DIR}/include)
#include_directories (${EASTL_ROOT_DIR}/test/packages/EAAssert/include)
#include_directories (${EASTL_ROOT_DIR}/test/packages/EABase/include/Common)
#include_directories (${EASTL_ROOT_DIR}/test/packages/EAMain/include)
#include_directories (${EASTL_ROOT_DIR}/test/packages/EAStdC/include)
#include_directories (${EASTL_ROOT_DIR}/test/packages/EATest/include)
#include_directories (${EASTL_ROOT_DIR}/test/packages/EAThread/include)

#set(EASTL_LIBRARY debug ${EASTL_ROOT_DIR}/build/Debug/EASTL.lib optimized ${EASTL_ROOT_DIR}/build/Release/EASTL.lib)
#add_custom_target(NatVis SOURCES ${EASTL_ROOT_DIR}/doc/EASTL.natvis)

# EASTL
include(ExternalProject)

ExternalProject_Add(
    EABASE
    PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/Common/External/EABase"
    GIT_REPOSITORY git@github.com:electronicarts/EABase.git
    GIT_TAG 521cb05
    GIT_CONFIG advice.detachedHead=false
    GIT_SHALLOW 1
    GIT_SUBMODULES_RECURSE 0
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/Common/External/EABase/install
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Common/External/EABase/install/include)

ExternalProject_Add(
    EASTL
    PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/Common/External/EASTL"
    GIT_REPOSITORY git@github.com:electronicarts/EASTL.git
    GIT_TAG 3.21.12
    GIT_CONFIG advice.detachedHead=false
    GIT_SHALLOW 1
    GIT_SUBMODULES_RECURSE 0
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_SOURCE_DIR}/Common/External/EASTL/install
        -DEASTL_BUILD_BENCHMARK:BOOL=OFF
        -DEASTL_BUILD_TESTS:BOOL=OFF
    DEPENDS
        EABASE
)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Common/External/EASTL/install/include)
set(EASTL_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/Common/External/EASTL/install/lib/EASTL.lib)