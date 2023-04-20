#include "vulkan_helpers.hpp"

namespace mill::rhi::vulkan
{
    bool is_instance_ext_supported(const std::string_view& ext_name)
    {
        auto inst_exts = vk::enumerateInstanceExtensionProperties();
        for (const auto& ext : inst_exts)
        {
            if (ext.extensionName == ext_name)
            {
                return true;
            }
        }
        return false;
    }

    bool is_instance_layer_supported(const std::string_view& layer_name)
    {
        auto inst_layers = vk::enumerateInstanceLayerProperties();
        for (const auto& layer : inst_layers)
        {
            if (layer.layerName == layer_name)
            {
                return true;
            }
        }
        return false;
    }

    auto get_best_dedicated_gpu(vk::Instance instance) -> vk::PhysicalDevice
    {
        auto physicalDevices = instance.enumeratePhysicalDevices();

        u64 best_score{};
        vk::PhysicalDevice best_gpu{};
        for (auto gpu : physicalDevices)
        {
            u64 score = 0;

            auto props = gpu.getProperties();
            if (props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
            {
                score += 1000;
            }
            else if (props.deviceType == vk::PhysicalDeviceType::eIntegratedGpu)
            {
                score += 500;
            }

            score += props.limits.maxImageDimension2D;

            auto mem_props = gpu.getMemoryProperties();
            for (auto& heap : mem_props.memoryHeaps)
            {
                if (heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal)
                {
                    score += heap.size;
                    break;
                }
            }

            if (score > best_score)
            {
                best_score = score;
                best_gpu = gpu;
            }
        }
        return best_gpu;
    }

    bool is_device_ext_supported(vk::PhysicalDevice physical_device, const std::string_view& ext_name)
    {
        auto device_exts = physical_device.enumerateDeviceExtensionProperties();
        for (const auto& ext : device_exts)
        {
            if (ext.extensionName == ext_name)
            {
                return true;
            }
        }
        return false;
    }

    auto find_graphics_queue_family(vk::PhysicalDevice physical_device) -> i32
    {
        auto queue_props = physical_device.getQueueFamilyProperties();

        // Try find a Graphics queue without compute
        for (u32 i = 0; i < queue_props.size(); ++i)
        {
            const auto& family = queue_props[i];
            if (family.queueFlags & vk::QueueFlagBits::eGraphics && !(family.queueFlags & vk::QueueFlagBits::eCompute))
            {
                return i;
            }
        }

        // Now, just return the first Graphics queue available
        for (u32 i = 0; i < queue_props.size(); ++i)
        {
            const auto& family = queue_props[i];
            if (family.queueFlags & vk::QueueFlagBits::eGraphics)
            {
                return i;
            }
        }

        return -1;
    }

    auto find_transfer_queue_family(vk::PhysicalDevice physical_device) -> i32
    {
        auto queue_props = physical_device.getQueueFamilyProperties();

        // Try find a combined Transfer, Graphics & Compute queue
        for (u32 i = 0; i < queue_props.size(); ++i)
        {
            const auto& family = queue_props[i];
            if (family.queueFlags & (vk::QueueFlagBits::eTransfer | vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute))
            {
                return i;
            }
        }

        // Now, try find a combined Transfer & Graphics queue
        for (u32 i = 0; i < queue_props.size(); ++i)
        {
            const auto& family = queue_props[i];
            if (family.queueFlags & (vk::QueueFlagBits::eTransfer | vk::QueueFlagBits::eGraphics))
            {
                return i;
            }
        }

        // Now, just return the first Transfer queue available
        for (u32 i = 0; i < queue_props.size(); ++i)
        {
            const auto& family = queue_props[i];
            if (family.queueFlags & vk::QueueFlagBits::eTransfer)
            {
                return i;
            }
        }

        return -1;
    }

    void set_object_name(vk::Device device, u64 object, vk::ObjectType type, const char* name)
    {
#ifndef MILL_DISTRO
        vk::DebugUtilsObjectNameInfoEXT name_info{};
        name_info.setObjectHandle(object);
        name_info.setObjectType(type);
        name_info.setPObjectName(name);
        device.setDebugUtilsObjectNameEXT(name_info);
#endif
    }

    auto select_swapchain_present_mode(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface, bool vsync) -> vk::PresentModeKHR
    {
        auto present_modes = physical_device.getSurfacePresentModesKHR(surface);

        // If vsync is not requested, return Immediate, if supported.
        if (!vsync)
        {
            for (auto mode : present_modes)
            {
                if (mode == vk::PresentModeKHR::eImmediate)
                {
                    return vk::PresentModeKHR::eImmediate;
                }
            }
        }

#define MILL_VULKAN_ENABLE_MAILBOX 0
#if MILL_VULKAN_ENABLE_MAILBOX
        // Try to return eMailbox, if supported.
        // Mailbox will render as fast as possible without screen tearing
        for (auto mode : present_modes)
        {
            if (mode == vk::PresentModeKHR::eMailbox)
            {
                return vk::PresentModeKHR::eMailbox;
            }
        }
#endif

        // FIFO is required to be supported, according to Vulkan specs.
        return vk::PresentModeKHR::eFifo;
    }

    auto select_swapchain_surface_format(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface) -> vk::SurfaceFormatKHR
    {
        auto supported_formats = physical_device.getSurfaceFormatsKHR(surface);
        ASSERT(!supported_formats.empty());

        std::vector preferred_formats{
            vk::Format::eB8G8R8A8Srgb,
            vk::Format::eR8G8B8A8Srgb,
            vk::Format::eB8G8R8A8Unorm,
            vk::Format::eR8G8B8A8Unorm,
        };

        for (auto preferred_format : preferred_formats)
        {
            for (auto supported_format : supported_formats)
            {
                if (supported_format.format == preferred_format)
                {
                    return supported_format;
                }
            }
        }

        return { vk::Format::eB8G8R8A8Srgb, vk::ColorSpaceKHR::eSrgbNonlinear };
    }

    auto select_swapchain_extent(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface, u32 width, u32 height) -> vk::Extent2D
    {
        auto caps = physical_device.getSurfaceCapabilitiesKHR(surface);

        width = std::clamp(width, caps.minImageExtent.width, caps.maxImageExtent.width);
        height = std::clamp(height, caps.minImageExtent.height, caps.maxImageExtent.height);

        return vk::Extent2D(width, height);
    }

    auto select_swapchain_image_count(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface) -> u32
    {
        auto caps = physical_device.getSurfaceCapabilitiesKHR(surface);

        u32 image_count = caps.minImageCount + 1;
        if (caps.maxImageCount > 0 && image_count > caps.maxImageCount)
        {
            image_count = caps.maxImageCount;
        }
        return image_count;
    }

    auto get_image_view_create_info_2d(vk::Image image, vk::Format format, u32 base_mip_lvl, u32 mip_lvl_count) -> vk::ImageViewCreateInfo
    {
        vk::ImageViewCreateInfo view_info{};
        view_info.setImage(image);
        view_info.setFormat(format);
        view_info.setViewType(vk::ImageViewType::e2D);
        view_info.subresourceRange.setAspectMask(get_image_aspect_from_format(format));
        view_info.subresourceRange.setBaseMipLevel(base_mip_lvl);
        view_info.subresourceRange.setLevelCount(mip_lvl_count);
        view_info.subresourceRange.setBaseArrayLayer(0);
        view_info.subresourceRange.setLayerCount(1);
        return view_info;
    }

    auto get_image_subresource_layers_2d(vk::Format format, u32 mip_lvl) -> vk::ImageSubresourceLayers
    {
        vk::ImageSubresourceLayers layer{};
        layer.setAspectMask(get_image_aspect_from_format(format));
        layer.setMipLevel(mip_lvl);
        layer.setBaseArrayLayer(0);
        layer.setLayerCount(1);
        return layer;
    }

    auto get_image_subresource_range_2d(vk::Format format, u32 base_mip_lvl, u32 mip_lvl_count) -> vk::ImageSubresourceRange
    {
        vk::ImageSubresourceRange range{};
        range.setAspectMask(get_image_aspect_from_format(format));
        range.setBaseMipLevel(base_mip_lvl);
        range.setLevelCount(mip_lvl_count);
        range.setBaseArrayLayer(0);
        range.setLayerCount(1);
        return range;
    }

    DescriptorSetLayout::DescriptorSetLayout(vk::Device device) : m_device(device) {}

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        reset();
    }

