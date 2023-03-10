#include "renderer_d3d12.hpp"

#include "mill/core/debug.hpp"
#include "d3d12_common_headers.hpp"
#include "d3d12_core.hpp"

#include <vector>

namespace mill::platform
{
#if 0
    constexpr D3D_FEATURE_LEVEL minimum_feature_level = D3D_FEATURE_LEVEL_11_0;

    namespace
    {
        /*
            Get the first most performant adapter that supports the minimum feature level.
            #NOTE: This function can be expanded in functionality by, for example, checking if any output devices (i.e. screens) are
            attached, enumerate the supported resolutions, provide a means for the user to choose which adapter to use in a
            multi-adapter setting, etc.
        */
        auto determine_adapter(IDXGIFactory7* factory) -> IDXGIAdapter4*
        {
            IDXGIAdapter4* adapter = nullptr;

            // Get adapters in descending order of performance
            for (u32 i = 0; factory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)) !=
                            DXGI_ERROR_NOT_FOUND;
                 ++i)
            {
                // Pick the first adapter that suppoerts the minimum feature level
                if (SUCCEEDED(D3D12CreateDevice(adapter, minimum_feature_level, __uuidof(ID3D12Device), nullptr)))
                {
                    return adapter;
                }

                safe_release(adapter);
            }

            return nullptr;
        }

        auto get_max_feature_level(IDXGIAdapter4* adapter)
        {
            constexpr D3D_FEATURE_LEVEL feature_levels[]{
                D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_12_0, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_2,
            };

            D3D12_FEATURE_DATA_FEATURE_LEVELS feature_levels_info{};
            feature_levels_info.NumFeatureLevels = _countof(feature_levels);
            feature_levels_info.pFeatureLevelsRequested = feature_levels;

            ComPtr<ID3D12Device> device = nullptr;
            D3D12CreateDevice(adapter, minimum_feature_level, IID_PPV_ARGS(&device));
            device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &feature_levels_info, sizeof(feature_levels_info));
            return feature_levels_info.MaxSupportedFeatureLevel;
        }
    }

    void d3d12_debug_callback(
        D3D12_MESSAGE_CATEGORY /*Category*/, D3D12_MESSAGE_SEVERITY Severity, D3D12_MESSAGE_ID ID, LPCSTR pDescription, void* /*pContext*/)
    {
        if (Severity == D3D12_MESSAGE_SEVERITY_ERROR || Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION)
        {
            LOG_ERROR("D3D12 - [{}] {}", ID, pDescription);
        }
        else if (Severity == D3D12_MESSAGE_SEVERITY_WARNING)
        {
            LOG_WARN("D3D12 - [{}] {}", ID, pDescription);
        }
        else
        {
            LOG_DEBUG("D3D12 - [{}] {}", ID, pDescription);
        }
    }
#endif

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
        if (!m_device->add_surface(m_hwnd, init.window_width, init.window_height))
        {
            LOG_ERROR("RendererD3D12 - Failed to add surface to device!");
            return;
        }

