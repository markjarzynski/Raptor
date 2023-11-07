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

static uint8* GetBufferData(std::vector<tinygltf::BufferView> buffer_views, uint32 buffer_index, eastl::vector<void*> buffers_data, uint32* buffer_size = nullptr, char** buffer_name = nullptr)
{
    tinygltf::BufferView& buffer = buffer_views[buffer_index];

    uint32 offset = buffer.byteOffset;

    if (buffer_name != nullptr)
    {
        *buffer_name = buffer.name.data();
    }

    if (buffer_size != nullptr)
    {
        *buffer_size = buffer.byteLength;
    }

    uint8* data = (uint8*)buffers_data[buffer.buffer] + offset;

    return data;
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

    char cwd[Raptor::Core::MAX_FILENAME_LENGTH];
    Raptor::Core::CurrentDirectory(cwd);

    char base_path[Raptor::Core::MAX_FILENAME_LENGTH];
    memcpy(base_path, argv[1], strlen(argv[1]));
    Raptor::Core::DirectoryFromPath(base_path);

    Raptor::Core::ChangeDirectory(base_path);

    char gltf_file[Raptor::Core::MAX_FILENAME_LENGTH];
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

    eastl::vector<void*> buffers_data(model.buffers.size(), allocator);
    for (uint32 i = 0; i < model.buffers.size(); i++)
    {
        tinygltf::Buffer& buffer = model.buffers[i];

        Raptor::Core::FileReadResult buffer_data = Raptor::Core::FileReadBinary(buffer.uri.data(), &allocator);
        buffers_data[i] = buffer_data.data;
    }

    eastl::vector<Raptor::Graphics::BufferResource> buffers(model.bufferViews.size(), allocator);
    for (uint32 i = 0; i < model.bufferViews.size(); i++)
    {
        char* buffer_name = nullptr;
        uint32 buffer_size = 0;
        uint8* data = GetBufferData(model.bufferViews, i, buffers_data, &buffer_size, &buffer_name);

        VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        if (buffer_name == nullptr)
        {
            snprintf(buffer_name, 64, "Buffer_%u", i);
        }
        else
        {
            snprintf(buffer_name, 64, "%s_%u", buffer_name, i);
        }

        Raptor::Graphics::BufferResource* br = renderer.CreateBuffer(Raptor::Graphics::ResourceUsageType::Immutable, flags, buffer_size, data, buffer_name);
        ASSERT(br != nullptr);

        buffers[i] = *br;
    }

    Raptor::Core::ChangeDirectory(cwd);


    while (!window.ShouldClose())
    {
        window.PollEvents();
        //debugUI.Update();
        //debugUI.Render();
    }

    return 0;
}

