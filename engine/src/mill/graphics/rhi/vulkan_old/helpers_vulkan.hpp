#pragma once

#include "mill/core/base.hpp"
#include "includes_vulkan.hpp"

#include <string_view>

namespace mill::rhi::vulkan
{
#pragma region Instance Helpers

    bool is_instance_ext_supported(const std::string_view& ext_name);

    bool is_instance_layer_supported(const std::string_view& layer_name);

#pragma endregion

#pragma region PhysicalDevice Helpers

    auto get_best_dedicated_gpu(vk::Instance instance) -> vk::PhysicalDevice;

#pragma endregion

#pragma region Device Helpers

    bool is_device_ext_supported(vk::PhysicalDevice physical_device, const std::string_view& ext_name);

    auto find_graphics_queue_family(vk::PhysicalDevice physical_device) -> i32;

    auto find_transfer_queue_family(vk::PhysicalDevice physical_device) -> i32;

    void set_object_name(vk::Device device, u64 object, vk::ObjectType type, const char* name);

#define SET_VK_OBJECT_NAME(_device, _typeClass, _object, _name) \
    ::mill::rhi::vulkan::set_object_name(_device, u64(static_cast<_typeClass>(_object)), _object.objectType, _name);

#define SET_VK_OBJECT_NAME_INDEXED(_device, _typeClass, _object, _name, _index) \
    ::mill::rhi::vulkan::set_object_name(                                       \
        _device, u64(static_cast<_typeClass>(_object)), _object.objectType, std::format("{} [{}]", _name, _index).c_str());

#pragma endregion

#pragma region Swapchain Helpers

    auto select_swapchain_present_mode(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface, bool vsync) -> vk::PresentModeKHR;

    auto select_swapchain_surface_format(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface) -> vk::SurfaceFormatKHR;

    auto select_swapchain_extent(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface, u32 width, u32 height) -> vk::Extent2D;

    auto select_swapchain_image_count(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface) -> u32;

#pragma endregion

#pragma region CreateInfo Helpers

    auto get_image_view_create_info_2d(vk::Image image, vk::Format format, u32 base_mip_lvl = 0, u32 mip_lvl_count = 1)
        -> vk::ImageViewCreateInfo;

    auto get_image_subresource_layers_2d(vk::Format format, u32 mip_lvl) -> vk::ImageSubresourceLayers;

    auto get_image_subresource_range_2d(vk::Format format, u32 base_mip_lvl = 0, u32 mip_lvl_count = 1) -> vk::ImageSubresourceRange;

#pragma endregion

#pragma region Pipeline Helpers

    class DescriptorSetLayout
    {
    public:
        explicit DescriptorSetLayout(vk::Device device);

        ~DescriptorSetLayout();

        void reset();

        void add_binding(u32 binding,
                         vk::DescriptorType descriptor_type,
                         vk::ShaderStageFlags shader_stages,
                         u32 count = 1,
                         vk::DescriptorBindingFlags binding_flags = {});

        void build();

        auto get_hash() const -> hasht;
        auto get_layout() const -> vk::DescriptorSetLayout;

    private:
        auto calculate_hash() const -> hasht;

    private:
        vk::Device m_device{};

        std::vector<vk::DescriptorSetLayoutBinding> m_bindings{};
        std::vector<vk::DescriptorBindingFlags> m_bindingFlags{};
        vk::DescriptorSetLayoutCreateInfo m_layoutInfo{};

        hasht m_hash{};
        vk::DescriptorSetLayout m_layout{};
    };

    class PipelineLayout
    {
    public:
        explicit PipelineLayout(vk::Device device);
        ~PipelineLayout();

        void reset();

        void add_push_constant_range(vk::ShaderStageFlags stages, u32 offset_bytes, u32 size_bytes);

        void add_descriptor_set_layout(u32 set, const DescriptorSetLayout& descriptor_layout);

        void build();

        auto get_hash() const -> u64;
        auto get_layout() const -> vk::PipelineLayout;

    private:
        auto calculate_hash() const -> hasht;

    private:
        vk::Device m_device{};

        std::vector<vk::PushConstantRange> m_pushConstantRanges{};
        std::vector<const DescriptorSetLayout*> m_descriptorSetLayouts{};
        vk::PipelineLayoutCreateInfo m_layoutInfo{};

        vk::PipelineLayout m_layout{};
    };

    class DescriptorSet
    {
    public:
        explicit DescriptorSet(vk::Device device, vk::DescriptorPool descriptor_pool, DescriptorSetLayout& layout);
        ~DescriptorSet();

        void bind_buffer(u32 binding, vk::Buffer buffer, u64 range_bytes);
        void bind_image(u32 binding, vk::ImageView image_view, vk::Sampler sampler, u32 array_index = 0);

        void flush_writes();

        auto get_set() const -> vk::DescriptorSet;

    private:
        vk::Device m_device{};
        vk::DescriptorPool m_pool{};
        DescriptorSetLayout& m_layout;

        vk::DescriptorSet m_set{};

        std::vector<vk::DescriptorBufferInfo> m_bufferInfos{};
        // #TODO: Try figure out (and fix) the reason that these (DescriptorImageInfo) become invalid if not Owned<>
        std::vector<Owned<vk::DescriptorImageInfo>> m_imageInfos{};
        std::vector<vk::WriteDescriptorSet> m_pendingWrites{};
    };

#pragma endregion

#pragma region Barriers

    auto get_barrier_image_to_transfer_src(vk::Image image, vk::Format format, vk::ImageLayout old_layout) -> vk::ImageMemoryBarrier2;
    auto get_barrier_image_to_transfer_dst(vk::Image image, vk::Format format, vk::ImageLayout old_layout) -> vk::ImageMemoryBarrier2;
    auto get_barrier_image_to_attachment(vk::Image image, vk::Format format, vk::ImageLayout old_layout) -> vk::ImageMemoryBarrier2;
    auto get_barrier_image_to_present(vk::Image image, vk::Format format, vk::ImageLayout old_layout) -> vk::ImageMemoryBarrier2;

#pragma endregion

#pragma region Miscellaneous

    bool is_depth_format(vk::Format format);

    bool is_stencil_format(vk::Format format);

    auto get_image_aspect_from_format(vk::Format format) -> vk::ImageAspectFlags;

#pragma endregion

}
