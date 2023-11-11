#include "Pipeline.h"
namespace Raptor
{
namespace Graphics
{

CreateDepthStencilParams& CreateDepthStencilParams::SetDepth(bool write, VkCompareOp comparison_test)
{
    depth_write_enable = write;
    depth_comparison = comparison_test;
    depth_enable = 1;
    return *this;
}

CreateVertexInputParams& CreateVertexInputParams::Reset()
{
    num_vertex_streams = num_vertex_attributes = 0;
    return *this;
}

CreateVertexInputParams& CreateVertexInputParams::AddVertexAttribute( const VertexAttribute& attribute)
{
    vertex_attributes[num_vertex_attributes++] = attribute;
    return *this;
}

CreateVertexInputParams& CreateVertexInputParams::AddVertexStream(const VertexStream& stream)
{
    vertex_streams[num_vertex_streams++] = stream;
    return *this;
}

CreateShaderStateParams& CreateShaderStateParams::Reset()
{
    stages_count = 0;
    return *this;
}

CreateShaderStateParams& CreateShaderStateParams::SetName(const char* name)
{
    this->name = name;
    return *this;
}

CreateShaderStateParams& CreateShaderStateParams::AddStage(const char* code, uint32 code_size, VkShaderStageFlagBits type)
{
    stages[stages_count].code = code;
    stages[stages_count].code_size = code_size;
    stages[stages_count].type = type;
    stages_count++;
    return *this;
}

CreateShaderStateParams& CreateShaderStateParams::SetSpvInput(bool value)
{
    spv_input = value;
    return *this;
}

CreatePipelineParams& CreatePipelineParams::AddDescriptorSetLayout(DescriptorSetLayoutHandle handle)
{
    descriptor_set_layout[num_active_layouts++] = handle;
    return *this;
}

RenderPassOutput& CreatePipelineParams::RenderPassOutput()
{
    return render_pass;
}


} // namespace Graphics
} // namespace Raptor