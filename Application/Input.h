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

    void NewFrame();
    void Update(float delta);

    bool KeyPress(Key key);
    bool MousePress(MouseButton button);
    vec2d GetCursorPos();

    Window* window;

    vec2d mouse_position;
    vec2d previous_mouse_position;



}; // class Input
} // namespace Application
} // namespace Raptor