project(Graphics)

add_library(${PROJECT_NAME})
add_library("Raptor::${PROJECT_NAME}" ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/CommandBuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CommandBufferRing.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GPUDevice.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GPUProfiler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GPUTimestampManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Renderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RenderPass.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ResourceCache.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ResourcePool.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Window.cpp
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Buffer.h
    ${CMAKE_CURRENT_LIST_DIR}/CommandBuffer.h
    ${CMAKE_CURRENT_LIST_DIR}/CommandBufferRing.h
    ${CMAKE_CURRENT_LIST_DIR}/DescriptorBinding.h
    ${CMAKE_CURRENT_LIST_DIR}/DescriptorSet.h
    ${CMAKE_CURRENT_LIST_DIR}/DescriptorSetLayout.h
    ${CMAKE_CURRENT_LIST_DIR}/GPUDevice.h
    ${CMAKE_CURRENT_LIST_DIR}/GPUProfiler.h
    ${CMAKE_CURRENT_LIST_DIR}/GPUTimestampManager.h
    ${CMAKE_CURRENT_LIST_DIR}/Renderer.h
    ${CMAKE_CURRENT_LIST_DIR}/RenderPass.h
    ${CMAKE_CURRENT_LIST_DIR}/ResourceCache.h
    ${CMAKE_CURRENT_LIST_DIR}/ResourcePool.h
    ${CMAKE_CURRENT_LIST_DIR}/Resources.h
    ${CMAKE_CURRENT_LIST_DIR}/ShaderState.h
    ${CMAKE_CURRENT_LIST_DIR}/Pipeline.h
    ${CMAKE_CURRENT_LIST_DIR}/Sampler.h
    ${CMAKE_CURRENT_LIST_DIR}/Texture.h
    ${CMAKE_CURRENT_LIST_DIR}/Window.h
)

target_include_directories(${PROJECT_NAME}
PUBLIC
    ${Vulkan_INCLUDE_DIRS}
    ${VulkanMemoryAllocator_INCLUDE_DIR}
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_BINARY_DIR} # Required to find Raptor.h
)

target_link_libraries(${PROJECT_NAME}
PRIVATE
    EASTL
    EAStdC
    "Vulkan::Vulkan"
    glfw
    tinygltf
    "imgui"
    "Raptor::Core"
    "Raptor::Debug"
)