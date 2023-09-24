#include "Vulkan.h"
#include "Config.h"
#include <EAStdC/EASprintf.h>

namespace Raptor
{
namespace Graphics
{

Vulkan::Vulkan(Window& window)
{
    createInstance();
}

void Vulkan::createInstance()
{
    VkResult result;

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = window->GetName();
    appInfo.applicationVersion = VK_MAKE_VERSION(RAPTOR_VERSION_MAJOR, RAPTOR_VERSION_MINOR, RAPTOR_VERSION_PATCH);
    appInfo.pEngineName = RAPTOR_PROJECT_NAME;
    appInfo.engineVersion = VK_MAKE_VERSION(RAPTOR_VERSION_MAJOR, RAPTOR_VERSION_MINOR, RAPTOR_VERSION_PATCH);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    result = vkCreateInstance(&createInfo, nullptr, &instance);
    check_result(result);

    
}

void Vulkan::destroyInstance()
{
    vkDestroyInstance(instance, nullptr);
}

static bool check_result(VkResult result)
{
    if (result == VK_SUCCESS)
        return true;
    
    EA::StdC::Fprintf(stderr, "Vulkan error: code(%u)\n", result);
}

} // namespace Graphics
} // namespace Raptor