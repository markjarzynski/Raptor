#include <stb_image.h>

#include "Renderer.h"
#include "Hash.h"
#include "Log.h"

namespace Raptor
{
namespace Graphics
{

using Raptor::Core::Allocator;
using Raptor::Core::Resource;
using Raptor::Core::HashString;
using Raptor::Core::Pair;

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
    return gpu_device->swapchain_width * 1.f / gpu_device->swapchain_height;
}

BufferResource* Renderer::CreateBuffer(const CreateBufferParams& params)
{
    BufferResource* buffer = buffers.obtain();

    if (buffer)
    {
        BufferHandle handle = gpu_device->CreateBuffer(params);
        buffer->handle = handle;
        buffer->name = params.name;
        gpu_device->QueryBuffer(handle, buffer->desc);

        if (params.name != nullptr)
        {
            uint64 hash = HashString(params.name);
            Pair<uint64, BufferResource*> pair {hash, buffer};
            resource_cache.buffers.insert(pair);
        }
        
        buffer->references = 1;

        return buffer;
    }

    return nullptr;
}

BufferResource* Renderer::CreateBuffer(ResourceUsageType usage, VkBufferUsageFlags flags, uint32 size, void* data, const char* name)
{
    CreateBufferParams params {usage, flags, size, data, name};
    return CreateBuffer(params);
}

TextureResource* Renderer::CreateTexture(const CreateTextureParams& params)
{
    TextureResource* texture = textures.obtain();

    if (texture)
    {
        TextureHandle handle = gpu_device->CreateTexture(params);
        texture->handle = handle;
        texture->name = params.name;
        gpu_device->QueryTexture(handle, texture->desc);

        if (params.name != nullptr)
        {
            uint64 hash = HashString(params.name);
            Pair<uint64, TextureResource*> pair = {hash, texture};
            resource_cache.textures.insert(pair);
        }

        texture->references = 1;

        return texture;
    }

    return nullptr;
}

TextureResource* Renderer::CreateTexture(const char* name, const char* filename)
{
    TextureResource* texture = textures.obtain();

    if (texture)
    {
        TextureHandle handle = CreateTextureFromFile(*gpu_device, filename, name);
        texture->handle = handle; 
        gpu_device->QueryTexture(handle, texture->desc);
        texture->references = 1;
        texture->name = name;

        uint64 hash = HashString(name);
        Pair<uint64, TextureResource*> pair = {hash, texture};
        resource_cache.textures.insert(pair);

        return texture;
    }

    return nullptr;
}

SamplerResource* Renderer::CreateSampler(const CreateSamplerParams& params)
{
    SamplerResource* sampler = samplers.obtain();
    
    if (sampler)
    {
        SamplerHandle handle = gpu_device->CreateSampler(params);
        sampler->handle = handle;
        sampler->name = params.name;
        gpu_device->QuerySampler(handle, sampler->desc);

        if (params.name != nullptr)
        {
            uint64 hash = HashString(params.name);
            Pair<uint64, SamplerResource*> pair = {hash, sampler};
            resource_cache.samplers.insert(pair);
        }

        sampler->references = 1;

        return sampler;
    }

    return nullptr;
}

void Renderer::DestroyBuffer(BufferResource* buffer)
{
    if (!buffer)
        return;
    
    buffer->RemoveReference();
    if (buffer->references)
        return;
    
    uint64 hash = HashString(buffer->name);
    auto it = resource_cache.buffers.find(hash);
    resource_cache.buffers.erase(it);

    gpu_device->DestroyBuffer(buffer->handle);
    buffers.release(buffer);
}

void Renderer::DestroyTexture(TextureResource* texture)
{
    if (!texture)
        return;
    
    texture->RemoveReference();
    if (texture->references)
        return;
    
    uint64 hash = HashString(texture->name);
    auto it = resource_cache.textures.find(hash);
    resource_cache.textures.erase(it);

    gpu_device->DestroyTexture(texture->handle);
    textures.release(texture);
}

void Renderer::DestroySampler(SamplerResource* sampler)
{
    if (!sampler)
        return;
    
    sampler->RemoveReference();
    if (sampler->references)
        return;
    
    uint64 hash = HashString(sampler->name);
    auto it = resource_cache.samplers.find(hash);
    resource_cache.samplers.erase(it);

    gpu_device->DestroyTexture(sampler->handle);
    samplers.release(sampler);
}

void* Renderer::MapBuffer(BufferResource* buffer, uint32 offset, uint32 size)
{
    MapBufferParams cb_map = {buffer->handle, offset, size};
    return gpu_device->MapBuffer(cb_map);
}

void Renderer::UnmapBuffer(BufferResource* buffer)
{
    if (buffer->desc.parent_handle == InvalidBuffer)
    {
        MapBufferParams cb_map = {buffer->handle, 0, 0};
        gpu_device->UnmapBuffer(cb_map);
    }
}

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

static TextureHandle CreateTextureFromFile(GPUDevice& gpu_device, const char* filename, const char* name)
{
    if (filename)
    {
        int comp, width, height;
        uint8* image_data = stbi_load(filename, &width, &height, &comp, 4);
        if (!image_data)
        {
            Raptor::Debug::Log("[Vulkan] Error: Could not load texture %s\n", filename);
            return InvalidTexture;
        }

        CreateTextureParams params;
        params.SetData(image_data).SetFormatType(VK_FORMAT_R8G8B8A8_UNORM, TextureType::Enum::Texture2D).SetFlags(1, 0).SetSize((uint16)width, (uint16)height, 1).SetName(name);

        TextureHandle new_texture = gpu_device.CreateTexture(params);

        free(image_data);

        return new_texture;

    }

    return InvalidTexture;
}

} // namespace Graphics
} // namespace Raptor