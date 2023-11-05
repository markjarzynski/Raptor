#pragma once

#include <EASTL/allocator.h>
#include "GPUDevice.h"
#include "ResourceManager.h"

namespace Raptor
{
namespace Graphics
{

struct BufferResource
{

}; // struct BufferResource

class Renderer
{

    using Allocator = eastl::allocator;
    using ResourceManager = Raptor::Core::ResourceManager;

    // TODO

public:

    Renderer(GPUDevice* gpu_device, Allocator& allocator);
    ~Renderer();

    void Init();
    void Shutdown();

    void SetLoaders(ResourceManager* manager);

    void BeginFrame();
    void EndFrame();

    void ResizeSwapchain(uint32 width, uint32 height);
    float AspectRatio() const;

    BufferResource* CreateBuffer(const CreateBufferParams params);
    //BufferResource* 




    GPUDevice* gpu_device;
    Allocator* allocator;

}; // class Renderer
} // namespace Graphics
} // namespace Raptor