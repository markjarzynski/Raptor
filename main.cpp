#include "Config.h"

#include <EASTL/version.h>
#include <EASTL/allocator.h>
#include <EAStdC/EASprintf.h>

#include "Window.h"
#include "Vulkan.h"
#include "DebugUI.h"

// These new operators are required by EASTL
void* __cdecl operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

void* __cdecl operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

void debug_print_versions()
{
    EA::StdC::Printf("Raptor version: %s\n", RAPTOR_VERSION);
    EA::StdC::Printf("EASTL version: %s\n", EASTL_VERSION);
    EA::StdC::Printf("GLFW version: %s\n", glfwGetVersionString());
    EA::StdC::Printf("Dear ImGui version: %s\n", ImGui::GetVersion());

    uint32_t instanceVersion = VK_API_VERSION_1_0;
    vkEnumerateInstanceVersion(&instanceVersion);
    EA::StdC::Printf("Vulkan version: %d.%d.%d\n", VK_VERSION_MAJOR(instanceVersion), VK_VERSION_MINOR(instanceVersion), VK_VERSION_PATCH(instanceVersion));
}

int main( int argc, char** argv)
{
    debug_print_versions();

    using Allocator = eastl::allocator;
    Allocator allocator {};

    Raptor::Graphics::Window window {1920, 1080, "Raptor"};
    Raptor::Graphics::Vulkan vulkan {window, allocator};
    //Raptor::Debug::UI::DebugUI debugUI {window, vulkan};

    while (!window.ShouldClose())
    {
        window.PollEvents();
        //debugUI.Update();
        //debugUI.Render();
    }

    return 0;
}

