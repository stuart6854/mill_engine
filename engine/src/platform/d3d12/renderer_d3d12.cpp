#include "renderer_d3d12.hpp"

#include "mill/core/debug.hpp"
#include "d3d12_common_headers.hpp"
#include "d3d12_core.hpp"

#include <vector>

namespace mill::platform
{
    void RendererD3D12::inititialise(const RendererInit& init)
    {
        m_device = CreateOwned<DeviceD3D12>();
        if (!m_device->inititialise())
        {
            LOG_ERROR("RendererD3D12 - Failed to initialise device!");
            return;
        }

        m_graphicsContext = m_device->create_context();

        m_hwnd = static_cast<HWND>(init.window_handle);
        m_screenSize = { init.window_width, init.window_height };
        if (!m_device->add_surface(m_hwnd, m_screenSize.x, m_screenSize.y))
        {
            LOG_ERROR("RendererD3D12 - Failed to add surface to device!");
            return;
        }

        {
            CD3DX12_ROOT_SIGNATURE_DESC root_signature_desc{};
            root_signature_desc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

            ComPtr<ID3DBlob> signature{ nullptr };
            ComPtr<ID3DBlob> error{ nullptr };
            assert_if_failed(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
            assert_if_failed(m_device->get_device()->CreateRootSignature(
                0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
        }
        {
            ComPtr<ID3DBlob> vertexShader{ nullptr };
            ComPtr<ID3DBlob> fragmentShader{ nullptr };

            u32 compile_flags = 0;
#if MILL_DEBUG
            compile_flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

            assert_if_failed(D3DCompileFromFile(
                L"../../assets/shaders/default.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compile_flags, 0, &vertexShader, nullptr));
            assert_if_failed(D3DCompileFromFile(
                L"../../assets/shaders/default.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compile_flags, 0, &fragmentShader, nullptr));

            D3D12_INPUT_ELEMENT_DESC input_element_descs[] = {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            };

            D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc{};
            // pso_desc.InputLayout = { input_element_descs, _countof(input_element_descs) };
            pso_desc.pRootSignature = m_rootSignature.Get();
            pso_desc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
            pso_desc.PS = CD3DX12_SHADER_BYTECODE(fragmentShader.Get());
            pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
            pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            pso_desc.DepthStencilState.DepthEnable = FALSE;
            pso_desc.DepthStencilState.StencilEnable = FALSE;
            pso_desc.SampleMask = UINT_MAX;
            pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            pso_desc.NumRenderTargets = 1;
            pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            pso_desc.SampleDesc.Count = 1;
            assert_if_failed(m_device->get_device()->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&m_pipelineState)));
        }
    }

    void RendererD3D12::shutdown()
    {
        m_device->wait_for_idle();

        m_pipelineState = nullptr;
        m_rootSignature = nullptr;

        m_device->destroy_context(std::move(m_graphicsContext));

        if (!m_device->shutdown())
        {
            LOG_ERROR("RendererD3D12 - Failed to shutdown device!");
        }
        m_device = nullptr;
    }

    void RendererD3D12::render(const SceneInfo& /*scene_info*/)
    {
        m_device->begin_frame();

        m_graphicsContext->reset();

        auto& backbuffer = *m_device->get_current_back_buffer(m_hwnd);

        m_graphicsContext->add_barrier(backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_graphicsContext->clear_render_target(backbuffer, { 0.36f, 0.54f, 0.86f, 1.0f });

        m_graphicsContext->set_default_viewport_and_scissor(m_screenSize);
        m_graphicsContext->set_render_target(backbuffer);
        m_graphicsContext->set_primitive_topology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_graphicsContext->set_pipeline(m_pipelineState.Get(), m_rootSignature.Get());
        m_graphicsContext->draw(3, 0);

        m_graphicsContext->add_barrier(backbuffer, D3D12_RESOURCE_STATE_PRESENT);
        m_device->submit_context(*m_graphicsContext);

        m_device->end_frame();
        m_device->present();
    }

    auto create_renderer() -> Owned<RendererInterface>
    {
        return CreateOwned<RendererD3D12>();
    }

}