    void DescriptorSetLayout::reset()
    {
        m_layoutInfo = vk::DescriptorSetLayoutCreateInfo();
        m_bindings.clear();
        if (m_layout)
        {
            m_device.destroy(m_layout);
        }
    }

    void DescriptorSetLayout::add_binding(u32 binding,
                                          vk::DescriptorType descriptor_type,
                                          vk::ShaderStageFlags shader_stages,
                                          u32 count,
                                          vk::DescriptorBindingFlags binding_flags)
    {
        if (m_bindings.size() < binding + 1)
        {
            m_bindings.resize(binding + 1);
            m_bindingFlags.resize(binding + 1);
        }

        auto& set_binding = m_bindings[binding];
        set_binding.setBinding(binding);
        set_binding.setDescriptorCount(count);
        set_binding.setDescriptorType(descriptor_type);
        set_binding.setStageFlags(shader_stages);

        m_bindingFlags[binding] = binding_flags;
    }

    void DescriptorSetLayout::build()
    {
        if (m_layout)
        {
            m_device.destroy(m_layout);
        }

        vk::DescriptorSetLayoutBindingFlagsCreateInfo binding_flags_info{};
        binding_flags_info.setBindingFlags(m_bindingFlags);

        m_layoutInfo.setBindings(m_bindings);
        m_layoutInfo.setPNext(&binding_flags_info);
        m_layout = m_device.createDescriptorSetLayout(m_layoutInfo);
    }

