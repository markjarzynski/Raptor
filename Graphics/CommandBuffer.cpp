#include "CommandBuffer.h"
#include "DescriptorSet.h"

namespace Raptor
{
namespace Graphics
{

CommandBuffer::CommandBuffer(QueueType type, uint32 bufferSize, uint32 submitSize, uint32 flags)
    : type(type), buffer_size(bufferSize), m_uFlags(flags)
{
    Reset();
}

CommandBuffer::~CommandBuffer()
{
    
}

void CommandBuffer::Init(QueueType queue_type, uint32 bufferSize, uint32 submitSize, uint32 flags)
{
    type = queue_type;
    buffer_size = bufferSize;
    m_uFlags = flags;

    Reset();
}

void CommandBuffer::Terminate()
{
    m_uFlags &= ~CommandBufferFlags::isRecording;
}

void CommandBuffer::BindPass(RenderPassHandle handle)
{
    m_uFlags |= CommandBufferFlags::isRecording;
    
    RenderPass* render_pass = gpu_device->AccessRenderPass(handle);

    if (current_render_pass && (current_render_pass->type != RenderPassType::Compute) && (render_pass != current_render_pass))
    {
        vkCmdEndRenderPass(vk_command_buffer);
    }

    if (render_pass != current_render_pass && (render_pass->type != RenderPassType::Compute))
    {
        VkRenderPassBeginInfo render_pass_begin {};
        render_pass_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin.framebuffer = (render_pass->type == RenderPassType::Swapchain) ? gpu_device->vk_swapchain_framebuffers[gpu_device->image_index] : render_pass->vk_framebuffer;
        render_pass_begin.renderPass = render_pass->vk_render_pass;
        render_pass_begin.renderArea.offset = {0, 0};
        render_pass_begin.renderArea.extent = {render_pass->width, render_pass->height};
        render_pass_begin.clearValueCount = 2;
        render_pass_begin.pClearValues = vk_clears;

        vkCmdBeginRenderPass(vk_command_buffer, &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);
    }

    current_render_pass = render_pass;
}

void CommandBuffer::BindPipeline(PipelineHandle handle)
{
    Pipeline* pipeline = gpu_device->AccessPipeline(handle);
    vkCmdBindPipeline(vk_command_buffer, pipeline->vk_pipeline_bind_point, pipeline->vk_pipeline);

    current_pipeline = pipeline;
}

void CommandBuffer::BindVertexBuffer(BufferHandle handle, uint32 binding, uint32 offset)
{
    Buffer* buffer = gpu_device->AccessBuffer(handle);
    VkDeviceSize offsets[] = {offset};
    
    VkBuffer vk_buffer = buffer->vk_buffer;
    if (buffer->parent_buffer != InvalidBuffer)
    {
        Buffer* parent_buffer = gpu_device->AccessBuffer(buffer->parent_buffer);
        vk_buffer = parent_buffer->vk_buffer;
        offsets[0] = buffer->global_offset;
    }

    vkCmdBindVertexBuffers(vk_command_buffer, binding, 1, &vk_buffer, offsets);
}

void CommandBuffer::BindIndexBuffer(BufferHandle handle, uint32 _offset, VkIndexType index_type)
{
    Buffer* buffer = gpu_device->AccessBuffer(handle);
    VkDeviceSize offset = _offset;

    VkBuffer vk_buffer = buffer->vk_buffer;
    if (buffer->parent_buffer != InvalidBuffer)
    {
        Buffer* parent_buffer = gpu_device->AccessBuffer(buffer->parent_buffer);
        vk_buffer = parent_buffer->vk_buffer;
        offset = buffer->global_offset;
    }

    vkCmdBindIndexBuffer(vk_command_buffer, vk_buffer, offset, index_type);
}

void CommandBuffer::BindDescriptorSet(DescriptorSetHandle* handles, uint32 num_lists, uint32* offsets, uint32 num_offsets)
{
    uint32 offset_cache[8];
    num_offsets = 0;

    for(uint32 i = 0; i < num_lists; i++)
    {
        DescriptorSet* descriptor_set = gpu_device->AccessDescriptorSet(handles[i]);
        vk_descriptor_sets[i] = descriptor_set->vk_descriptor_set;

        const DescriptorSetLayout* descriptor_set_layout = descriptor_set->layout;
        for (uint32 j = 0; j < descriptor_set_layout->num_bindings; j++)
        {
            const DescriptorBinding& rb = descriptor_set_layout->bindings[j];

            if (rb.vk_descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            {
                const uint32 resource_index = descriptor_set->bindings[j];
                ResourceHandle buffer_handle = descriptor_set->resources[resource_index];
                Buffer* buffer = gpu_device->AccessBuffer(buffer_handle);

                offset_cache[num_offsets++] = buffer->global_offset;
            }
        }
    }

    const uint32 FIRST_SET = 0;
    vkCmdBindDescriptorSets(vk_command_buffer, current_pipeline->vk_pipeline_bind_point, current_pipeline->vk_pipeline_layout, FIRST_SET, num_lists, vk_descriptor_sets, num_offsets, offset_cache);
}


void CommandBuffer::SetViewport(const Viewport* viewport)
{
    VkViewport vk_viewport;

    if (viewport)
    {
        vk_viewport.x = viewport->rect.x * 1.f;
        vk_viewport.y = viewport->rect.height * 1.f - viewport->rect.y;
        vk_viewport.width = viewport->rect.width * 1.f;
        vk_viewport.height = -viewport->rect.height * 1.f;
        vk_viewport.minDepth = viewport->min_depth;
        vk_viewport.maxDepth = viewport->max_depth;
    }
    else
    {
        vk_viewport.x = 0.f;

        if (current_render_pass)
        {
            vk_viewport.y = current_render_pass->height * 1.f;
            vk_viewport.width = current_render_pass->width * 1.f;
            vk_viewport.height = -current_render_pass->height * 1.f;
        }
        else
        {
            vk_viewport.y = gpu_device->swapchain_height * 1.f;
            vk_viewport.width = gpu_device->swapchain_width * 1.f;
            vk_viewport.height = -gpu_device->swapchain_height * 1.f;
        }
        vk_viewport.minDepth = 0.f;
        vk_viewport.maxDepth = 1.f;
    }

    vkCmdSetViewport(vk_command_buffer, 0, 1, &vk_viewport);
}

void CommandBuffer::SetScissor(const Rect2DInt* rect)
{
    VkRect2D vk_scissor;

    if (rect)
    {
        vk_scissor.offset.x = rect->x;
        vk_scissor.offset.y = rect->y;
        vk_scissor.extent.width = rect->width;
        vk_scissor.extent.height = rect->height;
    }
    else
    {
        vk_scissor.offset.x = 0;
        vk_scissor.offset.y = 0;
        vk_scissor.extent.width = gpu_device->swapchain_width;
        vk_scissor.extent.height = gpu_device->swapchain_height;
    }

    vkCmdSetScissor(vk_command_buffer, 0, 1, &vk_scissor);
}


void CommandBuffer::Clear(float red, float green, float blue, float alpha)
{
    vk_clears[0].color = {red, green, blue, alpha};
}

void CommandBuffer::ClearDepthStencil(float depth, uint8 stencil)
{
    vk_clears[1].depthStencil.depth = depth;
    vk_clears[1].depthStencil.stencil = stencil;
}

void CommandBuffer::Draw(TopologyType topology, uint32 first_vertex, uint32 vertex_count, uint32 first_instance, uint32 instance_count)
{
    vkCmdDraw(vk_command_buffer, vertex_count, instance_count, first_vertex, first_instance);
}

void CommandBuffer::DrawIndexed(TopologyType topology, uint32 index_count, uint32 instance_count, uint32 first_index, uint32 vertex_offset, uint32 first_instance)
{
    vkCmdDrawIndexed(vk_command_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
}

void CommandBuffer::DrawIndirect(BufferHandle handle, uint32 offset, uint32 stride)
{
    Buffer* buffer = gpu_device->AccessBuffer(handle);

    VkBuffer vk_buffer = buffer->vk_buffer;
    VkDeviceSize vk_offset = offset;

    vkCmdDrawIndirect(vk_command_buffer, vk_buffer, vk_offset, 1, sizeof(VkDrawIndirectCommand));
}

void CommandBuffer::DrawIndexedIndirect(BufferHandle handle, uint32 offset, uint32 stride)
{
    Buffer* buffer = gpu_device->AccessBuffer(handle);

    VkBuffer vk_buffer = buffer->vk_buffer;
    VkDeviceSize vk_offset = offset;

    vkCmdDrawIndexedIndirect(vk_command_buffer, vk_buffer, vk_offset, 1, sizeof(VkDrawIndirectCommand));
}

void CommandBuffer::Dispatch(uint32 group_x, uint32 group_y, uint32 group_z)
{
    vkCmdDispatch(vk_command_buffer, group_x, group_y, group_z);
}

void CommandBuffer::DispatchIndirect(BufferHandle handle, uint32 offset)
{
    Buffer* buffer = gpu_device->AccessBuffer(handle);

    VkBuffer vk_buffer = buffer->vk_buffer;
    VkDeviceSize vk_offset = offset;

    vkCmdDispatchIndirect(vk_command_buffer, vk_buffer, vk_offset);
}

static ResourceState ToResourceState(PipelineStage stage)
{
    static ResourceState states[] = {ResourceState::INDIRECT_ARGUMENT, ResourceState::VERTEX_AND_CONSTANT_BUFFER, ResourceState::NON_PIXEL_SHADER_RESOURCE, ResourceState::PIXEL_SHADER_RESOURCE, ResourceState::RENDER_TARGET, ResourceState::UNORDERED_ACCESS, ResourceState::COPY_DEST};
    return states[stage];
}

void CommandBuffer::Barrier(const ExecutionBarrier& barrier)
{
    if (current_render_pass && (current_render_pass->type != RenderPassType::Compute))
    {
        vkCmdEndRenderPass(vk_command_buffer);

        current_render_pass = nullptr;
    }

    static VkImageMemoryBarrier image_barriers[8];
    if (barrier.new_barrier_experimental != UINT32_MAX)
    {
        VkPipelineStageFlags source_stage_mask = 0;
        VkPipelineStageFlags destination_stage_mask = 0;
        VkAccessFlags source_access_flags = VK_ACCESS_NONE_KHR;
        VkAccessFlags destination_access_flags = VK_ACCESS_NONE_KHR;

        for (uint32 i = 0; i< barrier.num_image_barriers; i++)
        {
            Texture* texture = gpu_device->AccessTexture(barrier.image_barriers[i].texture);

            VkImageMemoryBarrier& vk_barrier = image_barriers[i];
            const bool is_color = !TextureFormat::HasDepthOrStencil(texture->vk_format);

            VkImageMemoryBarrier* pImageBarrier = &vk_barrier;
            pImageBarrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            pImageBarrier->pNext = NULL;

            ResourceState current_state = (barrier.src_pipeline_stage == PipelineStage::RenderTarget) ? ResourceState::RENDER_TARGET : ResourceState::PIXEL_SHADER_RESOURCE;
            ResourceState next_state = (barrier.dst_pipeline_stage == PipelineStage::RenderTarget) ? ResourceState::RENDER_TARGET : ResourceState::PIXEL_SHADER_RESOURCE;
            if (!is_color)
            {
                current_state = barrier.src_pipeline_stage == (PipelineStage::RenderTarget) ? ResourceState::DEPTH_WRITE : ResourceState::DEPTH_READ;
                next_state = barrier.dst_pipeline_stage == (PipelineStage::RenderTarget) ? ResourceState::DEPTH_WRITE : ResourceState::DEPTH_READ;
            }

            pImageBarrier->srcAccessMask = ToVkAccessFlags(current_state);
            pImageBarrier->dstAccessMask = ToVkAccessFlags(next_state);
            pImageBarrier->oldLayout = ToVkImageLayout(current_state);
            pImageBarrier->newLayout = ToVkImageLayout(next_state);
            pImageBarrier->image = texture->vk_image;
            pImageBarrier->subresourceRange.aspectMask = (is_color) ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            pImageBarrier->subresourceRange.baseMipLevel = 0;
            pImageBarrier->subresourceRange.levelCount = 1;
            pImageBarrier->subresourceRange.baseArrayLayer = 0;
            pImageBarrier->subresourceRange.layerCount = 1;
            pImageBarrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            pImageBarrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

            source_access_flags |= pImageBarrier->srcAccessMask;
            destination_access_flags |= pImageBarrier->dstAccessMask;

            vk_barrier.oldLayout = texture->vk_image_layout;
            texture->vk_image_layout = vk_barrier.newLayout;
        }

        static VkBufferMemoryBarrier buffer_memory_barriers[8];
        for (uint32 i = 0; i < barrier.num_memory_barriers; i++)
        {
            VkBufferMemoryBarrier& vk_barrier = buffer_memory_barriers[i];
            vk_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;

            Buffer* buffer = gpu_device->AccessBuffer(barrier.memory_barriers[i].buffer);

            vk_barrier.buffer = buffer->vk_buffer;
            vk_barrier.offset = 0;
            vk_barrier.size = buffer->size;

            ResourceState current_state = ToResourceState(barrier.src_pipeline_stage);
            ResourceState next_state = ToResourceState(barrier.dst_pipeline_stage);
            vk_barrier.srcAccessMask = ToVkAccessFlags(current_state);

            vk_barrier.dstAccessMask = ToVkAccessFlags(next_state);
            source_access_flags |= vk_barrier.srcAccessMask;
            destination_access_flags |= vk_barrier.dstAccessMask;

            vk_barrier.srcQueueFamilyIndex = 0;
            vk_barrier.dstQueueFamilyIndex = 0;
        }

        source_stage_mask = DeterminePipelineStageFlags(source_access_flags, (barrier.src_pipeline_stage == PipelineStage::ComputeShader) ? QueueType::Compute : QueueType::Graphics);
        destination_stage_mask = DeterminePipelineStageFlags(destination_access_flags, (barrier.dst_pipeline_stage == PipelineStage::ComputeShader) ? QueueType::Compute : QueueType::Graphics);

        vkCmdPipelineBarrier(vk_command_buffer, source_stage_mask, destination_stage_mask, 0, 0, nullptr, barrier.num_memory_barriers, buffer_memory_barriers, barrier.num_image_barriers, image_barriers);
        return;
    }

    VkImageLayout new_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkImageLayout new_depth_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    VkAccessFlags source_access_mask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
    VkAccessFlags source_buffer_access_mask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
    VkAccessFlags source_depth_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    VkAccessFlags destination_access_mask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
    VkAccessFlags destination_buffer_access_mask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
    VkAccessFlags destination_depth_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    switch (barrier.dst_pipeline_stage)
    {
        case PipelineStage::FragmentShader: break;
        case PipelineStage::ComputeShader:
        {
            new_layout = VK_IMAGE_LAYOUT_GENERAL;
        } break;
        case PipelineStage::RenderTarget:
        {
            new_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            new_depth_layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            destination_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
            destination_depth_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        } break;
        case PipelineStage::DrawIndirect:
        {
            destination_buffer_access_mask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
        } break;
    }

    switch (barrier.src_pipeline_stage)
    {
        case PipelineStage::FragmentShader: break;
        case PipelineStage::ComputeShader: break;
        case PipelineStage::RenderTarget:
        {
            source_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            source_depth_access_mask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        } break;
        case PipelineStage::DrawIndirect:
        {
            source_buffer_access_mask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
        } break;
    }

    bool has_depth = false;

    for (uint32 i = 0; i < barrier.num_image_barriers; i++)
    {
        Texture* texture = gpu_device->AccessTexture(barrier.image_barriers[i].texture);

        VkImageMemoryBarrier& vk_barrier = image_barriers[i];
        vk_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        vk_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        vk_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        const bool is_color = !TextureFormat::HasDepthOrStencil(texture->vk_format);
        has_depth = has_depth || !is_color;

        vk_barrier.image = texture->vk_image;
        vk_barrier.subresourceRange.aspectMask = (is_color) ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        vk_barrier.subresourceRange.baseMipLevel = 0;
        vk_barrier.subresourceRange.levelCount = 1;
        vk_barrier.subresourceRange.baseArrayLayer = 0;
        vk_barrier.subresourceRange.layerCount = 1;

        vk_barrier.oldLayout = texture->vk_image_layout;
        vk_barrier.newLayout = (is_color) ? new_layout : new_depth_layout;

        vk_barrier.srcAccessMask = (is_color) ? source_access_mask : source_depth_access_mask;
        vk_barrier.dstAccessMask = (is_color) ? destination_access_mask : destination_depth_access_mask;

        texture->vk_image_layout = vk_barrier.newLayout;
    }

    VkPipelineStageFlags source_stage_mask = ToVkPipelineStage((PipelineStage)barrier.src_pipeline_stage);
    VkPipelineStageFlags destination_stage_mask = ToVkPipelineStage((PipelineStage)barrier.dst_pipeline_stage);

    if (has_depth)
    {
        source_stage_mask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        destination_stage_mask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }

    static VkBufferMemoryBarrier buffer_memory_barriers[8];
    for (uint32 i = 0; i < barrier.num_memory_barriers; i++)
    {
        VkBufferMemoryBarrier& vk_barrier = buffer_memory_barriers[i];
        vk_barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;

        Buffer* buffer = gpu_device->AccessBuffer(barrier.memory_barriers[i].buffer);

        vk_barrier.buffer = buffer->vk_buffer;
        vk_barrier.offset = 0;
        vk_barrier.size = buffer->size;
        vk_barrier.srcAccessMask = source_buffer_access_mask;
        vk_barrier.dstAccessMask = destination_buffer_access_mask;

        vk_barrier.srcQueueFamilyIndex = 0;
        vk_barrier.dstQueueFamilyIndex = 0;
    }

    vkCmdPipelineBarrier(vk_command_buffer, source_stage_mask, destination_stage_mask, 0, 0, nullptr, barrier.num_memory_barriers, buffer_memory_barriers, barrier.num_image_barriers, image_barriers);
}

void CommandBuffer::FillBuffer(BufferHandle handle, uint32 offset, uint32 size, uint32 data)
{
    Buffer* buffer = gpu_device->AccessBuffer(handle);
    vkCmdFillBuffer(vk_command_buffer, buffer->vk_buffer, VkDeviceSize(offset), size ? VkDeviceSize(size) : VkDeviceSize(buffer->size), data);
}

void CommandBuffer::PushMarker(const char* name)
{
    gpu_device->PushGPUTimestamp(this, name);

    if (!gpu_device->m_uFlags & GPUDevice::Flags::DebugUtilsExtensionExist)
        return;
    
    gpu_device->PushMarker(vk_command_buffer, name);
}

void CommandBuffer::PopMarker()
{
    gpu_device->PopGPUTimestamp(this);

    if (!gpu_device->m_uFlags & GPUDevice::Flags::DebugUtilsExtensionExist)
        return;

    gpu_device->PopMarker(vk_command_buffer);
}

void CommandBuffer::Reset()
{
    m_uFlags &= ~CommandBufferFlags::isRecording;
    current_render_pass = nullptr;
    current_pipeline = nullptr;
    current_command = 0;
}

} // namespace Graphics
} // namespace Raptor