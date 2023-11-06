#include "ResourceCache.h"
#include "Renderer.h"

namespace Raptor
{
namespace Graphics
{

/*
ResourceCache::ResourceCache(Allocator& allocator)
{
    buffers.set_allocator(allocator);
    textures.set_allocator(allocator);
    samplers.set_allocator(allocator);
    //programs.set_allocator(allocator);
    //materials.set_allocator(allocator);
}

ResourceCache::~ResourceCache()
{

}
*/

void ResourceCache::init(Allocator& allocator)
{
    buffers.set_allocator(allocator);
    textures.set_allocator(allocator);
    samplers.set_allocator(allocator);
    //programs.set_allocator(allocator);
    //materials.set_allocator(allocator);
}

void ResourceCache::shutdown(Renderer* renderer)
{
    for (auto it = buffers.begin(); it != buffers.end(); it++)
    {
        BufferResource* buffer = it->second;
        renderer->DestroyBuffer(buffer);
    }

    for (auto it = textures.begin(); it != textures.end(); it++)
    {
        TextureResource* texture = it->second;
        renderer->DestroyTexture(texture);
    }

    for (auto it = samplers.begin(); it != samplers.end(); it++)
    {
        SamplerResource* sampler = it->second;
        renderer->DestroySampler(sampler);
    }
}

} // namespace Graphics
} // namespace Raptor