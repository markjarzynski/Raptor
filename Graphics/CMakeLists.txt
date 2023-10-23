project(Graphics)

add_library(${PROJECT_NAME})
add_library("Raptor::${PROJECT_NAME}" ALIAS ${PROJECT_NAME})

message(STATUS "VulkanMemoryAllocator: ${VulkanMemoryAllocator_INCLUDE_DIR}")
#include_directories(${VulkanMemoryAllocator_INCLUDE_DIR})

target_sources(${PROJECT_NAME}
PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}/CommandBuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GPUTimestampManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RenderPass.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ResourcePool.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Vulkan.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Window.cpp
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/CommandBuffer.h
    ${CMAKE_CURRENT_LIST_DIR}/GPUTimestampManager.h
    ${CMAKE_CURRENT_LIST_DIR}/RenderPass.h
    ${CMAKE_CURRENT_LIST_DIR}/ResourcePool.h
    ${CMAKE_CURRENT_LIST_DIR}/Resources.h
    ${CMAKE_CURRENT_LIST_DIR}/Sampler.h
    ${CMAKE_CURRENT_LIST_DIR}/Texture.h
    ${CMAKE_CURRENT_LIST_DIR}/Vulkan.h
    ${CMAKE_CURRENT_LIST_DIR}/Window.h
    #${VulkanMemoryAllocator_INCLUDE_DIR}/vk_mem_alloc.h
)

message(STATUS "VMA: ${VulkanMemoryAllocator_INCLUDE_DIR}")

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${Vulkan_INCLUDE_DIRS}
    ${VulkanMemoryAllocator_INCLUDE_DIR}
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    EASTL
    EAStdC
    "Vulkan::Vulkan"
    #VulkanMemoryAllocator
    glfw
    "imgui"
    "Raptor::Core"
    "Raptor::Debug"
)