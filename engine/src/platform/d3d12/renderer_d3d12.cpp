#include "renderer_d3d12.hpp"

#include "mill/core/debug.hpp"
#include "d3d12_common_headers.hpp"
#include "d3d12_core.hpp"

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

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
            CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
            CD3DX12_ROOT_PARAMETER1 root_params[1];

            ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
            root_params[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);

            CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC root_signature_desc{};
            root_signature_desc.Init_1_1(
                _countof(root_params), root_params, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

            ComPtr<ID3DBlob> signature{ nullptr };
            ComPtr<ID3DBlob> error{ nullptr };
            assert_if_failed(
                D3DX12SerializeVersionedRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error));
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
            pso_desc.InputLayout = { input_element_descs, _countof(input_element_descs) };
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

        const f32 aspect_ratio = static_cast<f32>(m_screenSize.x) / static_cast<f32>(m_screenSize.y);
        {
            /* Vertex Buffer */
            struct Vertex
            {
                glm::vec3 position;
                glm::vec4 color;
            };

            std::vector<Vertex> vertices{
                { { 0.0f, 0.5f * aspect_ratio, 0.0f }, { 1.0f, 0.0f, 0.0f, 0.0f } },
                { { 0.5f, -0.5f * aspect_ratio, 0.0f }, { 0.0f, 1.0f, 0.0f, 0.0f } },
                { { -0.5f, -0.5f * aspect_ratio, 0.0f }, { 0.0f, 0.0f, 1.0f, 0.0f } },
            };

            const auto vertex_buffer_size = static_cast<u32>(sizeof(Vertex) * vertices.size());

            CD3DX12_HEAP_PROPERTIES heap_props(D3D12_HEAP_TYPE_UPLOAD);
            auto resource_desc = CD3DX12_RESOURCE_DESC::Buffer(vertex_buffer_size);
            assert_if_failed(m_device->get_device()->CreateCommittedResource(&heap_props,
                                                                             D3D12_HEAP_FLAG_NONE,
                                                                             &resource_desc,
                                                                             D3D12_RESOURCE_STATE_GENERIC_READ,
                                                                             nullptr,
                                                                             IID_PPV_ARGS(&m_vertexBuffer)));

            u8* mappedData{ nullptr };
            CD3DX12_RANGE read_range(0, 0);
            assert_if_failed(m_vertexBuffer->Map(0, &read_range, reinterpret_cast<void**>(&mappedData)));
            std::memcpy(mappedData, vertices.data(), vertex_buffer_size);
            m_vertexBuffer->Unmap(0, nullptr);

            m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
            m_vertexBufferView.StrideInBytes = sizeof(Vertex);
            m_vertexBufferView.SizeInBytes = vertex_buffer_size;
        }
        {
            /* Index Buffer */
            std::vector<u16> indices{ 0, 1, 2 };

            const auto index_buffer_size = static_cast<u32>(sizeof(u16) * indices.size());

            auto heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            auto resource_desc = CD3DX12_RESOURCE_DESC::Buffer(index_buffer_size);
            assert_if_failed(m_device->get_device()->CreateCommittedResource(&heap_props,
                                                                             D3D12_HEAP_FLAG_NONE,
                                                                             &resource_desc,
                                                                             D3D12_RESOURCE_STATE_GENERIC_READ,
                                                                             nullptr,
                                                                             IID_PPV_ARGS(&m_indexBuffer)));

            u8* mappedData{ nullptr };
            auto read_range = CD3DX12_RANGE(0, 0);
            assert_if_failed(m_indexBuffer->Map(0, &read_range, reinterpret_cast<void**>(&mappedData)));
            std::memcpy(mappedData, indices.data(), index_buffer_size);
            m_indexBuffer->Unmap(0, nullptr);

            m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
            m_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
            m_indexBufferView.SizeInBytes = index_buffer_size;
        }
        {
            D3D12_DESCRIPTOR_HEAP_DESC heap_desc{};
            heap_desc.NumDescriptors = 1;
            heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            assert_if_failed(m_device->get_device()->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&m_cbvHeap)));

            /* Constant (Uniform) Buffer */
            std::vector uniforms = {
                glm::perspectiveLH_ZO(glm::radians(60.0f), aspect_ratio, 0.1f, 10.0f),
                // glm::mat4(1.0f),
                glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -5))),
            };

            const auto buffer_size = static_cast<u32>(sizeof(glm::mat4) * uniforms.size());

            auto heap_props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
            auto resource_desc = CD3DX12_RESOURCE_DESC::Buffer(AlignU32(buffer_size, 256));
            assert_if_failed(m_device->get_device()->CreateCommittedResource(&heap_props,
                                                                             D3D12_HEAP_FLAG_NONE,
                                                                             &resource_desc,
                                                                             D3D12_RESOURCE_STATE_GENERIC_READ,
                                                                             nullptr,
                                                                             IID_PPV_ARGS(&m_constantBuffer)));

            u8* mappedData{ nullptr };
            auto read_range = CD3DX12_RANGE(0, 0);
            assert_if_failed(m_constantBuffer->Map(0, &read_range, reinterpret_cast<void**>(&mappedData)));
            std::memcpy(mappedData, uniforms.data(), buffer_size);
            m_constantBuffer->Unmap(0, nullptr);

            D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc{};
            cbv_desc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
            cbv_desc.SizeInBytes = AlignU32(buffer_size, 256);
            m_device->get_device()->CreateConstantBufferView(&cbv_desc, m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
        }
    }

    void RendererD3D12::shutdown()
    {
        m_device->wait_for_idle();

        m_constantBuffer = nullptr;
        m_vertexBuffer = nullptr;
        m_indexBuffer = nullptr;

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

        ID3D12DescriptorHeap* heaps[] = { m_cbvHeap.Get() };
        m_graphicsContext->get_cmd_list()->SetDescriptorHeaps(_countof(heaps), heaps);
        m_graphicsContext->get_cmd_list()->SetGraphicsRootDescriptorTable(0, m_cbvHeap->GetGPUDescriptorHandleForHeapStart());

        m_graphicsContext->get_cmd_list()->IASetVertexBuffers(0, 1, &m_vertexBufferView);
        m_graphicsContext->get_cmd_list()->IASetIndexBuffer(&m_indexBufferView);
        m_graphicsContext->draw_indexed(3, 0, 0);

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
