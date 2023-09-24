#include "DebugUI.h"
#include <EAStdC/EASprintf.h>

namespace Raptor
{
namespace Debug
{
namespace UI
{

DebugUI::DebugUI(Window& window, Vulkan& vulkan)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(window->window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = YOUR_INSTANCE;
    init_info.PhysicalDevice = YOUR_PHYSICAL_DEVICE;
    init_info.Device = YOUR_DEVICE;
    init_info.QueueFamily = YOUR_QUEUE_FAMILY;
    init_info.Queue = YOUR_QUEUE;
    init_info.PipelineCache = YOUR_PIPELINE_CACHE;
    init_info.DescriptorPool = YOUR_DESCRIPTOR_POOL;
    init_info.Subpass = 0;
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = YOUR_ALLOCATOR;
    init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info, wd->RenderPass);
    // (this gets a bit more complicated, see example app for full reference)
    ImGui_ImplVulkan_CreateFontsTexture(YOUR_COMMAND_BUFFER);
    // (your code submit a queue)
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

DebugUI::~DebugUI()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

DebugUI::Update()
{
    // After glfwPollEvents()
    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow(); // Show demo window! :)
}

DebugUI::Render()
{
    // Rendering
    // (clear framebuffer, render other stuff etc.)
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), YOUR_COMMAND_BUFFER);
    // (call vkCmdEndRenderPass, vkQueueSubmit, vkQueuePresentKHR etc.)
}

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    EA::StdC::Fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

} // namespace UI
} // namespace Debug
} // namespace Raptor