    auto DescriptorSetLayout::get_hash() const -> hasht
    {
        return calculate_hash();
    }

    auto DescriptorSetLayout::get_layout() const -> vk::DescriptorSetLayout
    {
        return m_layout;
    }

    auto DescriptorSetLayout::calculate_hash() const -> hasht
    {
        hasht final_hash{ 0 };

        for (auto& binding : m_bindings)
        {
            hasht hash{ 0 };
            hash_combine(hash, binding.binding);
            hash_combine(hash, binding.descriptorType);
            hash_combine(hash, binding.descriptorCount);
            hash_combine(hash, static_cast<u32>(binding.stageFlags));

            hash_combine(final_hash, hash);
        }

        return final_hash;
    }

    PipelineLayout::PipelineLayout(vk::Device device) : m_device(device) {}

    PipelineLayout::~PipelineLayout()
    {
        reset();
    }

    void PipelineLayout::reset()
    {
        m_layoutInfo = vk::PipelineLayoutCreateInfo();
        m_pushConstantRanges.clear();
        m_descriptorSetLayouts.clear();
        if (m_layout)
        {
            m_device.destroy(m_layout);
        }
    }

    void PipelineLayout::add_push_constant_range(vk::ShaderStageFlags stages, u32 offset_bytes, u32 size_bytes)
    {
        auto& push_range = m_pushConstantRanges.emplace_back();
        push_range.setStageFlags(stages);
        push_range.setOffset(offset_bytes);
        push_range.setSize(size_bytes);
    }

    void PipelineLayout::add_descriptor_set_layout(u32 set, const DescriptorSetLayout& descriptor_layout)
    {
        if (m_descriptorSetLayouts.size() < set + 1)
        {
            m_descriptorSetLayouts.resize(set + 1);
        }

        m_descriptorSetLayouts[set] = &descriptor_layout;
    }

    void PipelineLayout::build()
    {
        if (m_layout)
        {
            m_device.destroy(m_layout);
        }

        std::vector<vk::DescriptorSetLayout> set_layouts{};
        for (auto* set_layout : m_descriptorSetLayouts)
        {
            set_layouts.push_back(set_layout->get_layout());
        }

        m_layoutInfo.setPushConstantRanges(m_pushConstantRanges);
        m_layoutInfo.setSetLayouts(set_layouts);
        m_layout = m_device.createPipelineLayout(m_layoutInfo);
    }

    auto PipelineLayout::get_hash() const -> u64
    {
        return calculate_hash();
    }

    auto PipelineLayout::get_layout() const -> vk::PipelineLayout
    {
        return m_layout;
    }

