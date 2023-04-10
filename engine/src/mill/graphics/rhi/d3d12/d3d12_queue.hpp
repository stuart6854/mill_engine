#pragma once

#include "mill/core/base.hpp"
#include "d3d12_common_headers.hpp"

namespace mill::platform
{
    class DeviceD3D12;

    class QueueD3D12
    {
    public:
        QueueD3D12(DeviceD3D12& device, D3D12_COMMAND_LIST_TYPE cmd_type);
        ~QueueD3D12();

        DISABLE_COPY(QueueD3D12);

        /* Commands */

        /* Execute a command list and get its fence signal value. */
        auto execute_command_list(ID3D12CommandList* cmd_list) -> u64;

        /* Signal the fence and return the signal value. */
        auto signal_fence() -> u64;

        void wait_for_signal(u64 signal_value);
        void wait_for_idle();

        /* Getters */

        auto get_queue() const -> ID3D12CommandQueue*;

    private:
        DeviceD3D12& m_device;
        ComPtr<ID3D12CommandQueue> m_queue{ nullptr };
        ComPtr<ID3D12Fence> m_fence{ nullptr };
        HANDLE m_fenceEvent{ nullptr };
        u64 m_nextFenceValue{};
    };
}