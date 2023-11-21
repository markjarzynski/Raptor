#pragma once

#include "Window.h"
#include "Key.h"
#include "Vector.h"

namespace Raptor
{
namespace Application
{

using Raptor::Math::vec2d;

class Input
{
public:

    Input(Window& window);
    ~Input();

    Input(const Input &) = delete;
    Input &operator=(const Input &) = delete;

    bool KeyPress(Key key);
    bool MousePress(MouseButton button);
    vec2d GetCursorPos();

    Window* window;

}; // class Input
} // namespace Application
} // namespace Raptor