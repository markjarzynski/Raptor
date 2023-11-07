#pragma once

#include <vulkan/vulkan.h>

#include "GPUDevice.h"

#include "Allocator.h"
#include "Types.h"
#include "ResourceManager.h"
#include "CommandBuffer.h"
#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"
#include "ResourceCache.h"

namespace Raptor
{
namespace Graphics
{
using Raptor::Core::Allocator;
using Raptor::Core::ResourceManager;
using Raptor::Core::ResourceLoader;

class Renderer
{

// TODO

public:

    //Renderer(){}
    Renderer(GPUDevice* gpu_device, ResourceManager* resource_manager, Allocator& allocator);
    ~Renderer();

    void Init();
    void Shutdown();
    Renderer* Renderer::Instance();

    void SetLoaders(ResourceManager* manager);

    void BeginFrame();
    void EndFrame();

    void ResizeSwapchain(uint32 width, uint32 height);
    float AspectRatio() const;

    BufferResource* CreateBuffer(const CreateBufferParams& params);
    BufferResource* CreateBuffer(ResourceUsageType usage, VkBufferUsageFlags flags, uint32 size, void* data, const char* name);

    TextureResource* CreateTexture(const CreateTextureParams& params);
    TextureResource* CreateTexture(const char* name, const char* filename);

    SamplerResource* CreateSampler(const CreateSamplerParams& params);

    void DestroyBuffer(BufferResource* buffer);
    void DestroyTexture(TextureResource* texture);
    void DestroySampler(SamplerResource* sampler);

    void* MapBuffer(BufferResource* buffer, uint32 offset = 0, uint32 size = 0);
    void UnmapBuffer(BufferResource* buffer);

    CommandBuffer* GetCommandBuffer(QueueType type, bool begin) { return gpu_device->GetCommandBuffer(type, begin); }
    void QueueCommandBuffer(CommandBuffer* command) { gpu_device->QueueCommandBuffer(command); }
 
    ResourcePoolTyped<TextureResource> textures;
    ResourcePoolTyped<BufferResource> buffers;
    ResourcePoolTyped<SamplerResource> samplers;

    ResourceCache resource_cache;

    GPUDevice* gpu_device;
    Allocator* allocator;

    uint16 width;
    uint16 height;

}; // class Renderer

// struct CreateRendererParams
// {
//     using Allocator = eastl::allocator;

//     GPUDevice* gpu_device;
//     Allocator* allocator;
// }; // CreateRendererParams

struct BufferLoader : public ResourceLoader
{
    Resource* Get(const char* name) override;
    Resource* Get(uint64 hash) override;
    Resource* Unload(const char* name) override;

    Renderer* renderer;
}; // struct BufferLoader

struct TextureLoader : public ResourceLoader
{
    Resource* Get(const char* name) override;
    Resource* Get(uint64 hash) override;
    Resource* Unload(const char* name) override;
    Resource* CreateFromFile(const char* name, const char* filename, ResourceManager* resource_manger) override;

    Renderer* renderer;
}; // struct TextureLoader

struct SamplerLoader : public ResourceLoader
{
    Resource* Get(const char* name) override;
    Resource* Get(uint64 hash) override;
    Resource* Unload(const char* name) override;

    Renderer* renderer;
}; // struct SamplerLoader


static TextureHandle CreateTextureFromFile(GPUDevice& gpu_device, const char* filename, const char* name);

} // namespace Graphics
} // namespace Raptor