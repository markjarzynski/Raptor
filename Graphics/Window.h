#pragma once

#include <EASTL/string.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Raptor
{
namespace Graphics
{
class Window
{
public:

    Window(int width, int height, eastl::string name);
    ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    bool ShouldClose() { return glfwWindowShouldClose(window); }
    void PollEvents() { glfwPollEvents(); }
    void SwapBuffers() { glfwSwapBuffers(window); }

    bool framebufferResized = false;

private:

    int width;
    int height;

    eastl::string name;
    GLFWwindow* window;

}; // class Window

static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

} // namespace Graphics
} // namespace Raptor