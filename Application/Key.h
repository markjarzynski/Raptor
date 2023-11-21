#include <GLFW/glfw3.h>
#include "Types.h"

namespace Raptor
{
namespace Application
{

enum Key : uint32
{
    KEY_A = GLFW_KEY_A,
    KEY_D = GLFW_KEY_D,
    KEY_S = GLFW_KEY_S,
    KEY_W = GLFW_KEY_W,
};

enum MouseButton : uint32
{
    MOUSE_BUTTON_LEFT = GLFW_MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT = GLFW_MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE,
};

} // namespace Application
} // namespace Raptor