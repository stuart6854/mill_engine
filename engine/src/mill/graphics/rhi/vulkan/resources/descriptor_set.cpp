#include "descriptor_set.hpp"

#include "../rhi_core_vulkan.hpp"
#include "descriptor_set_layout.hpp"
#include "buffer.hpp"
#include "sampler.hpp"
#include "../vulkan_image.hpp"
#include "../vulkan_device.hpp"

namespace mill::rhi
{
    DescriptorSet::DescriptorSet(DeviceVulkan& device, const Shared<DescriptorSetLayout>& layout, bool buffered)
        : m_device(device), m_layout(layout), m_buffered(buffered)
    {
        auto& descriptor_pool = m_device.get_descriptor_pool();

        const auto set_count = m_buffered ? g_FrameBufferCount : 1;
        const std::vector set_layouts(set_count, m_layout->get_layout());

        vk::DescriptorSetAllocateInfo alloc_info{};
        alloc_info.setDescriptorPool(descriptor_pool);
        alloc_info.setDescriptorSetCount(set_count);
        alloc_info.setSetLayouts(set_layouts);
        auto sets = device.get_device().allocateDescriptorSetsUnique(alloc_info);
        ASSERT(sets.size() == set_count);

        m_frames.resize(set_count);
        for (u32 i = 0; i < set_count; ++i)
        {
            m_frames[i].set = std::move(sets[i]);
        }
    }

    void DescriptorSet::set_uniform_buffer(u32 binding, const Buffer& buffer)
    {
        vk::WriteDescriptorSet write{};
        write.setDstBinding(binding);
        write.setDescriptorCount(1);
        write.setDescriptorType(vk::DescriptorType::eUniformBuffer);

        for (auto& frame : m_frames)
        {
            auto& buffer_info = frame.bufferInfos.emplace_back();
            buffer_info.setBuffer(buffer.get_buffer());
            buffer_info.setOffset(0);
            buffer_info.setRange(buffer.get_size());

            write.setDstSet(frame.set.get());
            write.setBufferInfo(buffer_info);

            frame.pendingWrites.push_back(write);
        }
    }

    void DescriptorSet::set_image(u32 binding, const ImageVulkan& image)
    {
        vk::WriteDescriptorSet write{};
        write.setDstBinding(binding);
        write.setDescriptorCount(1);
        write.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);

        for (auto& frame : m_frames)
        {
            auto& image_info = frame.imageInfos.emplace_back();
            image_info.setImageView(image.get_view());
            image_info.setImageLayout(image.get_layout());

            ASSERT(image.get_sampler());
            image_info.setSampler(image.get_sampler()->get_sampler());

            write.setDstSet(frame.set.get());
            write.setImageInfo(image_info);

            frame.pendingWrites.push_back(write);
        }
    }

    void DescriptorSet::next_frame()
    {
        m_frameIndex = (m_frameIndex + 1) % m_frames.size();

        auto& frame = m_frames.at(m_frameIndex);
        m_device.get_device().updateDescriptorSets(frame.pendingWrites, {});

        frame.bufferInfos.clear();
        frame.pendingWrites.clear();
    }

    auto DescriptorSet::get_set() -> vk::DescriptorSet
    {
        return m_frames.at(m_frameIndex).set.get();
    }

}