    auto PipelineLayout::calculate_hash() const -> hasht
    {
        hasht final_hash{ 0 };
        for (auto& push_range : m_pushConstantRanges)
        {
            hasht hash{ 0 };
            hash_combine(hash, static_cast<u32>(push_range.stageFlags));
            hash_combine(hash, push_range.offset);
            hash_combine(hash, push_range.size);

            hash_combine(final_hash, hash);
        }

        for (auto* set_layout : m_descriptorSetLayouts)
        {
            hash_combine(final_hash, set_layout->get_hash());
        }

        return final_hash;
    }

    DescriptorSet::DescriptorSet(vk::Device device, vk::DescriptorPool descriptor_pool, DescriptorSetLayout& layout)
        : m_device(device), m_pool(descriptor_pool), m_layout(layout)
    {
        auto set_layout = m_layout.get_layout();

        vk::DescriptorSetAllocateInfo alloc_info{};
        alloc_info.setDescriptorPool(m_pool);
        alloc_info.setDescriptorSetCount(1);
        alloc_info.setSetLayouts(set_layout);
        m_set = m_device.allocateDescriptorSets(alloc_info)[0];
    }

    DescriptorSet::~DescriptorSet()
    {
        m_device.freeDescriptorSets(m_pool, m_set);
    }

    void DescriptorSet::bind_buffer(u32 binding, vk::Buffer buffer, u64 range_bytes)
    {
        auto& buffer_info = m_bufferInfos.emplace_back();
        buffer_info.setBuffer(buffer);
        buffer_info.setOffset(0);
        buffer_info.setRange(range_bytes);

        auto& write = m_pendingWrites.emplace_back();
        write.setDstSet(m_set);
        write.setDstBinding(binding);
        write.setDescriptorCount(1);
        write.setDescriptorType(vk::DescriptorType::eUniformBuffer);
        write.setDstArrayElement(0);
        write.setBufferInfo(buffer_info);
    }

    void DescriptorSet::bind_image(u32 binding, vk::ImageView image_view, vk::Sampler sampler, u32 array_index)
    {
        auto image_info = CreateOwned<vk::DescriptorImageInfo>();
        image_info->setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        image_info->setImageView(image_view);
        image_info->setSampler(sampler);
        m_imageInfos.push_back(std::move(image_info));

        auto& write = m_pendingWrites.emplace_back();
        write.setDstSet(m_set);
        write.setDstBinding(binding);
        write.setDescriptorCount(1);
        write.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        write.setDstArrayElement(array_index);
        write.setPImageInfo(m_imageInfos.back().get());
    }

    void DescriptorSet::flush_writes()
    {
        m_device.updateDescriptorSets(m_pendingWrites, {});
        m_pendingWrites.clear();
        m_bufferInfos.clear();
        m_imageInfos.clear();
    }

    auto DescriptorSet::get_set() const -> vk::DescriptorSet
    {
        return m_set;
    }

    auto get_image_layout_stage_mask(vk::ImageLayout layout) -> vk::PipelineStageFlagBits2
    {
        switch (layout)
        {
            case vk::ImageLayout::eUndefined: return vk::PipelineStageFlagBits2::eTopOfPipe;
            case vk::ImageLayout::eTransferSrcOptimal: return vk::PipelineStageFlagBits2::eTransfer;
            case vk::ImageLayout::eTransferDstOptimal: return vk::PipelineStageFlagBits2::eTransfer;
            case vk::ImageLayout::eAttachmentOptimal: return vk::PipelineStageFlagBits2::eColorAttachmentOutput;
            case vk::ImageLayout::ePresentSrcKHR: return vk::PipelineStageFlagBits2::eBottomOfPipe;
            default: ASSERT(("Unsupported vk::ImageLayout!", false)); break;
        }
        return {};
    }

