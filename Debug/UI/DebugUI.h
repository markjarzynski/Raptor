#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include "../../Graphics/Window.h"
#include "../../Graphics/Vulkan.h"

namespace Raptor
{
namespace Debug
{
namespace UI
{
class DebugUI
{
public:

    DebugUI(Raptor::Graphics::Window& window, Raptor::Graphics::Vulkan& vulkan);
    ~DebugUI();

    DebugUI(const DebugUI &) = delete;
    DebugUI &operator=(const DebugUI &) = delete;

    Raptor::Graphics::Window* window;
    Raptor::Graphics::Vulkan* vulkan;

    void Update();
    void Render();

private:



}; // class DebugUI

static void check_vk_result(VkResult err);

} // namespace UI
} // namespace Debug
} // namespace Raptor