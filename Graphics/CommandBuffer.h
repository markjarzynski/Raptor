#pragma once

#include <vulkan/vulkan.h>
#include "Types.h"
#include "Resources.h"
#include "GPUDevice.h"
#include "Pipeline.h"

namespace Raptor
{
namespace Graphics
{

enum CommandBufferFlags : uint32
{
    None        = 0x0,
    isRecording = 0x1,
    isBaked     = 0x1 << 1,
};

class CommandBuffer
{
public:
    
    CommandBuffer(){}
    CommandBuffer(QueueType type, uint32 bufferSize, uint32 submitSize, uint32 m_uFlags = CommandBufferFlags::None);
    ~CommandBuffer();

    void Init(QueueType queue_type, uint32 bufferSize, uint32 submitSize, uint32 m_uFlags = CommandBufferFlags::None);
    void Terminate();

    void BindPass(RenderPassHandle handle);
    void BindPipeline(PipelineHandle handle);
    void BindVertexBuffer(BufferHandle handle, uint32 binding, uint32 offset);
    void BindIndexBuffer(BufferHandle handle, uint32 offset, VkIndexType index_type);
    void BindDescriptorSet(DescriptorSetHandle* handles, uint32 num_lists, uint32* offsets, uint32 num_offsets);

    void SetViewport(const Viewport* viewport);
    void SetScissor(const Rect2DInt* rect);

    void Clear(float red, float green, float blue, float alpha);
    void ClearDepthStencil(float depth, uint8 stencil);

    void Draw(TopologyType topology, uint32 first_vertex, uint32 vertex_count, uint32 first_instance, uint32 instance_count);
    void DrawIndexed(TopologyType topology, uint32 index_count, uint32 instance_count, uint32 first_index, uint32 vertex_offset, uint32 first_instance);
    void DrawIndirect(BufferHandle handle, uint32 offset, uint32 stride);
    void DrawIndexedIndirect(BufferHandle handle, uint32 offset, uint32 stride);

    void Dispatch(uint32 group_x, uint32 group_y, uint32 group_z);
    void DispatchIndirect(BufferHandle handle, uint32 offset);

    void Barrier(const ExecutionBarrier& barrier);

    void FillBuffer(BufferHandle buffer, uint32 offset, uint32 size, uint32 data);

    void PushMarker(const char* name);
    void PopMarker();

    void Reset();

    VkCommandBuffer vk_command_buffer;

    GPUDevice* gpu_device;

    VkDescriptorSet vk_descriptor_sets[16];

    RenderPass* current_render_pass;
    Pipeline* current_pipeline;

    VkClearValue vk_clears[2]; // 0 color, 1 depth

    uint32 m_uFlags = CommandBufferFlags::None;

    uint32 handle;

    uint32 current_command;
    ResourceHandle resource_handle;
    QueueType type = QueueType::Graphics;
    uint32 buffer_size = 0;
    
}; // class CommandBuffer
} // namespace Graphics
} // namespace Raptor