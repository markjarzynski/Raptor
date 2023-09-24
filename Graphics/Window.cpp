#include "Window.h"

namespace Raptor
{
namespace Graphics
{

Window::Window (int width, int height, eastl::string name)
    : width(width), height(height), name(name)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    win->framebufferResized = true;
}

} // namespace Graphics
} // namespace Raptor