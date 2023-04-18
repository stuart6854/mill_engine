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

        void add_uniform_buffer(u32 binding);
        void add_sampled_image(u32 binding);

        void build();

        /* Getters */

        auto get_device() const -> vk::Device;
        auto get_flags() const -> const vk::DescriptorSetLayoutCreateFlags&;
        auto get_hash() const -> hasht;
        auto get_layout() const -> const vk::DescriptorSetLayout&;

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