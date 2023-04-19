#pragma once

#include "mill/core/base.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    class DescriptorSetLayout
    {
    public:
        DescriptorSetLayout(vk::Device& device, vk::DescriptorSetLayoutCreateFlags flags = {});
        DescriptorSetLayout(const DescriptorSetLayout&) = delete;
        DescriptorSetLayout(DescriptorSetLayout&& other) noexcept;
        ~DescriptorSetLayout();

        void add_binding(u32 binding, vk::DescriptorType type, vk::ShaderStageFlags shader_stages);

        void add_uniform_buffer(u32 binding, vk::ShaderStageFlags shader_stages);
        void add_sampled_image(u32 binding, vk::ShaderStageFlags shader_stages);

        void build();

        /* Getters */

        auto get_device() const -> vk::Device;
        auto get_flags() const -> const vk::DescriptorSetLayoutCreateFlags&;
        auto get_hash() const -> hasht;
        auto get_layout() const -> const vk::DescriptorSetLayout&;

        auto get_bindings() const -> const std::vector<vk::DescriptorSetLayoutBinding>&;

        /* Operators */

        auto operator=(const DescriptorSetLayout&) -> DescriptorSetLayout& = delete;
        auto operator=(DescriptorSetLayout&& rhs) noexcept -> DescriptorSetLayout&;

    private:
        auto compute_hash() const -> hasht;

    private:
        vk::Device m_device{};
        vk::DescriptorSetLayoutCreateFlags m_createFlags{};

        hasht m_hash{};
        vk::UniqueDescriptorSetLayout m_layout{};

        std::vector<vk::DescriptorSetLayoutBinding> m_bindings{};
    };
}