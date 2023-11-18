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

} // namespace Application
} // namespace Raptor