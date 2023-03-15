#include "upload_context_vk.hpp"

#include "device_vk.hpp"
#include "resources_vk.hpp"

namespace mill::platform::vulkan
{
    UploadContextVulkan::UploadContextVulkan(DeviceVulkan& device, u64 heap_size) : m_device(device), m_heapSize(heap_size)
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

    void UploadContextVulkan::flush()
    {
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

            destroy_pending_upload(pending_upload);
        }

        LOG_DEBUG("UploadContextVulkan - Worker thread has finished.");
    }

}
