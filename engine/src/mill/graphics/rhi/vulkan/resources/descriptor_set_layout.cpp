#include "descriptor_set_layout.hpp"

#include "mill/core/base.hpp"
#include "../vulkan_includes.hpp"

namespace mill::rhi
{
    DescriptorSetLayout::DescriptorSetLayout(vk::Device& device, vk::DescriptorSetLayoutCreateFlags flags)
        : m_device(device), m_createFlags(flags)
    {
    }

    DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) noexcept
    {
        std::swap(m_device, other.m_device);
        std::swap(m_createFlags, other.m_createFlags);
        std::swap(m_hash, other.m_hash);
        std::swap(m_layout, other.m_layout);
        std::swap(m_bindings, other.m_bindings);
    }

    DescriptorSetLayout::~DescriptorSetLayout() = default;

    void DescriptorSetLayout::add_binding(u32 binding, vk::DescriptorType type, vk::ShaderStageFlags shader_stages)
    {
        auto& layout_binding = m_bindings.emplace_back();
        layout_binding.setBinding(binding);
        layout_binding.setDescriptorCount(1);
        layout_binding.setDescriptorType(type);
        layout_binding.setStageFlags(shader_stages);
    }

    void DescriptorSetLayout::add_uniform_buffer(u32 binding)
    {
        add_binding(binding, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAll);
    }

    void DescriptorSetLayout::add_sampled_image(u32 binding)
    {
        add_binding(binding, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eAll);
    }

    void DescriptorSetLayout::build()
    {
        vk::DescriptorSetLayoutCreateInfo layout_info{};
        layout_info.setFlags(m_createFlags);
        layout_info.setBindings(m_bindings);

        m_layout = m_device.createDescriptorSetLayoutUnique(layout_info);

        m_hash = compute_hash();
    }

    auto DescriptorSetLayout::get_device() const -> vk::Device
    {
        return m_device;
    }

    auto DescriptorSetLayout::get_flags() const -> const vk::DescriptorSetLayoutCreateFlags&
    {
        return m_createFlags;
    }

    auto DescriptorSetLayout::get_hash() const -> hasht
    {
        if (m_hash)
            return m_hash;

        return compute_hash();
    }

    auto DescriptorSetLayout::get_layout() const -> const vk::DescriptorSetLayout&
    {
        return m_layout.get();
    }

    auto DescriptorSetLayout::get_bindings() const -> const std::vector<vk::DescriptorSetLayoutBinding>&
    {
        return m_bindings;
    }

    auto DescriptorSetLayout::operator=(DescriptorSetLayout&& rhs) noexcept -> DescriptorSetLayout&
    {
        std::swap(m_device, rhs.m_device);
        std::swap(m_createFlags, rhs.m_createFlags);
        std::swap(m_hash, rhs.m_hash);
        std::swap(m_layout, rhs.m_layout);
        std::swap(m_bindings, rhs.m_bindings);

        return *this;
    }

    auto DescriptorSetLayout::compute_hash() const -> hasht
    {
        hasht hash{};

        hash_combine(hash, m_createFlags);

        hash_combine(hash, m_bindings.size());
        for (const auto& binding : m_bindings)
        {
            hash_combine(hash, binding.binding);
            hash_combine(hash, binding.descriptorCount);
            hash_combine(hash, binding.descriptorType);
            hash_combine(hash, binding.stageFlags);
        }

        return hash;
    }

}
