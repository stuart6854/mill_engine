#include "upload_context_vk.hpp"

#include "device_vk.hpp"
#include "resources_vk.hpp"

namespace mill::platform::vulkan
{
    constexpr u64 g_DefaultHeapSize = 10 << 20;  // 1MB

    UploadContextVulkan::UploadContextVulkan(DeviceVulkan& device) : m_device(device), m_heapSize(g_DefaultHeapSize)
    {
        m_transferQueue = m_device.get_transfer_queue();

        vk::CommandPoolCreateInfo pool_info{};
        pool_info.setQueueFamilyIndex(m_device.get_transfer_queue_family());
        pool_info.setFlags(vk::CommandPoolCreateFlagBits::eTransient | vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        m_cmdPool = m_device.get_device().createCommandPool(pool_info);

        m_currentPendingUpload = prepare_pending_upload();

        m_workerThread = std::thread(&UploadContextVulkan::thread_func, this);
    }

    UploadContextVulkan::~UploadContextVulkan()
    {
        m_workerStopFlag = true;
        m_condVar.notify_one();
        m_workerThread.join();

        m_transferQueue.waitIdle();

        while (!m_pendingUploads.empty())
        {
            destroy_pending_upload(m_pendingUploads.front());
            m_pendingUploads.pop();
        }
        destroy_pending_upload(m_currentPendingUpload);
        m_currentPendingUpload = {};

        m_device.get_device().destroy(m_cmdPool);
    }

    void UploadContextVulkan::add_buffer_upload(BufferVulkan& dst_buffer, u64 size_bytes, const void* data)
    {
        // Have we actually tried to upload anything?
        if (size_bytes == 0)
        {
            return;
        }

        ASSERT(size_bytes <= m_heapSize);

        if (m_currentPendingUpload.heapOffset + size_bytes > m_currentPendingUpload.uploadHeap->size)
        {
            flush();
        }

        auto& upload = m_currentPendingUpload.bufferUploads.emplace_back();
        upload.dst_buffer = &dst_buffer;
        upload.heap_offset = m_currentPendingUpload.heapOffset;
        upload.size_bytes = size_bytes;

        // Copy data to heap
        u8* offset_mapped_ptr = static_cast<u8*>(m_currentPendingUpload.uploadHeap->mappedPtr) + m_currentPendingUpload.heapOffset;
        std::memcpy(offset_mapped_ptr, data, size_bytes);

        m_currentPendingUpload.heapOffset += size_bytes;

        vk::BufferCopy copy_region{};
        copy_region.setSrcOffset(upload.heap_offset);
        copy_region.setDstOffset(0);
        copy_region.setSize(upload.size_bytes);
        m_currentPendingUpload.cmdBuffer.copyBuffer(m_currentPendingUpload.uploadHeap->buffer, dst_buffer.buffer, copy_region);
    }

    void UploadContextVulkan::add_image_upload(ImageVulkan& dst_image, u64 size_bytes, const void* data, u32 mip_level)
    {
        // Have we actually tried to upload anything?
        if (size_bytes == 0)
        {
            return;
        }

        ASSERT(size_bytes <= m_heapSize);

        if (m_currentPendingUpload.heapOffset + size_bytes > m_currentPendingUpload.uploadHeap->size)
        {
            flush();
        }

        auto& upload = m_currentPendingUpload.imageUploads.emplace_back();
        upload.dst_image = &dst_image;
        upload.heap_offset = m_currentPendingUpload.heapOffset;
        upload.size_bytes = size_bytes;

        // Copy data to heap
        u8* offset_mapped_ptr = static_cast<u8*>(m_currentPendingUpload.uploadHeap->mappedPtr) + m_currentPendingUpload.heapOffset;
        std::memcpy(offset_mapped_ptr, data, size_bytes);

        vk::BufferImageCopy copy_region{};
        copy_region.setBufferOffset(upload.heap_offset);
        copy_region.setImageExtent(dst_image.extent);
        copy_region.setImageSubresource(get_image_subresource_layers_2d(dst_image.init.format, mip_level));

        add_barrier(dst_image, vk::ImageLayout::eTransferDstOptimal);

        m_currentPendingUpload.cmdBuffer.copyBufferToImage(
            m_currentPendingUpload.uploadHeap->buffer, dst_image.image, dst_image.layout, copy_region);

        add_barrier(dst_image, vk::ImageLayout::eShaderReadOnlyOptimal);

        m_currentPendingUpload.heapOffset += size_bytes;
    }

    void UploadContextVulkan::flush()
    {
        // Have we actually tried to upload anything?
        if (m_currentPendingUpload.heapOffset == 0)
        {
            return;
        }

        LOG_DEBUG("UploadContextVulkan - Flushing upload (size = {})...", m_currentPendingUpload.heapOffset);

        m_currentPendingUpload.cmdBuffer.end();

        // Submit current pending upload
        vk::SubmitInfo submit_info{};
        submit_info.setCommandBuffers(m_currentPendingUpload.cmdBuffer);
        m_transferQueue.submit(submit_info, m_currentPendingUpload.fence);

        {
            std::lock_guard lock(m_mutex);
            m_pendingUploads.push(m_currentPendingUpload);
        }
        m_condVar.notify_one();  // Tell worker queue to wake up

        m_currentPendingUpload = prepare_pending_upload();
    }

    void UploadContextVulkan::add_barrier(ImageVulkan& image_resource, vk::ImageLayout new_layout)
    {
        auto old_layout = image_resource.layout;

        vk::PipelineStageFlags2 srcStage{};
        vk::AccessFlags2 srcAccess{};
        vk::PipelineStageFlags2 dstStage{};
        vk::AccessFlags2 dstAccess{};
        if (new_layout == vk::ImageLayout::eTransferDstOptimal)
        {
            srcStage = vk::PipelineStageFlagBits2::eTopOfPipe;

            dstStage = vk::PipelineStageFlagBits2::eTransfer;
            dstAccess = vk::AccessFlagBits2::eTransferWrite;
        }
        else if (new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
        {
            srcStage = vk::PipelineStageFlagBits2::eTransfer;
            srcAccess = vk::AccessFlagBits2::eTransferWrite;

            dstStage = vk::PipelineStageFlagBits2::eBottomOfPipe;
            dstAccess = vk::AccessFlagBits2::eShaderRead;
        }

        vk::ImageMemoryBarrier2 image_barrier{};
        image_barrier.setSrcStageMask(srcStage);
        image_barrier.setDstStageMask(dstStage);
        image_barrier.setOldLayout(old_layout);
        image_barrier.setNewLayout(new_layout);
        image_barrier.setImage(image_resource.image);
        image_barrier.setSubresourceRange(image_resource.range);

        vk::DependencyInfo dep_info{};
        dep_info.setImageMemoryBarriers(image_barrier);
        m_currentPendingUpload.cmdBuffer.pipelineBarrier2(dep_info);

        image_resource.layout = new_layout;
    }

    auto UploadContextVulkan::prepare_pending_upload() -> PendingUpload
    {
        vk::CommandBufferAllocateInfo alloc_info{};
        alloc_info.setCommandPool(m_cmdPool);
        alloc_info.setCommandBufferCount(1);
        alloc_info.setLevel(vk::CommandBufferLevel::ePrimary);
        auto cmd_buffer = m_device.get_device().allocateCommandBuffers(alloc_info)[0];

        auto fence = m_device.get_device().createFence({});

        BufferInit buffer_init{};
        buffer_init.size = m_heapSize;
        buffer_init.usage = vk::BufferUsageFlagBits::eTransferSrc;
        buffer_init.isCPUVisible = true;

        PendingUpload pending_upload{};
        pending_upload.cmdBuffer = cmd_buffer;
        pending_upload.fence = fence;
        pending_upload.uploadHeap = m_device.create_buffer(buffer_init);

        vk::CommandBufferBeginInfo begin_info{};
        pending_upload.cmdBuffer.begin(begin_info);

        return pending_upload;
    }

    void UploadContextVulkan::destroy_pending_upload(PendingUpload& pending_upload)
    {
        m_device.get_device().freeCommandBuffers(m_cmdPool, pending_upload.cmdBuffer);
        m_device.get_device().destroy(pending_upload.fence);
        m_device.destroy_buffer(pending_upload.uploadHeap);
    }

    void UploadContextVulkan::thread_func()
    {
        LOG_DEBUG("UploadContextVulkan - Worker thread has started.");

        PendingUpload pending_upload{};
        while (!m_workerStopFlag)
        {
            {
                std::unique_lock lock{ m_mutex };
                m_condVar.wait(lock, [this]() { return !m_pendingUploads.empty() || m_workerStopFlag; });

                if (m_workerStopFlag)
                {
                    break;
                }

                pending_upload = m_pendingUploads.front();
                m_pendingUploads.pop();
            }
            LOG_DEBUG("UploadContextVulkan - Worker - There is an upload pending completion...");

            UNUSED(m_device.get_device().waitForFences(pending_upload.fence, true, u64_max));
            LOG_DEBUG("UploadContextVulkan - Worker - An upload has been completed...");

            // Process completed uploads
            for (auto& upload : pending_upload.bufferUploads)
            {
                upload.dst_buffer->is_ready = true;
            }
            for (auto& upload : pending_upload.imageUploads)
            {
                upload.dst_image->is_ready = true;
            }

            destroy_pending_upload(pending_upload);
        }

        LOG_DEBUG("UploadContextVulkan - Worker thread has finished.");
    }

}
