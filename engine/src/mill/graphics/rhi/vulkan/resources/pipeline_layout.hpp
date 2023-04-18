#pragma once

#include "mill/core/base.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    class DescriptorSetLayout;

    class PipelineLayout
    {
    public:
        PipelineLayout(vk::Device& device, vk::PipelineLayoutCreateFlags flags = {});
        PipelineLayout(const PipelineLayout&) = delete;
        PipelineLayout(PipelineLayout&& other) noexcept;
        ~PipelineLayout();

        void add_set_layout(u32 set, DescriptorSetLayout& set_layout);
        void add_push_constant_buffer(u32 offset, u32 size, vk::ShaderStageFlags shader_stages);

        void build();

        /* Getters */

        auto get_device() const -> vk::Device;
        auto get_flags() const -> const vk::PipelineLayoutCreateFlags&;
        auto get_hash() const -> hasht;
        auto get_layout() const -> const vk::PipelineLayout&;

        /* Static */

        static auto merge_layouts(const PipelineLayout& a, const PipelineLayout& b) -> PipelineLayout;

        /* Operators */

        auto operator=(const PipelineLayout&) -> PipelineLayout& = delete;
        auto operator=(PipelineLayout&& rhs) noexcept -> PipelineLayout&;

    private:
        auto compute_hash() const -> hasht;

    private:
        vk::Device m_device{};
        vk::PipelineLayoutCreateFlags m_createFlags{};

        hasht m_hash{};
        vk::UniquePipelineLayout m_layout{};

        std::vector<DescriptorSetLayout*> m_sets{};
        std::vector<vk::PushConstantRange> m_pushConstantRanges{};
    };
}