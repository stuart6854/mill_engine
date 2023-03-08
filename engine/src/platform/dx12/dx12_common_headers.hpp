#pragma once

#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>  // For ComPtr smart pointer class
using namespace Microsoft::WRL;

// Tell linker which librarys to link to for DX12
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

// Assert that COM call to D3d API succeeded
#ifndef DXCall
    #ifdef MILL_DEBUG
        #define DXCall(x) ASSERT(FAILED(x) == false)
    #else
        #define DXCall(x) x
    #endif
#endif

#ifndef NAME_D3D12_OBJECT
    #ifdef MILL_DEBUG
        #define NAME_D3D12_OBJECT(obj, name) obj->SetName(name)
    #else
        #define NAME_D3D12_OBJECT(obj, name)
    #endif
#endif