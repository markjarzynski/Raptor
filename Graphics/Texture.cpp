#include "Texture.h"

namespace Raptor
{
namespace Graphics
{

CreateTextureParams& CreateTextureParams::SetSize(uint16 width, uint16 height, uint16 depth)
{
    this->width = width;
    this->height = height;
    this->depth = depth;

    return *this;
}

CreateTextureParams& CreateTextureParams::SetFlags(uint8 mipmaps, uint8 flags)
{
    this->mipmaps = mipmaps;
    this->flags = flags;
    return *this;
}

CreateTextureParams& CreateTextureParams::SetFormatType(VkFormat format, TextureType::Enum type)
{
    this->vk_format = format;
    this->type = type;
    return *this;
}

CreateTextureParams& CreateTextureParams::SetName(const char* name)
{
    this->name = name;
    return *this;
}

CreateTextureParams& CreateTextureParams::SetData(void* data)
{
    this->data = data;
    return *this;
}


} // namespace Graphics
} // namespace Raptor