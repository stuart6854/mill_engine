#include "renderer_dx12.hpp"

#include "mill/core/debug.hpp"

namespace mill::platform
{
    constexpr D3D_FEATURE_LEVEL minimum_feature_level = D3D_FEATURE_LEVEL_11_0;

    namespace
    {
        template <typename T>
        constexpr void release(T*& resource)
        {
            if (resource)
            {
                resource->Release();
                resource = nullptr;
            }
        }

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

                release(adapter);
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

    void RendererDX12::inititialise(const RendererInit& /*init*/)
    {
        ASSERT(m_device == nullptr);

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

        hr = CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&m_dxgi_factory));
        if (FAILED(hr))
        {
            LOG_ERROR("RendererDX12 - Failed to create factory!");
            return;
        }

        // Determine which adapter (graphics card) to use, if any
        ComPtr<IDXGIAdapter4> adapter = nullptr;
        adapter.Attach(determine_adapter(m_dxgi_factory));
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
            ComPtr<ID3D12InfoQueue> info_queue = nullptr;
            m_device->QueryInterface(IID_PPV_ARGS(&info_queue));
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
        }
#endif
    }

    void RendererDX12::shutdown()
    {
        release(m_dxgi_factory);

#ifdef MILL_DEBUG
        {
            ComPtr<ID3D12InfoQueue> info_queue = nullptr;
            m_device->QueryInterface(IID_PPV_ARGS(&info_queue));
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
        }
        {
            // Report any live objects
            ComPtr<ID3D12DebugDevice2> debug_device = nullptr;
            m_device->QueryInterface(IID_PPV_ARGS(&debug_device));
            release(m_device);
            // #NOTE: This will always report the debug_device
            debug_device->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
        }
#endif

        release(m_device);
    }

    void RendererDX12::wait_and_begin_frame() {}

    void RendererDX12::submit_and_present() {}

    auto create_renderer() -> Owned<RendererInterface>
    {
        return CreateOwned<RendererDX12>();
    }

}
