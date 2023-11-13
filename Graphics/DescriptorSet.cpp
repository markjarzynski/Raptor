#include "DescriptorSet.h"

namespace Raptor
{
namespace Graphics
{

CreateDescriptorSetParams& CreateDescriptorSetParams::Reset()
{
    num_resources = 0;
    return *this;
}

CreateDescriptorSetParams& CreateDescriptorSetParams::SetLayout(DescriptorSetLayoutHandle layout)
{
    this->layout = layout;
    return *this;
}

CreateDescriptorSetParams& CreateDescriptorSetParams::Texture(TextureHandle texture, uint16 binding)
{
    samplers[num_resources] = InvalidSampler;
    bindings[num_resources] = binding;
    resources[num_resources] = texture;

    num_resources++;

    return *this;
}

CreateDescriptorSetParams& CreateDescriptorSetParams::Buffer(BufferHandle buffer, uint16 binding)
{
    samplers[num_resources] = InvalidSampler;
    bindings[num_resources] = binding;
    resources[num_resources] = buffer;

    num_resources++;

    return *this;
}

CreateDescriptorSetParams& CreateDescriptorSetParams::TextureSampler(TextureHandle texture, SamplerHandle sampler, uint16 binding)
{
    samplers[num_resources] = sampler;
    bindings[num_resources] = binding;
    resources[num_resources] = texture;

    num_resources++;

    return *this;
}

CreateDescriptorSetParams& CreateDescriptorSetParams::SetName(const char* name)
{
    this->name = name;
    return *this;
}

} // namespace Graphics
} // namespace Raptor