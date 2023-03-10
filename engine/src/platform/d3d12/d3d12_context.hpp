#pragma once

#include "mill/core/base.hpp"
#include "d3d12_common_headers.hpp"
#include "d3d12_resources.hpp"

#include <glm/ext/vector_float4.hpp>

namespace mill::platform
{
    class DeviceD3D12;

    class ContextD3D12
    {
    public:
        ContextD3D12(DeviceD3D12& device, D3D12_COMMAND_LIST_TYPE command_type);
        ~ContextD3D12();

        DISABLE_COPY(ContextD3D12);

        void reset();

        /* Common Operations */

        void add_barrier(ResourceD3D12& resource, D3D12_RESOURCE_STATES new_state);

        /* Graphics Operations */

        void clear_render_target(const TextureResourceD3D12& target, const glm::vec4& color);

        /* Getters */

        auto get_cmd_type() const -> D3D12_COMMAND_LIST_TYPE;
        auto get_cmd_list() const -> ID3D12GraphicsCommandList*;

    private:
        class DeviceD3D12& m_device;
        D3D12_COMMAND_LIST_TYPE m_cmdType{};
        ComPtr<ID3D12GraphicsCommandList> m_cmdList{ nullptr };
        std::array<ComPtr<ID3D12CommandAllocator>, g_FrameBufferCount> m_cmdAllocators{};
    };
}