#if 0
        // ASSERT(m_device == nullptr);

        HRESULT hr{ S_OK };

        u32 dxgi_factory_flags{ 0 };
    #ifdef MILL_DEBUG
        // Enable debugging layer. Requires "Graphics Tools" optional feature
        {
            ComPtr<ID3D12Debug6> debug_interface = nullptr;
            hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface));
            if (FAILED(hr))
            {
                LOG_ERROR("RendererDX12 - Failed to get debug interface!");
            }
            debug_interface->EnableDebugLayer();
            // debug_interface->SetEnableGPUBasedValidation();
        }

        // Enable debug errors/warnings
        dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
    #endif

        hr = CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&m_factory));
        if (FAILED(hr))
        {
            LOG_ERROR("RendererDX12 - Failed to create factory!");
            return;
        }

        // Determine which adapter (graphics card) to use, if any
        ComPtr<IDXGIAdapter4> adapter = nullptr;
        adapter.Attach(determine_adapter(m_factory));
        if (!adapter)
        {
            LOG_ERROR("RendererDX12 - Failed to determine an adapter to use!");
            return;
        }

        // Determine what is the maximum feature level is supported
        D3D_FEATURE_LEVEL max_feature_level = get_max_feature_level(adapter.Get());
        ASSERT(max_feature_level >= minimum_feature_level);

        // Create a ID3D12Device (virtual adapter)
        hr = D3D12CreateDevice(adapter.Get(), max_feature_level, IID_PPV_ARGS(&m_device));
        if (FAILED(hr))
        {
            LOG_ERROR("RendererDX12 - Failed to create device!");
            return;
        }
        NAME_D3D12_OBJECT(m_device, L"MAIN DEVICE (D3D12)");

    #ifdef MILL_DEBUG
        {
            ComPtr<ID3D12InfoQueue1> info_queue = nullptr;
            m_device->QueryInterface(IID_PPV_ARGS(&info_queue));
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

            assert_if_failed(info_queue->RegisterMessageCallback(
                &d3d12_debug_callback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &m_debugCallbackCookie));
        }
    #endif

        // m_cmd = CreateOwned<CommandD3D12>(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);

        D3D12_COMMAND_QUEUE_DESC cmd_queue_desc{};
        cmd_queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        cmd_queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        assert_if_failed(m_device->CreateCommandQueue(&cmd_queue_desc, IID_PPV_ARGS(&m_cmdQueue)));

        assert_if_failed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_cmdAllocator)));

        assert_if_failed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_cmdAllocator, nullptr, IID_PPV_ARGS(&m_cmdList)));
        assert_if_failed(m_cmdList->Close());

        DXGI_SWAP_CHAIN_DESC1 swapchain_desc{};
        swapchain_desc.BufferCount = FrameCount;
        swapchain_desc.Width = init.window_width;
        swapchain_desc.Height = init.window_height;
        swapchain_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapchain_desc.SampleDesc.Count = 1;
        ComPtr<IDXGISwapChain1> swapchain = nullptr;

        auto hwnd = static_cast<HWND>(init.window_handle);
        hr = m_factory->CreateSwapChainForHwnd(m_cmdQueue, hwnd, &swapchain_desc, nullptr, nullptr, &swapchain);
        if (FAILED(hr))
        {
            LOG_ERROR("Failed!");
        }

        assert_if_failed(swapchain.As(&m_swapchain));
        m_backbufferIndex = m_swapchain->GetCurrentBackBufferIndex();

        {
            D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc{};
            rtv_heap_desc.NumDescriptors = FrameCount;
            rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            assert_if_failed(m_device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&m_rtvHeap)));

            m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }

        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtv_handle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
            for (u32 i = 0; i < FrameCount; ++i)
            {
                assert_if_failed(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i])));
                m_device->CreateRenderTargetView(m_renderTargets[i], nullptr, rtv_handle);
                rtv_handle.Offset(1, m_rtvDescriptorSize);
            }
        }

        {
            assert_if_failed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
            m_fenceValue = 0;

            m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
            ASSERT(m_fenceEvent != nullptr);
        }
