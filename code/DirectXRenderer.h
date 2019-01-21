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



//
// Shaders
//

//
// Vertex shader
struct vertex_shader
{
    ID3D10VertexShader *Program;
    ID3D10InputLayout *InputLayout;
};

struct directx_state;

b32 CreateShader(directx_state *State, const char *PathAndName, vertex_shader *Shader);
void UseShader(directx_state *State, vertex_shader *Shader);
void ReleaseShader(vertex_shader *Shader);

//
// Pixel shader
struct pixel_shader
{
    ID3D10PixelShader *Program;
};

b32 CreateShader(directx_state *State, const char *PathAndName, pixel_shader *Shader);
void UseShader(directx_state *State, pixel_shader *Shader);
void ReleaseShader(pixel_shader *Shader);



//
// Direct general state
//
struct directx_state
{
    ID3D10Device1 *Device;
    IDXGISwapChain *SwapChain;
    
    ID3D10Texture2D *RenderTarget;
    ID3D10RenderTargetView *RenderTargetView;
    
    ID3D10Texture2D *DepthStencilBuffer;
    ID3D10DepthStencilView *DepthStencilView;
    ID3D10DepthStencilState *DepthStencilState;
    
    vertex_shader VertexShader;
    pixel_shader PixelShader;
    
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
void SetViewport(directx_state *State, u32 x0 = 0, u32 y0 = 0, u32 w = 0, u32 h = 0, 
                 f32 MinDepth = 0.0f, f32 MaxDepth = 1.0f);
void ReleaseDirectXState(directx_state *State);



//
// Buffer
//
struct directx_buffer
{
    ID3D10Buffer *Ptr;
    size_t ElementSize;
    u32 ElementCount;
    D3D10_BIND_FLAG BindFlag;
};

b32 CreateBuffer(directx_state *State, 
                 D3D10_BIND_FLAG BindFlag, 
                 void *Data, size_t ElementSize, u32 ElementCount,
                 directx_buffer *Buffer);

void UpdateBuffer(directx_state *State, directx_buffer *Buffer, void *Data, D3D10_BOX *Box = nullptr);
void ReleaseBuffer(directx_buffer *Buffer);

b32 CreateConstantBuffer(directx_state *State, void *Data, size_t DataSize, directx_buffer *Buffer);
void UseConstantBuffer(directx_state *State, directx_buffer *Buffer, u32 SlotNumber = 0);

b32 CreateVertexBuffer(directx_state *State, void *Data, size_t VertexSize, u32 VertexCount, directx_buffer *Buffer);
b32 CreateIndexBuffer(directx_state *State, void *Data, size_t IndexSize, u32 IndexCount, directx_buffer *Buffer);



//
// Renderables
//

//
// Indexed
struct directx_renderable_indexed
{
    directx_buffer VertexBuffer;
    directx_buffer IndexBuffer;
    D3D10_PRIMITIVE_TOPOLOGY Topology;
};

b32 CreateRenderable(directx_state *State,
                     directx_renderable_indexed *Renderable,
                     void *VertexData, size_t VertexSize, u32 VertexCount,
                     void *IndexData, size_t IndexSize, u32 IndexCount,
                     D3D10_PRIMITIVE_TOPOLOGY Topology);

void RenderRenderable(directx_state *State, directx_renderable_indexed *Renderable, 
                      u32 const Stride = sizeof(v2), u32 const Offset = 0);

void ReleaseRenderable(directx_renderable_indexed *Renderable);


//
// Non-indexed
struct directx_renderable
{
    directx_buffer VertexBuffer;
    D3D10_PRIMITIVE_TOPOLOGY Topology;
};

b32 CreateRenderable(directx_state *State,
                     directx_renderable *Renderable,
                     void *VertexData, size_t VertexSize, u32 VertexCount,
                     D3D10_PRIMITIVE_TOPOLOGY Topology);

void RenderRenderable(directx_state *State, directx_renderable *Renderable, 
                      u32 const Stride = sizeof(v2), u32 const Offset = 0);

void ReleaseRenderable(directx_renderable *Renderable);



//
// DirectWrite
//

struct directwrite_state
{
    ID2D1Device *Device;
    ID2D1DeviceContext *DeviceContext;
    ID2D1Bitmap1 *RenderTarget;
    ID2D1SolidColorBrush *Brush;
    IDWriteTextFormat *TextFormat;
};

b32 InitDirectWrite(directx_state *DirectXState, directwrite_state *State);
void ReleaseDirectWrite(directwrite_state *State);
void BeginDraw(directwrite_state *State);
HRESULT EndDraw(directwrite_state *State);

void DrawText(directwrite_state *State, WCHAR const *String, v2 P);



#endif // End DirectXRenderer_h