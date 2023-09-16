#include <EASTL/vector.h>
#include <EASTL/version.h>
#include <stdio.h>

#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <imgui.h>

// These new operators are required by EASTL
void* __cdecl operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

void* __cdecl operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}


int main( int argc, char** argv)
{
    eastl::vector<uint32_t> v = {0, 1, 2};

    for (uint32_t i : v)
    {
        printf("%d\n", i);
    }

    glfwInit();

    printf("EASTL version: %s\n", EASTL_VERSION);
    printf("GLFW version: %s\n", glfwGetVersionString());
    printf("Dear ImGui version: %s\n", ImGui::GetVersion());

    uint32_t instanceVersion = VK_API_VERSION_1_0;
    vkEnumerateInstanceVersion(&instanceVersion);
    printf("Vulkan version: %d.%d.%d\n", VK_VERSION_MAJOR(instanceVersion), VK_VERSION_MINOR(instanceVersion), VK_VERSION_PATCH(instanceVersion));

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello World!", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    if (glfwVulkanSupported())
    {
        printf("GLFW: Vulkan supported\n");
    }

    glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    while (!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();

        ImGui::ShowDemoWindow();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
