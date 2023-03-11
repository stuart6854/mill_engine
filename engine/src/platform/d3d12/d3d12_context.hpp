#pragma once

#include "mill/core/base.hpp"
#include "d3d12_common_headers.hpp"
#include "d3d12_resources.hpp"

#include <glm/ext/vector_uint2.hpp>
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

        void set_default_viewport_and_scissor(const glm::uvec2& screen_size);
        void set_viewport(const D3D12_VIEWPORT& viewport);
        void set_scissor(const D3D12_RECT& rect);
        void set_render_target(const TextureResourceD3D12& target);
        void set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY topology);
        void set_pipeline(ID3D12PipelineState* pipeline, ID3D12RootSignature* root_signature);

        void draw(u32 vertex_count, u32 vertex_offset);
        void draw_indexed(u32 index_count, u32 index_offset, u32 vertex_offset);

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