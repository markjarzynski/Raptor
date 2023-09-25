#include "Vulkan.h"
#include "Config.h"
#include "Assert.h"
#include "Type.h"
#include <EAStdC/EASprintf.h>

namespace Raptor
{
namespace Graphics
{

Vulkan::Vulkan(Window& window)
    : window(&window)
{
    createInstance();
}

Vulkan::~Vulkan()
{
    destroyInstance();
}

void Vulkan::createInstance()
{
    VkResult result;

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = window->GetName();
    appInfo.applicationVersion = 1; //VK_MAKE_VERSION(RAPTOR_VERSION_MAJOR, RAPTOR_VERSION_MINOR, RAPTOR_VERSION_PATCH);
    appInfo.pEngineName = RAPTOR_PROJECT_NAME;
    appInfo.engineVersion = 1; //VK_MAKE_VERSION(RAPTOR_VERSION_MAJOR, RAPTOR_VERSION_MINOR, RAPTOR_VERSION_PATCH);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    result = vkCreateInstance(&createInfo, nullptr, &instance);
    ASSERT_MESSAGE(result == VK_SUCCESS, "[Vulkan] Error: Failed to create instance. code(%u).", result);

    uint32 numExtensions;

}

void Vulkan::destroyInstance()
{
    vkDestroyInstance(instance, nullptr);
}

} // namespace Graphics
} // namespace Raptor