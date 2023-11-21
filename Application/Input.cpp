#include "Input.h"

namespace Raptor
{
namespace Application
{

Input::Input(Window& window) : window(&window)
{

}

Input::~Input()
{

}

bool Input::KeyPress(Key key)
{
    int state = glfwGetKey(window->GetGLFWwindow(), key);
    return state == GLFW_PRESS;
}

bool Input::MousePress(MouseButton button)
{
    int state = glfwGetMouseButton(window->GetGLFWwindow(), button);
    return state == GLFW_PRESS;
}

vec2d Input::GetCursorPos()
{
    double x, y;
    glfwGetCursorPos(window->GetGLFWwindow(), &x, &y);
    return vec2d(x, y);
}

} // namespace Application
} // namespace Raptor