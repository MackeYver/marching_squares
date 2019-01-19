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

#ifndef DirectXRenderer__h
#define DirectXRenderer__h

#include <d3d10_1.h>
#include <dxgi.h>
#ifdef DEBUG
#include <dxgidebug.h>
#endif

#include <dwrite.h> // DirectWrite (Dwrite.lib)
#include <d2d1_1.h> // Direct2D    (D2d1_1.lib)

#include "Mathematics.h"


struct directx_state
{
    ID3D10Device1 *Device;
    IDXGISwapChain *SwapChain;
    
    ID3D10Texture2D *RenderTarget;
    ID3D10RenderTargetView *RenderTargetView;
    
    ID3D10Texture2D *DepthStencilBuffer;
    ID3D10DepthStencilView *DepthStencilView;
    ID3D10DepthStencilState *DepthStencilState;
    
    D3D10_VIEWPORT Viewport;
    
    u32 Width;
    u32 Height;
};

struct directx_config
{
    HWND hWnd;
    u32 Width;
    u32 Height;
};


b32 CreateDevice(directx_state *State);
b32 CreateSwapChain(directx_state *State, directx_config *Config);
b32 CreateDepthAndStencilbuffers(directx_state *State);
b32 CreateRenderTargetView(directx_state *State);
void SetViewport(directx_state *State, 
                 u32 x0 = 0, u32 y0 = 0, 
                 u32 w = 0, u32 h = 0, 
                 f32 MinDepth = 0.0f, f32 MaxDepth = 1.0f);




#endif // End DirectXRenderer_h