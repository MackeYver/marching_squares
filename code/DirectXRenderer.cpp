// 
// MIT License
// 
// Copyright (c) 2018 Marcus Larsson
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//


//
// TODO
// - Add better error handling
//


#include "DirectXRenderer.h"

#ifdef DEBUG
#include <assert.h>
#else
#define assert()
#endif

#include <stdio.h>



b32 CreateDevice(directx_state *State)
{
    assert(State);
    
    // This flag adds support for surfaces with a color-channel ordering different
    // from the API default. It is required for compatibility with Direct2D.
    // UINT DeviceFlags = D3D10_CREATE_DEVICE_BGRA_SUPPORT;
    // TODO(Marcus): Handle pixel format a bit more professionaly, check which formats that are
    //               supported etc...
    //               -- 2018-12-16
    //
    UINT DeviceFlags = D3D10_CREATE_DEVICE_BGRA_SUPPORT;
    
#ifdef DEBUG
    DeviceFlags = DeviceFlags | D3D10_CREATE_DEVICE_DEBUG;
#endif
    
    //
    // Create device (default GPU)
    HRESULT Result = D3D10CreateDevice1(nullptr,                     // Adapter, nullptr = default
                                        D3D10_DRIVER_TYPE_HARDWARE,  // Driver type
                                        nullptr,                     // HMODULE
                                        DeviceFlags,                 // Flags
                                        D3D10_FEATURE_LEVEL_10_0,    // Feature level
                                        D3D10_1_SDK_VERSION,         // Version of the SDK
                                        &State->Device);             // Pointer to the device
    if (FAILED(Result) || !State->Device)
    {
        printf("Failed to create device!\n");
        return false;
    }
    
    return true;
}



b32 CreateSwapChain(directx_state *State, directx_config *Config)
{
    assert(State);
    assert(Config);
    assert(State->Device);
    
    if (Config->Width <= 0)  return false;
    if (Config->Height <= 0)  return false;
    
    State->Width = Config->Width;
    State->Height = Config->Height;
    
    
    //
    // SwapChain
    //
    //Fill out a structure describing the SwapChain
    DXGI_SWAP_CHAIN_DESC SwapChainDesc;
    ZeroMemory(&SwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    SwapChainDesc.BufferDesc.Width = State->Width;
    SwapChainDesc.BufferDesc.Height = State->Height;
    SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    
    SwapChainDesc.SampleDesc.Count = 1;      // multisampling setting
    SwapChainDesc.SampleDesc.Quality = 0;    // vendor-specific flag
    
    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.BufferCount = 2;
    SwapChainDesc.OutputWindow = Config->hWnd;
    SwapChainDesc.Windowed = true;           
    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    SwapChainDesc.Flags = 0;
    
    
    //
    // Create the SwapChain
    IDXGIFactory *Factory;
    HRESULT Result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&Factory);
    if (FAILED(Result)) 
    {
        printf("Failed to create DXGIFactory!\n");
        return false;
    }
    
    Result = Factory->CreateSwapChain(State->Device, &SwapChainDesc, &State->SwapChain);
    if (FAILED(Result)) 
    {
        printf("Failed to create the swap chain!\n");
        return false;
    }
    
    Factory->Release();
    
    
    //
    // Get a pointer to the backbuffer
    Result = State->SwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&State->RenderTarget);
    if (FAILED(Result)) 
    {
        printf("Failed to get the backbuffer!\n");
        return false;
    }
    
    return true;
}



