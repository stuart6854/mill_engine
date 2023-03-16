#pragma once

#include "mill/core/base.hpp"
#include "common_headers_vk.hpp"

#include <queue>
#include <thread>

namespace mill::platform::vulkan
{
    constexpr u32 g_UploadFrameCount = 3;

    class DeviceVulkan;
    struct BufferVulkan;
    struct ImageVulkan;

    class UploadContextVulkan
    {
    public:
        UploadContextVulkan(DeviceVulkan& device);
        ~UploadContextVulkan();

        void add_buffer_upload(BufferVulkan& dst_buffer, u64 size_bytes, const void* data);
        void add_image_upload(ImageVulkan& dst_image, u64 size_bytes, const void* data, u32 mip_level = 0);

        void flush();

    private:
        void add_barrier(ImageVulkan& image_resource, vk::ImageLayout new_layout);

        struct PendingUpload;
        auto prepare_pending_upload() -> PendingUpload;
        void destroy_pending_upload(PendingUpload& pending_upload);

        void thread_func();

    private:
        DeviceVulkan& m_device;
        u64 m_heapSize{};

        vk::Queue m_transferQueue{};
        vk::CommandPool m_cmdPool{};

        std::thread m_workerThread{};
        std::atomic_bool m_workerStopFlag{ false };

        struct BufferUpload
        {
            BufferVulkan* dst_buffer{ nullptr };
            u64 size_bytes{};
            u64 heap_offset{};
        };
        struct ImageUpload
        {
            ImageVulkan* dst_image{ nullptr };
            u64 size_bytes{};
            u64 heap_offset{};
        };

        struct PendingUpload
        {
            vk::CommandBuffer cmdBuffer{};
            vk::Fence fence{};

            BufferVulkan* uploadHeap{ nullptr };
            u64 heapOffset{};

            std::vector<BufferUpload> bufferUploads{};
            std::vector<ImageUpload> imageUploads{};
        };
        PendingUpload m_currentPendingUpload{};

        std::queue<PendingUpload> m_pendingUploads{};
        std::condition_variable m_condVar{};
        std::mutex m_mutex{};
    };
}