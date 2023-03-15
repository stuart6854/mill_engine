#pragma once

#include "common_headers_vk.hpp"

namespace mill::platform::vulkan
{
    class DeviceVulkan;

    class UploadContextVulkan
    {
    public:
        UploadContextVulkan(DeviceVulkan& device);
        ~UploadContextVulkan();

        void add_buffer_upload();

    private:
        DeviceVulkan& m_device;

    };
}