b32 CreateDepthAndStencilbuffers(directx_state *State)
{
    assert(State);
    assert(State->Device);
    assert(State->SwapChain);
    assert(State->RenderTarget);
    
    ID3D10Device1 *Device = State->Device;
    
    
    //
    // Create the texture
    D3D10_TEXTURE2D_DESC TexDesc;
    State->RenderTarget->GetDesc(&TexDesc);
    TexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    TexDesc.Usage = D3D10_USAGE_DEFAULT;
    TexDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
    
    HRESULT Result = Device->CreateTexture2D(&TexDesc, nullptr, &State->DepthStencilBuffer);
    if (FAILED(Result)) 
    {
        printf("Failed to create the depth-/stencil-buffer!\n");
        return false;
    }
    
    
    D3D10_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc;
    DepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthStencilViewDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
    DepthStencilViewDesc.Texture2D.MipSlice = 0;
    
    Result = Device->CreateDepthStencilView(State->DepthStencilBuffer, 
                                            &DepthStencilViewDesc, 
                                            &State->DepthStencilView);
    if (FAILED(Result)) 
    {
        printf("Failed to create the depth-/stencil-view!\n");
        return false;
    }
    
    
    //
    // Depth-/stencil-state
    D3D10_DEPTH_STENCIL_DESC DepthStencilDesc;
    DepthStencilDesc.DepthEnable = true;
    DepthStencilDesc.DepthWriteMask = D3D10_DEPTH_WRITE_MASK_ALL;
    DepthStencilDesc.DepthFunc      = D3D10_COMPARISON_LESS;
    
    // Stencil test parameters
    DepthStencilDesc.StencilEnable = true;
    DepthStencilDesc.StencilReadMask = 0xFF;
    DepthStencilDesc.StencilWriteMask = 0xFF;
    
    // Stencil operations if pixel is front-facing
    DepthStencilDesc.FrontFace.StencilFailOp      = D3D10_STENCIL_OP_KEEP;
    DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D10_STENCIL_OP_INCR;
    DepthStencilDesc.FrontFace.StencilPassOp      = D3D10_STENCIL_OP_KEEP;
    DepthStencilDesc.FrontFace.StencilFunc        = D3D10_COMPARISON_ALWAYS;
    
    // Stencil operations if pixel is back-facing
    DepthStencilDesc.BackFace.StencilFailOp      = D3D10_STENCIL_OP_KEEP;
    DepthStencilDesc.BackFace.StencilDepthFailOp = D3D10_STENCIL_OP_DECR;
    DepthStencilDesc.BackFace.StencilPassOp      = D3D10_STENCIL_OP_KEEP;
    DepthStencilDesc.BackFace.StencilFunc        = D3D10_COMPARISON_ALWAYS;
    
    // Create depth stencil state
    Result = Device->CreateDepthStencilState(&DepthStencilDesc, &State->DepthStencilState);
    if (FAILED(Result)) 
    {
        printf("Failed to create the depth-/stencil-state!\n");
        return false;
    }
    
    Device->OMSetDepthStencilState(State->DepthStencilState, 1);
    
    return true;
}



b32 CreateRenderTargetView(directx_state *State)
{
    assert(State);
    assert(State->Device);
    assert(State->RenderTarget);
    assert(State->DepthStencilView);
    
    ID3D10Device1 *Device = State->Device;
    
    //
    // RenderTargetView
    HRESULT Result = Device->CreateRenderTargetView(State->RenderTarget, nullptr, &State->RenderTargetView);
    if (FAILED(Result)) 
    {
        printf("Failed to create the RenderTargetView!\n");
        return false;
    }
    
    Device->OMSetRenderTargets(1, &State->RenderTargetView, State->DepthStencilView);
    
    return true;
}


void SetViewport(directx_state *State, u32 x0, u32 y0, u32 w, u32 h, f32 MinDepth, f32 MaxDepth)
{
    assert(State);
    assert(State->Device);
    
    D3D10_VIEWPORT *Viewport = &State->Viewport;
    
    Viewport->TopLeftX = x0;
    Viewport->TopLeftY = y0;
    Viewport->Width    = w == 0 ? State->Width  : w;
    Viewport->Height   = h == 0 ? State->Height : h;
    Viewport->MinDepth = MinDepth;
    Viewport->MaxDepth = MaxDepth;
    
    State->Device->RSSetViewports(1, &State->Viewport);
}