#endif
    }

    void RendererD3D12::shutdown()
    {
        m_device->wait_for_idle();

        m_device->destroy_context(std::move(m_graphicsContext));

        if (!m_device->shutdown())
        {
            LOG_ERROR("RendererD3D12 - Failed to shutdown device!");
        }
        m_device = nullptr;

#if 0
        // m_cmd->release();
        // m_cmd = nullptr;

        /* Wait for previous frame */

        // Signal and increment fence value
        const u64 fence_value = m_fenceValue;
        assert_if_failed(m_cmdQueue->Signal(m_fence, fence_value));
        ++m_fenceValue;

        // Wait until the previous frame is finished
        if (m_fence->GetCompletedValue() < fence_value)
        {
            assert_if_failed(m_fence->SetEventOnCompletion(fence_value, m_fenceEvent));
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }

        m_backbufferIndex = m_swapchain->GetCurrentBackBufferIndex();

        CloseHandle(m_fenceEvent);
        safe_release(m_fence);

        safe_release(m_rtvHeap);

        m_swapchain->Release();
        m_swapchain = nullptr;

        safe_release(m_cmdList);
        safe_release(m_cmdAllocator);
        safe_release(m_cmdQueue);

        safe_release(m_factory);

    #ifdef MILL_DEBUG
        {
            ComPtr<ID3D12InfoQueue1> info_queue = nullptr;
            m_device->QueryInterface(IID_PPV_ARGS(&info_queue));
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
            info_queue->UnregisterMessageCallback(m_debugCallbackCookie);
        }
        {
            // Report any live objects
            ComPtr<ID3D12DebugDevice2> debug_device = nullptr;
            m_device->QueryInterface(IID_PPV_ARGS(&debug_device));
            safe_release(m_device);
            // #NOTE: This will always report the debug_device
            debug_device->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
        }
    #endif

        safe_release(m_device);
#endif
    }

    void RendererD3D12::render(const SceneInfo& /*scene_info*/)
    {
        m_device->begin_frame();

        m_graphicsContext->reset();

        auto& backbuffer = *m_device->get_current_back_buffer(m_hwnd);

        m_graphicsContext->add_barrier(backbuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);

        m_graphicsContext->clear_render_target(backbuffer, { 0.36f, 0.54f, 0.86f, 1.0f });

        m_graphicsContext->add_barrier(backbuffer, D3D12_RESOURCE_STATE_PRESENT);

        m_device->submit_context(*m_graphicsContext);

        m_device->end_frame();
        m_device->present();

#if 0
        // Wait for the GPU to finish with the command allocator and
        // reset the allocator once the GPU is done with it.
        // This frees the memory that was used to store the commands.
        // m_cmd->begin_frame();
        // auto* cmdList = m_cmd->get_cmd_list();

        // Record commands
        // ...
        //
        // Done recording commands. Now execute commands,
        // signal and increment the fence value for the next frame.
        // m_cmd->end_frame();

        // Record cmd list
        assert_if_failed(m_cmdAllocator->Reset());
        assert_if_failed(m_cmdList->Reset(m_cmdAllocator, nullptr));

        // Record commands

        // Indicate that the back buffer will be used as a render target
        auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_backbufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        m_cmdList->ResourceBarrier(1, &barrier);

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_backbufferIndex, m_rtvDescriptorSize);

        const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
        m_cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

        // Indicate that the back buffer will now be used to present.
        barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_renderTargets[m_backbufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        m_cmdList->ResourceBarrier(1, &barrier);

        assert_if_failed(m_cmdList->Close());

        // Execute cmd list
        std::vector<ID3D12CommandList*> cmdLists = { m_cmdList };
        m_cmdQueue->ExecuteCommandLists(static_cast<u32>(cmdLists.size()), cmdLists.data());

        // Present frame
        assert_if_failed(m_swapchain->Present(1, 0));

        /* Wait for previous frame */

        // Signal and increment fence value
        const u64 fence_value = m_fenceValue;
        assert_if_failed(m_cmdQueue->Signal(m_fence, fence_value));
        ++m_fenceValue;

        // Wait until the previous frame is finished
        if (m_fence->GetCompletedValue() < fence_value)
        {
            assert_if_failed(m_fence->SetEventOnCompletion(fence_value, m_fenceEvent));
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }

        m_backbufferIndex = m_swapchain->GetCurrentBackBufferIndex();
#endif
    }

    auto create_renderer() -> Owned<RendererInterface>
    {
        return CreateOwned<RendererD3D12>();
    }

}
