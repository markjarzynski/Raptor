find_path(VulkanMemoryAllocator_INCLUDE_DIR NAMES vk_mem_alloc.h PATHS "/usr/include/" "/usr/local/include/" "${CMAKE_SOURCE_DIR}/Common/External/VulkanMemoryAllocator/include")

set(VulkanMemoryAllocator_INCLUDE_DIRS ${VulkanMemoryAllocator_INCLUDE_DIR})

find_package_handle_standard_args(VulkanMemoryAllocator REQUIRED_VARS VulkanMemoryAllocator_INCLUDE_DIR VERSION_VAR VulkanMemoryAllocator_VERSION)

message(STATUS "Found VulkanMemoryAllocator: ${VulkanMemoryAllocator_INCLUDE_DIRS}")