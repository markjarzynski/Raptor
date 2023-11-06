#pragma once

#include "Allocator.h"
#include "Types.h"
#include "HashMap.h"
#include "Buffer.h"
#include "Texture.h"
#include "Sampler.h"

namespace Raptor
{
namespace Graphics
{
using Raptor::Core::Allocator;
using Raptor::Core::HashMap;

struct ResourceCache
{
    void init(Allocator& allocator);
    void shutdown(Renderer* renderer);

    HashMap<uint64, BufferResource*> buffers;
    HashMap<uint64, TextureResource*> textures;
    HashMap<uint64, SamplerResource*> samplers;
    //HashMap<uint64, Program*> programs;
    //HashMap<uint64, Material*> materials;

}; // struct ResourceCache

} // namespace Graphics
} // namespace Raptor