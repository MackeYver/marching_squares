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
#define assert(x)
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



void ReleaseDirectXState(directx_state *State)
{
    if (State)
    {
        ReleaseShader(&State->PixelShader);
        ReleaseShader(&State->VertexShader);
        
        State->RenderTargetView ? State->RenderTargetView->Release() : 0;
        State->DepthStencilState->Release() ? State->DepthStencilState->Release() : 0;
        State->DepthStencilView->Release() ? State->DepthStencilView->Release() : 0;
        State->DepthStencilBuffer->Release() ? State->DepthStencilBuffer->Release() : 0;
        State->SwapChain->Release() ? State->SwapChain->Release() : 0;
        State->Device->Release() ? State->Device->Release() : 0;
    }
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



//
// Buffers
//
b32 CreateBuffer(directx_state *State, 
                 D3D10_BIND_FLAG BindFlag, 
                 void *Data, size_t ElementSize, u32 ElementCount,
                 directx_buffer *Buffer)
{
    assert(State);
    assert(State->Device);
    assert(ElementSize > 0 && ElementCount > 0);
    assert(Buffer);
    
    Buffer->BindFlag = BindFlag;
    Buffer->ElementSize = ElementSize;
    Buffer->ElementCount = ElementCount;
    
    size_t Size = ElementCount * ElementSize;
    if (Size % 16 != 0)
    {
        Size = 16 * (u32)ceilf((f32)Size / 16.0f);
    }
    
    D3D10_BUFFER_DESC BufferDesc;
    BufferDesc.ByteWidth = Size;             // size of the buffer
    BufferDesc.Usage = D3D10_USAGE_DEFAULT;  // only usable by the GPU
    BufferDesc.BindFlags = Buffer->BindFlag;
    BufferDesc.CPUAccessFlags = 0;           // No CPU access to the buffer
    BufferDesc.MiscFlags = 0;                // No other option
    
    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = Data;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;
    
    HRESULT Result = State->Device->CreateBuffer(&BufferDesc, Data ? &InitData : nullptr, &Buffer->Ptr);
    if (FAILED(Result)) 
    {
        printf("Failed to create the buffer\n");
        return false;
    }
    
    
    return true;
}

void UpdateBuffer(directx_state *State, directx_buffer *Buffer, void *Data, D3D10_BOX *Box)
{
    assert(State);
    assert(State->Device);
    assert(Buffer && Buffer->Ptr);
    assert(Data);
    
    State->Device->UpdateSubresource(Buffer->Ptr, // Resource to update
                                     0,           // Subresource index
                                     Box,         // Destination box, nullptr for the entire buffer 
                                     Data,        // Pointer to the data                
                                     0,           // Row pitch (only for textures?) 
                                     0);          // Depth pitch (only for textures?)
}

void ReleaseBuffer(directx_buffer *Buffer)
{
    if (Buffer)
    {
        Buffer->Ptr ? Buffer->Ptr->Release() : 0;
        Buffer->ElementCount = 0;
        Buffer->ElementSize = 0;
    }
}



//
// Constant buffer
b32 CreateConstantBuffer(directx_state *State, void *Data, size_t DataSize, directx_buffer *Buffer)
{
    b32 Result = CreateBuffer(State, D3D10_BIND_CONSTANT_BUFFER, Data, DataSize, 1, Buffer);
    assert(Result);
    
    return Result;
}

void UseConstantBuffer(directx_state *State, directx_buffer *Buffer, u32 SlotNumber)
{
    assert(State);
    assert(State->Device);
    assert(Buffer);
    assert(Buffer->BindFlag == D3D10_BIND_CONSTANT_BUFFER);
    
    ID3D10Device *Device = State->Device;
    Device->VSSetConstantBuffers(SlotNumber, 1, &Buffer->Ptr);
    Device->PSSetConstantBuffers(SlotNumber, 1, &Buffer->Ptr);
}



//
// Vertex and index buffers
b32 CreateVertexBuffer(directx_state *State, void *Data, size_t VertexSize, u32 VertexCount, directx_buffer *Buffer)
{
    b32 Result = CreateBuffer(State, D3D10_BIND_VERTEX_BUFFER, Data, VertexSize, VertexCount, Buffer);
    assert(Result);
    
    return Result;
}

b32 CreateIndexBuffer(directx_state *State, void *Data, size_t IndexSize, u32 IndexCount, directx_buffer *Buffer)
{
    b32 Result = CreateBuffer(State, D3D10_BIND_INDEX_BUFFER, Data, IndexSize, IndexCount, Buffer);
    assert(Result);
    
    return Result;
}



//
// Renderables
// 

//
// Indexed
b32 CreateRenderable(directx_state *State,
                     directx_renderable_indexed *Renderable,
                     void *VertexData, size_t VertexSize, u32 VertexCount,
                     void *IndexData, size_t IndexSize, u32 IndexCount,
                     D3D10_PRIMITIVE_TOPOLOGY Topology)
{
    //
    // Vertexbuffer
    b32 Result = CreateVertexBuffer(State, VertexData, VertexSize, VertexCount, &Renderable->VertexBuffer);
    assert(Result);
    
    Result = CreateIndexBuffer(State, IndexData, IndexSize, IndexCount, &Renderable->IndexBuffer);
    assert(Result);
    
    Renderable->Topology = Topology;
    
    return true;
}


void RenderRenderable(directx_state *State, directx_renderable_indexed *Renderable,
                      u32 const Stride, u32 const Offset)
{
    assert(State);
    assert(State->Device);
    assert(Renderable);
    assert(Renderable->VertexBuffer.Ptr);
    assert(Renderable->IndexBuffer.Ptr && Renderable->IndexBuffer.ElementCount > 0);
    
    ID3D10Device *Device = State->Device;
    Device->IASetVertexBuffers(0, 1, &Renderable->VertexBuffer.Ptr, &Stride, &Offset);
    Device->IASetIndexBuffer(Renderable->IndexBuffer.Ptr, DXGI_FORMAT_R16_UINT, 0);
    Device->IASetPrimitiveTopology(Renderable->Topology);
    Device->DrawIndexed(Renderable->IndexBuffer.ElementCount, 0, 0);
}



void ReleaseRenderable(directx_renderable_indexed *Renderable)
{
    if (Renderable)
    {
        ReleaseBuffer(&Renderable->VertexBuffer);
        ReleaseBuffer(&Renderable->IndexBuffer);
    }
}


//
// Non-indexed
b32 CreateRenderable(directx_state *State,
                     directx_renderable *Renderable,
                     void *VertexData, size_t VertexSize, u32 VertexCount,
                     D3D10_PRIMITIVE_TOPOLOGY Topology)
{
    assert(State);
    assert(State->Device);
    assert(Renderable);
    assert(VertexData && VertexCount > 0);
    
    ID3D10Device *Device = State->Device;
    
    //
    // Vertexbuffer
    Renderable->VertexBuffer.ElementCount = VertexCount;
    Renderable->VertexBuffer.ElementSize = VertexSize;
    
    size_t Size = VertexCount * VertexSize;
    D3D10_BUFFER_DESC BufferDesc;
    BufferDesc.ByteWidth = Size;                     // size of the buffer
    BufferDesc.Usage = D3D10_USAGE_DEFAULT;          // only usable by the GPU
    BufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER; // use in vertex shader
    BufferDesc.CPUAccessFlags = 0;                   // No CPU access to the buffer
    BufferDesc.MiscFlags = 0;                        // No other option
    
    D3D10_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = VertexData;
    InitData.SysMemPitch = 0;
    InitData.SysMemSlicePitch = 0;
    
    HRESULT Result = Device->CreateBuffer(&BufferDesc, &InitData, &Renderable->VertexBuffer.Ptr);
    if (FAILED(Result)) 
    {
        printf("Failed to create the Vertexbuffer\n");
        return false;
    }
    
    Renderable->Topology = Topology;
    
    return true;
}



void RenderRenderable(directx_state *State, directx_renderable *Renderable,
                      u32 const Stride, u32 const Offset)
{
    assert(State);
    assert(State->Device);
    assert(Renderable);
    assert(Renderable->VertexBuffer.Ptr);
    assert(Renderable->VertexBuffer.ElementCount > 0);
    
    ID3D10Device *Device = State->Device;
    Device->IASetVertexBuffers(0, 1, &Renderable->VertexBuffer.Ptr, &Stride, &Offset);
    Device->IASetPrimitiveTopology(Renderable->Topology);
    Device->Draw(Renderable->VertexBuffer.ElementCount, 0);
}



void ReleaseRenderable(directx_renderable *Renderable)
{
    if (Renderable)
    {
        ReleaseBuffer(&Renderable->VertexBuffer);
    }
}



//
// Shaders
//


//
// Vertex shader
// TODO(Marcus): Pull out the input layout as well, currently this only allows one layout...
b32 CreateShader(directx_state *State, const char *PathAndName, vertex_shader *Shader)
{
    assert(State);
    assert(State->Device);
    assert(PathAndName);
    assert(Shader);
    
    ID3D10Device *Device = State->Device;
    
    // Open file
    FILE *ShaderFile;
    fopen_s(&ShaderFile, PathAndName, "rb");
    assert(ShaderFile);
    
    // Get size
    fseek(ShaderFile, 0L, SEEK_END);
    size_t FileSize = ftell(ShaderFile);
    rewind(ShaderFile);
    
    u8 *Bytes = (u8 *)malloc(FileSize);
    assert(Bytes);
    
    // Read file
    size_t BytesRead = fread_s(Bytes, FileSize, 1, FileSize, ShaderFile);
    fclose(ShaderFile);
    
    // Create shader
    HRESULT Result = Device->CreateVertexShader(Bytes,
                                                BytesRead,
                                                &Shader->Program);
    if (FAILED(Result)) {
        if (Bytes)  free(Bytes);
        printf("Failed to create vertex shader from file!\n");
        return false;
    }
    
    
    // Input layout
    D3D10_INPUT_ELEMENT_DESC E = {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0};
    Result = Device->CreateInputLayout(&E, 1, Bytes, BytesRead, &Shader->InputLayout);
    
    if (Bytes)  free(Bytes);
    
    if (FAILED(Result)) {
        printf("Failed to create element layout!\n");
        return false;
    }
    
    return true;
}

void ReleaseShader(vertex_shader *Shader)
{
    if (Shader)
    {
        Shader->InputLayout ? Shader->InputLayout->Release() : 0;
        Shader->Program ? Shader->Program->Release() : 0;
    }
}

void UseShader(directx_state *State, vertex_shader *Shader)
{
    assert(State && State->Device);
    assert(Shader);
    assert(Shader->Program);
    assert(Shader->InputLayout);
    
    State->Device->VSSetShader(Shader->Program);
    State->Device->IASetInputLayout(Shader->InputLayout);
}


b32 CreateShader(directx_state *State, const char *PathAndName, pixel_shader *Shader)
{
    assert(State);
    assert(State->Device);
    assert(PathAndName);
    assert(Shader);
    
    ID3D10Device *Device = State->Device;
    
    // Open file
    FILE *ShaderFile;
    fopen_s(&ShaderFile, PathAndName, "rb");
    assert(ShaderFile);
    
    // Get size
    fseek(ShaderFile, 0L, SEEK_END);
    size_t FileSize = ftell(ShaderFile);
    rewind(ShaderFile);
    
    u8 *Bytes = (u8 *)malloc(FileSize);
    assert(Bytes);
    
    // Read file
    size_t BytesRead = fread_s(Bytes, FileSize, 1, FileSize, ShaderFile);
    fclose(ShaderFile);
    
    // Create shader
    HRESULT Result = Device->CreatePixelShader(Bytes,
                                               BytesRead,
                                               &Shader->Program);
    
    if (Bytes)  free(Bytes);
    
    if (FAILED(Result)) {
        printf("Failed to create pixel shader from file!\n");
        return false;
    }
    
    return true;
}

void ReleaseShader(pixel_shader *Shader)
{
    
    if (Shader)
    {
        Shader->Program ? Shader->Program->Release() : 0;
    }
}

void UseShader(directx_state *State, pixel_shader *Shader)
{
    assert(State && State->Device);
    assert(Shader);
    assert(Shader->Program);
    
    State->Device->PSSetShader(Shader->Program);
}


//
// DirectWrite
//
b32 InitDirectWrite(directx_state *DirectXState, directwrite_state *State)
{
    assert(DirectXState);
    assert(DirectXState->Device);
    assert(DirectXState->SwapChain);
    assert(State);
    
    IDWriteFactory *DWriteFactory;
    HRESULT Result = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
                                         __uuidof(IDWriteFactory), 
                                         (IUnknown **)&DWriteFactory);
    if (FAILED(Result))
    {
        // TODO(Marcus): Add better error handling
        printf("Failed to create the DirectWrite factory!\n");
        return false;
    }
    
    
    //
    // Direct2D
    D2D1_FACTORY_OPTIONS Options;
    Options.debugLevel = D2D1_DEBUG_LEVEL_WARNING;
    ID2D1Factory1 *D2DFactory;
    Result = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, // Will only be used in this thread
                               __uuidof(ID2D1Factory),
                               &Options,
                               (void **)&D2DFactory);
    if (FAILED(Result))
    {
        DWriteFactory->Release();
        printf("Failed to create the Direct2D factory!\n");
        return false;
    }
    
    IDXGIDevice *DXGIDevice;
    Result = DirectXState->Device->QueryInterface(__uuidof(IDXGIDevice), (void **)&DXGIDevice);
    if (FAILED(Result))
    {
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to get the DXGIDevice from the D3D10Device1!\n");
        return false;
    }
    
    Result = D2DFactory->CreateDevice(DXGIDevice, &State->Device);
    if (FAILED(Result))
    {
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to create the D2DDevice!\n");
        return false;
    }
    
    ID2D1Device *Device = State->Device;
    Result = Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &State->DeviceContext);
    if (FAILED(Result))
    {
        State->Device->Release();
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to create the D2DDeviceContext!\n");
        return false;
    }
    
    IDXGISurface *D2DBuffer;
    Result = DirectXState->SwapChain->GetBuffer(0, __uuidof(IDXGISurface), (void **)&D2DBuffer);
    if (FAILED(Result))
    {
        State->DeviceContext->Release();
        State->Device->Release();
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to get the backbuffer as IDXGISurface!\n");
        return false;
    }
    
    D2D1_BITMAP_PROPERTIES1 Properties;
    Properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    Properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
    Properties.dpiX = 0;
    Properties.dpiX = 0;
    Properties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
    Properties.colorContext = nullptr;
    
    ID2D1DeviceContext *DeviceContext = State->DeviceContext;
    Result = DeviceContext->CreateBitmapFromDxgiSurface(D2DBuffer, Properties, &State->RenderTarget);
    if (FAILED(Result))
    {
        D2DBuffer->Release();
        State->DeviceContext->Release();
        State->Device->Release();
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to create the D2DBitmap!\n");
        return false;
    }
    DeviceContext->SetTarget(State->RenderTarget);
    
    Result = DeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 1.0f), &State->Brush);
    if (FAILED(Result))
    {
        State->RenderTarget->Release();
        D2DBuffer->Release();
        State->DeviceContext->Release();
        State->Device->Release();
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to create the D2DBrush!\n");
        return false;
    }
    
    Result = DWriteFactory->CreateTextFormat(L"Microsoft New Tai Lue",
                                             nullptr,
                                             DWRITE_FONT_WEIGHT_NORMAL,
                                             DWRITE_FONT_STYLE_NORMAL,
                                             DWRITE_FONT_STRETCH_NORMAL,
                                             96.0f/5.0f,
                                             L"en-GB",
                                             &State->TextFormat);
    if (FAILED(Result))
    {
        State->Brush->Release();
        State->RenderTarget->Release();
        D2DBuffer->Release();
        State->DeviceContext->Release();
        State->Device->Release();
        DXGIDevice->Release();
        D2DFactory->Release();
        DWriteFactory->Release();
        printf("Failed to create the D2DTextFormat!\n");
        return false;
    }
    
    State->TextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    State->TextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    
    D2DBuffer->Release();
    DXGIDevice->Release();
    D2DFactory->Release();
    DWriteFactory->Release();
    
    return true;
}



