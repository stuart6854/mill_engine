#include "d3d12_context.hpp"

#include "mill/core/debug.hpp"
#include "d3d12_core.hpp"
#include "d3d12_device.hpp"
#include "d3d12_resources.hpp"

namespace mill::platform
{
    ContextD3D12::ContextD3D12(DeviceD3D12& device, D3D12_COMMAND_LIST_TYPE command_type) : m_device(device), m_cmdType(command_type)
    {
        for (auto& allocator : m_cmdAllocators)
        {
            assert_if_failed(device.get_device()->CreateCommandAllocator(m_cmdType, IID_PPV_ARGS(&allocator)));
        }
        assert_if_failed(
            m_device.get_device()->CreateCommandList(0, m_cmdType, m_cmdAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_cmdList)));

        // Command Lists are created in the recording state, but there is nothing
        // to record yet, so close it.
        m_cmdList->Close();
    }

    ContextD3D12::~ContextD3D12()
    {
        m_cmdList = nullptr;
        for (auto& allocator : m_cmdAllocators)
        {
            allocator = nullptr;
        }
    }

    void ContextD3D12::reset()
    {
        const auto frameIndex = m_device.get_frame_index();
        m_cmdAllocators[frameIndex]->Reset();
        m_cmdList->Reset(m_cmdAllocators[frameIndex].Get(), nullptr);
    }

    void ContextD3D12::add_barrier(ResourceD3D12& resource, D3D12_RESOURCE_STATES new_state)
    {
        const auto old_state = resource.state;

        if (old_state != new_state)
        {
            D3D12_RESOURCE_BARRIER barrier{};
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource = resource.resource;
            barrier.Transition.StateBefore = old_state;
            barrier.Transition.StateAfter = new_state;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

            m_cmdList->ResourceBarrier(1, &barrier);

            resource.state = new_state;
        }
    }

    void ContextD3D12::clear_render_target(const TextureResourceD3D12& target, const glm::vec4& color)
    {
        m_cmdList->ClearRenderTargetView(target.rtvDescriptor.cpuHandle, &color.x, 0, nullptr);
    }

    auto ContextD3D12::get_cmd_type() const -> D3D12_COMMAND_LIST_TYPE
    {
        return m_cmdType;
    }

    auto ContextD3D12::get_cmd_list() const -> ID3D12GraphicsCommandList*
    {
        return m_cmdList.Get();
    }

}
