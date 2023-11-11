#include "DescriptorSetLayout.h"

namespace Raptor
{
namespace Graphics
{

CreateDescriptorSetLayoutParams& CreateDescriptorSetLayoutParams::Reset()
{
    num_bindings = set_index = 0;
    return *this;
}

CreateDescriptorSetLayoutParams& CreateDescriptorSetLayoutParams::AddBinding(const Binding& binding)
{
    bindings[num_bindings++] = binding;
    return *this;
}

CreateDescriptorSetLayoutParams& CreateDescriptorSetLayoutParams::SetName(const char* name)
{
    this->name = name;
    return *this;
}

CreateDescriptorSetLayoutParams& CreateDescriptorSetLayoutParams::SetIndex(uint32 index)
{
    this->set_index = index;
    return *this;
}
} // namespace Graphics
} // namespace Raptor