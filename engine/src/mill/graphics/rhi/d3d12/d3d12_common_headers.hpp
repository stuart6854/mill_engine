#pragma once

#include "mill/core/base.hpp"
#include "mill/core/debug.hpp"

#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <directx/d3dx12.h>
#include <wrl.h>  // For ComPtr smart pointer class
using namespace Microsoft::WRL;

// Tell linker which librarys to link to for DX12
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace mill::platform
{
    constexpr u32 g_FrameBufferCount = 2;
}

// Assert that COM call to D3d API succeeded
#ifndef DXCall
    #ifdef MILL_DEBUG
        #define DXCall(x) ASSERT(FAILED(x) == false)
    #else
        #define DXCall(x) x
    #endif
#endif

#ifdef MILL_DEBUG
    #define NAME_D3D12_OBJECT(obj, name) obj->SetName(name)
    #define NAME_D3D12_OBJECT_INDEXED(obj, n, name)           \
        do                                                    \
        {                                                     \
            wchar_t full_name[128];                           \
            if (swprintf_s(full_name, L"%s[%u]", name, n) > 0) \
            {                                                 \
                obj->SetName(full_name);                      \
            }                                                 \
        } while (false)
#else
    #define NAME_D3D12_OBJECT(obj, name)
    #define NAME_D3D12_OBJECT_INDEXED(obj, n, name)
#endif