void ReleaseDirectWrite(directwrite_state *State)
{
    assert(State);
    State->TextFormat ? State->TextFormat->Release() : 0;
    State->Brush ? State->Brush->Release() : 0;
    State->RenderTarget ? State->RenderTarget->Release() : 0;
    State->DeviceContext? State->DeviceContext->Release() : 0;
    State->Device ? State->Device->Release() : 0;
}



void BeginDraw(directwrite_state *State)
{
    assert(State && State->DeviceContext);
    State->DeviceContext->BeginDraw();
}


HRESULT EndDraw(directwrite_state *State)
{
    assert(State && State->DeviceContext);
    HRESULT Result = State->DeviceContext->EndDraw();
    
    return Result;
}


void DrawTextW(directwrite_state *State, WCHAR const *String, v2 P)
{
    assert(State && State->DeviceContext);
    assert(String);
    
    v2 Offset = V2(60.0f, 20.0f);
    D2D1_RECT_F LayoutRect;
    LayoutRect.top    = P.y - Offset.y;
    LayoutRect.left   = P.x - Offset.x;
    LayoutRect.bottom = P.y + Offset.y;
    LayoutRect.right  = P.x + Offset.x;
    
    State->DeviceContext->DrawText(String,
                                   wcslen(String),
                                   State->TextFormat,
                                   &LayoutRect,
                                   State->Brush,
                                   D2D1_DRAW_TEXT_OPTIONS_NONE,
                                   DWRITE_MEASURING_MODE_NATURAL);
}
