#pragma once

#include <vulkan/vulkan.h>
#include "Window.h"

namespace Raptor
{
namespace Graphics
{
class Vulkan
{
public:

    Vulkan(Window& window);
    ~Vulkan();

    Vulkan(const Vulkan &) = delete;
    Vulkan &operator=(const Vulkan &) = delete;

    Window* window;

    const char* version();

private:

    // Vulkan Instance
    void createInstance();
    void destroyInstance();
    
    VkInstance instance;

}; // class Vulkan

static bool check_result(VkResult result);

} // namespace Graphics
} // namespace Raptor