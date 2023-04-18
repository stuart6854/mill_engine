#include "pipeline_layout.hpp"

#include "mill/core/base.hpp"
#include "descriptor_set_layout.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    PipelineLayout::PipelineLayout(vk::Device& device, vk::PipelineLayoutCreateFlags flags) : m_device(device), m_createFlags(flags) {}

    PipelineLayout::PipelineLayout(PipelineLayout&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_createFlags, other.m_createFlags);
        std::swap(m_hash, other.m_hash);
        std::swap(m_layout, other.m_layout);
        std::swap(m_setLayouts, other.m_setLayouts);
        std::swap(m_pushConstantRanges, other.m_pushConstantRanges);
    }

    PipelineLayout::~PipelineLayout() = default;

    void PipelineLayout::add_set_layout(u32 set, Shared<DescriptorSetLayout> set_layout)
    {
        if (m_setLayouts.size() < CAST_U64(set + 1))
            m_setLayouts.resize(CAST_U64(set + 1));

        m_setLayouts[set] = set_layout;
    }

    void PipelineLayout::add_push_constant_buffer(u32 offset, u32 size, vk::ShaderStageFlags shader_stages)
    {
        auto& const_block = m_pushConstantRanges.emplace_back();
        const_block.setOffset(offset);
        const_block.setSize(size);
        const_block.setStageFlags(shader_stages);
    }

    void PipelineLayout::build()
    {
        std::vector<vk::DescriptorSetLayout> set_layouts(m_setLayouts.size());
        for (u32 i = 0; i < m_setLayouts.size(); ++i)
        {
            if (m_setLayouts.at(i) != nullptr)
                set_layouts.at(i) = m_setLayouts.at(i)->get_layout();
        }

        vk::PipelineLayoutCreateInfo layout_info{};
        layout_info.setFlags(m_createFlags);
        layout_info.setSetLayouts(set_layouts);
        layout_info.setPushConstantRanges(m_pushConstantRanges);

        m_layout = m_device.createPipelineLayoutUnique(layout_info);

        m_hash = compute_hash();
    }

    auto PipelineLayout::get_device() const -> vk::Device
    {
        return m_device;
    }

    auto PipelineLayout::get_flags() const -> const vk::PipelineLayoutCreateFlags&
    {
        return m_createFlags;
    }

    auto PipelineLayout::get_hash() const -> hasht
    {
        if (m_hash)
            return m_hash;

        return compute_hash();
    }

    auto PipelineLayout::get_layout() const -> const vk::PipelineLayout&
    {
        return m_layout.get();
    }

    auto PipelineLayout::merge_layouts(const PipelineLayout& a, const PipelineLayout& b) -> PipelineLayout
    {
        vk::Device device = a.get_device();
        PipelineLayout new_layout(device, a.get_flags() | b.get_flags());
        // #TODO
        return new_layout;
    }

    auto PipelineLayout::operator=(PipelineLayout&& rhs) noexcept -> PipelineLayout&
    {
        std::swap(m_device, rhs.m_device);
        std::swap(m_createFlags, rhs.m_createFlags);
        std::swap(m_hash, rhs.m_hash);
        std::swap(m_layout, rhs.m_layout);
        std::swap(m_setLayouts, rhs.m_setLayouts);
        std::swap(m_pushConstantRanges, rhs.m_pushConstantRanges);
        return *this;
    }

    auto PipelineLayout::compute_hash() const -> hasht
    {
        hasht hash{};

        hash_combine(hash, m_createFlags);

        hash_combine(hash, m_setLayouts.size());
        for (const auto& set : m_setLayouts)
            hash_combine(hash, set->get_hash());

        hash_combine(hash, m_pushConstantRanges.size());
        for (const auto& const_range : m_pushConstantRanges)
        {
            hash_combine(hash, const_range.offset);
            hash_combine(hash, const_range.size);
            hash_combine(hash, const_range.stageFlags);
        }

        return hash;
    }
}