    auto get_image_layout_access_mask(vk::ImageLayout layout) -> vk::AccessFlagBits2
    {
        switch (layout)
        {
            case vk::ImageLayout::eUndefined: return vk::AccessFlagBits2::eNone;
            case vk::ImageLayout::eTransferSrcOptimal: return vk::AccessFlagBits2::eTransferRead;
            case vk::ImageLayout::eTransferDstOptimal: return vk::AccessFlagBits2::eTransferWrite;
            case vk::ImageLayout::eAttachmentOptimal: return vk::AccessFlagBits2::eColorAttachmentWrite;
            case vk::ImageLayout::ePresentSrcKHR: return vk::AccessFlagBits2::eNone;
            default: ASSERT(("Unsupported vk::ImageLayout!", false)); break;
        }
        return {};
    }

    auto get_barrier_image_to_transfer_src(vk::Image image, vk::Format format, vk::ImageLayout old_layout) -> vk::ImageMemoryBarrier2
    {
        const vk::ImageLayout new_layout = vk::ImageLayout::eTransferSrcOptimal;
        if (old_layout == new_layout)
        {
            return {};
        }

        auto range = get_image_subresource_range_2d(format);

        vk::ImageMemoryBarrier2 barrier{};
        barrier.setImage(image);
        barrier.setSubresourceRange(range);
        barrier.setNewLayout(new_layout);
        barrier.setDstAccessMask(vk::AccessFlagBits2::eTransferRead);
        barrier.setDstStageMask(vk::PipelineStageFlagBits2::eTransfer);

        barrier.setOldLayout(old_layout);
        if (old_layout == vk::ImageLayout::eUndefined)
        {
            barrier.setSrcAccessMask(vk::AccessFlagBits2::eNone);
            barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe);
        }
        else if (old_layout == vk::ImageLayout::eAttachmentOptimal)
        {
            barrier.setSrcAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite);
            barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);
        }
        else
        {
            LOG_ERROR("Unsupported `old_layout` for image barrier!");
            ASSERT(false);
        }

        return barrier;
    }

    auto get_barrier_image_to_transfer_dst(vk::Image image, vk::Format format, vk::ImageLayout old_layout) -> vk::ImageMemoryBarrier2
    {
        const vk::ImageLayout new_layout = vk::ImageLayout::eTransferDstOptimal;
        if (old_layout == new_layout)
        {
            return {};
        }

        auto range = get_image_subresource_range_2d(format);

        vk::ImageMemoryBarrier2 barrier{};
        barrier.setImage(image);
        barrier.setSubresourceRange(range);
        barrier.setNewLayout(new_layout);
        barrier.setDstAccessMask(vk::AccessFlagBits2::eTransferWrite);
        barrier.setDstStageMask(vk::PipelineStageFlagBits2::eTransfer);

        barrier.setOldLayout(old_layout);
        if (old_layout == vk::ImageLayout::eUndefined)
        {
            barrier.setSrcAccessMask(vk::AccessFlagBits2::eNone);
            barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe);
        }
        else if (old_layout == vk::ImageLayout::eAttachmentOptimal)
        {
            barrier.setSrcAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite);
            barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);
        }
        else if (old_layout == vk::ImageLayout::ePresentSrcKHR)
        {
            barrier.setSrcAccessMask(vk::AccessFlagBits2::eNone);
            barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eBottomOfPipe);
        }
        else
        {
            LOG_ERROR("Unsupported `old_layout` for image barrier!");
            ASSERT(false);
        }

        return barrier;
    }

    auto get_barrier_image_to_attachment(vk::Image image, vk::Format format, vk::ImageLayout old_layout) -> vk::ImageMemoryBarrier2
    {
        const vk::ImageLayout new_layout = vk::ImageLayout::eAttachmentOptimal;
        if (old_layout == new_layout)
        {
            return {};
        }

        auto range = get_image_subresource_range_2d(format);

        vk::ImageMemoryBarrier2 barrier{};
        barrier.setImage(image);
        barrier.setSubresourceRange(range);
        barrier.setNewLayout(new_layout);
        barrier.setDstAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite);
        barrier.setDstStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);

        barrier.setOldLayout(old_layout);
        if (old_layout == vk::ImageLayout::eUndefined)
        {
            barrier.setSrcAccessMask(vk::AccessFlagBits2::eNone);
            barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe);
        }
        else if (old_layout == vk::ImageLayout::eTransferSrcOptimal)
        {
            barrier.setSrcAccessMask(vk::AccessFlagBits2::eTransferRead);
            barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer);
        }
        else if (old_layout == vk::ImageLayout::eTransferDstOptimal)
        {
            barrier.setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite);
            barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer);
        }
        else
        {
            LOG_ERROR("Unsupported `old_layout` for image barrier!");
            ASSERT(false);
        }

        return barrier;
    }

    auto get_barrier_image_to_present(vk::Image image, vk::Format format, vk::ImageLayout old_layout) -> vk::ImageMemoryBarrier2
    {
        const vk::ImageLayout new_layout = vk::ImageLayout::ePresentSrcKHR;
        if (old_layout == new_layout)
        {
            return {};
        }

        auto range = get_image_subresource_range_2d(format);

        vk::ImageMemoryBarrier2 barrier{};
        barrier.setImage(image);
        barrier.setSubresourceRange(range);
        barrier.setNewLayout(new_layout);
        barrier.setDstAccessMask(vk::AccessFlagBits2::eNone);
        barrier.setDstStageMask(vk::PipelineStageFlagBits2::eBottomOfPipe);

        barrier.setOldLayout(old_layout);
        if (old_layout == vk::ImageLayout::eUndefined)
        {
            barrier.setSrcAccessMask(vk::AccessFlagBits2::eNone);
            barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eTopOfPipe);
        }
        else if (old_layout == vk::ImageLayout::eAttachmentOptimal)
        {
            barrier.setSrcAccessMask(vk::AccessFlagBits2::eColorAttachmentWrite);
            barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);
        }
        else if (old_layout == vk::ImageLayout::eTransferSrcOptimal)
        {
            barrier.setSrcAccessMask(vk::AccessFlagBits2::eTransferRead);
            barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer);
        }
        else if (old_layout == vk::ImageLayout::eTransferDstOptimal)
        {
            barrier.setSrcAccessMask(vk::AccessFlagBits2::eTransferWrite);
            barrier.setSrcStageMask(vk::PipelineStageFlagBits2::eTransfer);
        }
        else
        {
            LOG_ERROR("Unsupported `old_layout` for image barrier!");
            ASSERT(false);
        }

        return barrier;
    }

    bool is_depth_format(vk::Format format)
    {
        constexpr vk::Format depth_formats[]{
            vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint,
            vk::Format::eD16Unorm,  vk::Format::eD16UnormS8Uint,
        };
        return std::ranges::find(depth_formats, format) != std::end(depth_formats);
    }

    bool is_stencil_format(vk::Format format)
    {
        constexpr vk::Format stencil_formats[]{
            vk::Format::eD32SfloatS8Uint,
            vk::Format::eD24UnormS8Uint,
            vk::Format::eD16UnormS8Uint,
            vk::Format::eS8Uint,
        };
        return std::ranges::find(stencil_formats, format) != std::end(stencil_formats);
    }

    auto get_image_aspect_from_format(vk::Format format) -> vk::ImageAspectFlags
    {
        vk::ImageAspectFlags aspect{};

        if (is_depth_format(format) || is_stencil_format(format))
        {
            if (is_depth_format(format))
            {
                aspect |= vk::ImageAspectFlagBits::eDepth;
            }
            if (is_stencil_format(format))
            {
                aspect |= vk::ImageAspectFlagBits::eStencil;
            }
        }
        else
        {
            aspect |= vk::ImageAspectFlagBits::eColor;
        }

        return aspect;
    }

    auto get_format_byte_size(vk::Format format) -> u32
    {
        switch (format)
        {
            case vk::Format::eR8Srgb: return 1 * 1;
            case vk::Format::eR8G8Srgb: return 1 * 2;
            case vk::Format::eR8G8B8Srgb: return 1 * 3;
            case vk::Format::eR8G8B8A8Srgb: return 1 * 4;
            case vk::Format::eR32Sfloat: return 4 * 1;
            case vk::Format::eR32G32Sfloat: return 4 * 2;
            case vk::Format::eR32G32B32Sfloat: return 4 * 3;
            case vk::Format::eR32G32B32A32Sfloat: return 4 * 4;
            default: ASSERT(("Unknown vk::Format!", false)); break;
        }
        return {};
    }

}
