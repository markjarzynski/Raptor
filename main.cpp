#include <EASTL/version.h>
#include <EASTL/allocator.h>
#include <EAStdC/EASprintf.h>

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>

#include "Raptor.h"
#include "Window.h"
#include "GPUDevice.h"
#include "ResourceManager.h"
#include "GPUProfiler.h"
#include "Renderer.h"
#include "Texture.h"
#include "Sampler.h"
#include "DebugUI.h"
#include "File.h"

// These new operators are required by EASTL
void* __cdecl operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

void* __cdecl operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line)
{
    return new uint8_t[size];
}

void debug_print_versions()
{
    EA::StdC::Printf("%s version: %s\n", RAPTOR_PROJECT_NAME, RAPTOR_VERSION);
    EA::StdC::Printf("EASTL version: %s\n", EASTL_VERSION);
    EA::StdC::Printf("GLFW version: %s\n", glfwGetVersionString());
    EA::StdC::Printf("Dear ImGui version: %s\n", ImGui::GetVersion());

    uint32_t instanceVersion = VK_API_VERSION_1_0;
    vkEnumerateInstanceVersion(&instanceVersion);
    EA::StdC::Printf("Vulkan version: %d.%d.%d\n", VK_VERSION_MAJOR(instanceVersion), VK_VERSION_MINOR(instanceVersion), VK_VERSION_PATCH(instanceVersion));
}

int main( int argc, char** argv)
{
    debug_print_versions();

    if (argc < 2)
    {
        printf("Usage: %s [path to glTF model]\n", argv[0]);
        return 0;
    }
    
    Raptor::Debug::Log("%s\n", argv[1]);

    using Allocator = eastl::allocator;
    Allocator allocator {};

    Raptor::Graphics::Window window {1920, 1080, "Raptor"};
    Raptor::Graphics::GPUDevice gpu_device {window, allocator};
    Raptor::Core::ResourceManager resource_manager {allocator, nullptr};
    Raptor::Graphics::GPUProfiler gpu_profiler {allocator, 100};
    Raptor::Graphics::Renderer renderer {&gpu_device, &resource_manager, allocator};
    //Raptor::Debug::UI::DebugUI debugUI {window, gpu_device};

    //Directory cwd{};

    char base_path[512]{};
    memcpy(base_path, argv[1], strlen(argv[1]));
    Raptor::Core::DirectoryFromPath(base_path);

    Raptor::Core::ChangeDirectory(base_path);

    char gltf_file[512]{};
    memcpy(gltf_file, argv[1], strlen(argv[1]));
    Raptor::Core::FilenameFromPath(gltf_file);

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    
    loader.LoadASCIIFromFile(&model, &err, &warn, gltf_file);

    eastl::vector<Raptor::Graphics::TextureResource> images(model.images.size(), allocator);
    for (uint32 i = 0; i < model.images.size(); i++)
    {
        tinygltf::Image& image = model.images[i];
        Raptor::Graphics::TextureResource* tr = renderer.CreateTexture(image.uri.data(), image.uri.data());
        ASSERT(tr != nullptr);

        images[i] = *tr;
    }

    eastl::vector<Raptor::Graphics::SamplerResource> samplers(model.samplers.size(), allocator);
    for (uint32 i = 0; i < model.samplers.size(); i++)
    {
        tinygltf::Sampler& sampler = model.samplers[i];

        char name[64];
        snprintf(name, 64, "Sampler_%u", i);

        Raptor::Graphics::CreateSamplerParams params {};
        params.min_filter = (sampler.minFilter == TINYGLTF_TEXTURE_FILTER_LINEAR || sampler.minFilter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        params.mag_filter = (sampler.magFilter == TINYGLTF_TEXTURE_FILTER_LINEAR || sampler.magFilter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR) ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
        params.name = name;

        Raptor::Graphics::SamplerResource* sr = renderer.CreateSampler(params);
        ASSERT(sr != nullptr);

        samplers[i] = *sr;
    }

    while (!window.ShouldClose())
    {
        window.PollEvents();
        //debugUI.Update();
        //debugUI.Render();
    }

    return 0;
}

