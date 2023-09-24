#include "Config.h"

#include <EASTL/vector.h>
#include <EASTL/version.h>
//#include <EAStdC/EASprintf.h>
#include <stdio.h>

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <imgui.h>

#include "Graphics/Window.h"

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
    printf("Raptor version: %s\n", RAPTOR_VERSION);
    printf("EASTL version: %s\n", EASTL_VERSION);
    printf("GLFW version: %s\n", glfwGetVersionString());
    printf("Dear ImGui version: %s\n", ImGui::GetVersion());

    uint32_t instanceVersion = VK_API_VERSION_1_0;
    vkEnumerateInstanceVersion(&instanceVersion);
    printf("Vulkan version: %d.%d.%d\n", VK_VERSION_MAJOR(instanceVersion), VK_VERSION_MINOR(instanceVersion), VK_VERSION_PATCH(instanceVersion));
}

int main( int argc, char** argv)
{
    debug_print_versions();

    Raptor::Graphics::Window window {640, 480, "Raptor"};

    //glfwMakeContextCurrent(window);

    //IMGUI_CHECKVERSION();
    //ImGui::CreateContext();

    while (!window.ShouldClose())
    {
        window.SwapBuffers();
        window.PollEvents();

        //ImGui::ShowDemoWindow();
    }

    return 0;
}

