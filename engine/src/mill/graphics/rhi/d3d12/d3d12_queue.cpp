#include "d3d12_queue.hpp"

#include "mill/core/debug.hpp"
#include "d3d12_core.hpp"
#include "d3d12_device.hpp"

namespace mill::platform
{
    QueueD3D12::QueueD3D12(DeviceD3D12& device, D3D12_COMMAND_LIST_TYPE cmd_type) : m_device(device)
    {
        D3D12_COMMAND_QUEUE_DESC queue_desc{};
        queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queue_desc.Type = cmd_type;
        assert_if_failed(m_device.get_device()->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&m_queue)));

        assert_if_failed(m_device.get_device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

        m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);

        m_nextFenceValue = 0;
    }

    QueueD3D12::~QueueD3D12()
    {
        wait_for_idle();

        m_queue = nullptr;
        m_fence = nullptr;
        CloseHandle(m_fenceEvent);
    }

    auto QueueD3D12::execute_command_list(ID3D12CommandList* cmd_list) -> u64
    {
        m_queue->ExecuteCommandLists(1, &cmd_list);

        const auto signal_value = signal_fence();
        return signal_value;
    }

    auto QueueD3D12::signal_fence() -> u64
    {
        const auto signal_value = m_nextFenceValue;
        ++m_nextFenceValue;
        m_queue->Signal(m_fence.Get(), signal_value);
        return signal_value;
    }

    /* Wait for the queue to signal the given fence value. This call is CPU blocking. */
    void QueueD3D12::wait_for_signal(u64 signal_value)
    {
        if (m_fence->GetCompletedValue() < signal_value)
        {
            assert_if_failed(m_fence->SetEventOnCompletion(signal_value, m_fenceEvent));
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }
    }

    /* Add a Signal to the end of the Queue and wait for the Queue to reach it. This call is CPU blocking. */
    void QueueD3D12::wait_for_idle()
    {
        const auto signal_value = signal_fence();
        wait_for_signal(signal_value);
    }

    auto QueueD3D12::get_queue() const -> ID3D12CommandQueue*
    {
        return m_queue.Get();
    }

}
