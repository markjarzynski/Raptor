#pragma once

#include "Window.h"
#include "Key.h"

namespace Raptor
{
namespace Application
{
class Input
{
public:

    Input(Window& window);
    ~Input();

    Input(const Input &) = delete;
    Input &operator=(const Input &) = delete;

    bool KeyPress(Key key);


    Window* window;

}; // class Input
} // namespace Application
} // namespace Raptor