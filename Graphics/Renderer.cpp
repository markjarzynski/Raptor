#include "Renderer.h"
#include "Hash.h"

namespace Raptor
{
namespace Graphics
{

using Raptor::Core::Allocator;
using Raptor::Core::Resource;
using Raptor::Core::HashString;

uint64 TextureResource::type_hash = 0;
uint64 BufferResource::type_hash = 0;
uint64 SamplerResource::type_hash = 0;

static BufferLoader s_buffer_loader;
static SamplerLoader s_sampler_loader;
static TextureLoader s_texture_loader;

Renderer::Renderer(GPUDevice* gpu_device, ResourceManager* resource_manager, Allocator& allocator)
    : gpu_device(gpu_device), allocator(&allocator)
{
    width = gpu_device->swapchain_width;
    height = gpu_device->swapchain_height;

    textures.init(&allocator, 512);
    buffers.init(&allocator, 4096);
    samplers.init(&allocator, 128);

    resource_cache.init(allocator);

    TextureResource::type_hash = Raptor::Core::HashString(TextureResource::type);
    BufferResource::type_hash = Raptor::Core::HashString(BufferResource::type);
    SamplerResource::type_hash = Raptor::Core::HashString(SamplerResource::type);

    s_texture_loader.renderer = this;
    s_buffer_loader.renderer = this;
    s_sampler_loader.renderer = this;

    SetLoaders(resource_manager);
}

Renderer::~Renderer()
{

}

void Renderer::Init()
{

}

void Renderer::Shutdown()
{
    resource_cache.shutdown(this);

    textures.shutdown();
    buffers.shutdown();
    samplers.shutdown();

    gpu_device->Shutdown();
}

void Renderer::SetLoaders(ResourceManager* manager)
{
    manager->SetLoader(TextureResource::type, &s_texture_loader);
    manager->SetLoader(BufferResource::type, &s_buffer_loader);
    manager->SetLoader(SamplerResource::type, &s_sampler_loader);
}

void Renderer::BeginFrame()
{
    gpu_device->NewFrame();
}

void Renderer::EndFrame() {}

void Renderer::ResizeSwapchain(uint32 width, uint32 height) {}

float Renderer::AspectRatio() const
{

    return 1.f;
}

BufferResource* Renderer::CreateBuffer(const CreateBufferParams& params)
{
    return nullptr;
}

BufferResource* Renderer::CreateBuffer(VkBufferUsageFlags type, ResourceUsageType usage, uint32 size, void* data, const char* name)
{
    return nullptr;
}

TextureResource* Renderer::CreateTexture(const CreateTextureParams& params)
{
    return nullptr;
}

TextureResource* Renderer::CreateTexture(const char* name, const char* filename)
{
    return nullptr;
}

SamplerResource* Renderer::CreateSampler(const CreateSamplerParams& params)
{
    return nullptr;
}

void Renderer::DestroyBuffer(BufferResource* buffer) {}
void Renderer::DestroyTexture(TextureResource* texture) {}
void Renderer::DestroySampler(SamplerResource* sampler) {}

void* Renderer::MapBuffer(BufferResource* buffer, uint32 offset, uint32 size)
{
    return nullptr;
}

void Renderer::UnmapBuffer(BufferResource* buffer) {}


// BufferLoader  ----------------------------
Resource* BufferLoader::Get(const char* name)
{
    const uint64 hash = HashString(name);
    return renderer->resource_cache.buffers.at(hash);
}

Resource* BufferLoader::Get(uint64 hash)
{
    return renderer->resource_cache.buffers.at(hash);
}

Resource* BufferLoader::Unload(const char* name)
{
    const uint64 hash = HashString(name);
    BufferResource* buffer = renderer->resource_cache.buffers.at(hash);
    if (buffer)
        renderer->DestroyBuffer(buffer);
    
    return nullptr;
}

// TextureLoader ----------------------------
Resource* TextureLoader::Get(const char* name)
{
    const uint64 hash = HashString(name);
    return renderer->resource_cache.textures.at(hash);
}

Resource* TextureLoader::Get(uint64 hash)
{
    return renderer->resource_cache.textures.at(hash);
}

Resource* TextureLoader::Unload(const char* name)
{
    const uint64 hash = HashString(name);
    TextureResource* texture = renderer->resource_cache.textures.at(hash);
    if (texture)
        renderer->DestroyTexture(texture);
    
    return nullptr;
}

Resource* TextureLoader::CreateFromFile(const char* name, const char* filename, ResourceManager* resource_manager)
{
    return renderer->CreateTexture(name, filename);
}

// SamplerLoader ----------------------------
Resource* SamplerLoader::Get(const char* name)
{
    const uint64 hash = HashString(name);
    return renderer->resource_cache.samplers.at(hash);
}

Resource* SamplerLoader::Get(uint64 hash)
{
    return renderer->resource_cache.samplers.at(hash);
}

Resource* SamplerLoader::Unload(const char* name)
{
    const uint64 hash = HashString(name);
    SamplerResource* sampler = renderer->resource_cache.samplers.at(hash);
    if (sampler)
        renderer->DestroySampler(sampler);
    
    return nullptr;
}

/*
static Renderer s_renderer;

Renderer* Renderer::Instance()
{
    return &s_renderer;
}
*/

} // namespace Graphics
} // namespace Raptor