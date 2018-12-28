//
// Copyright (c) 2018 Marcus Larsson
// 



#define UNICODE
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <d3d10.h>
#include <dxgi.h>
#ifdef DEBUG
#include <dxgidebug.h>
#endif

#include <assert.h>

#include "Mathematics.h"


#include <stdio.h>
#define DebugPrint(...) {wchar_t cad[512]; swprintf_s(cad, sizeof(cad), __VA_ARGS__);  OutputDebugString(cad);}



//
// Todo:
// - Error checking!
// - Enumerate adapters
// - Get supported resolutions
// - Support full-screen
//



static f32 const g_BackgroundColour[] = {0.2f, 0.5f, 0.8f, 1.0f};


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    switch (uMsg) {
        case WM_CLOSE: {
            OutputDebugString(L"WM_CLOSE\n");
        } break;
        
        case WM_SIZE: {
            OutputDebugString(L"WM_SIZE\n");
        } break;
        
        case WM_DESTROY: {
            OutputDebugString(L"WM_DESTROY\n");
            PostQuitMessage(0);
            return 0;
        } break;
        
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hInstancePrev, LPSTR lpCmdLine, int nShowCmd) 
{
    // TODO(Marcus): Handle resolution in a proper way
    int const Width = 1024;
    int const Height = 1024;
    
    //
    // Create window
    HWND hWnd;
    {
        wchar_t const ClassName[] = L"Window Class";
        
        WNDCLASS WC;
        
        WC.style = CS_VREDRAW | CS_HREDRAW;
        WC.lpfnWndProc = WindowProc;
        WC.cbClsExtra = 0;
        WC.cbWndExtra = 0;
        WC.hInstance = hInstance;
        WC.hIcon = nullptr;
        WC.hCursor = LoadCursor(nullptr, IDC_ARROW);
        WC.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        WC.lpszMenuName = nullptr;
        WC.lpszClassName = TEXT("test");
        
        RECT Rect;
        Rect.left = 0;
        Rect.top = 0;
        Rect.right = Width;
        Rect.bottom = Height;
        AdjustWindowRect(&Rect, WC.style, false);
        
        if (!RegisterClass(&WC)) 
        {
            return 1;
        }
        
        hWnd = CreateWindow(
            TEXT("test"),                   /* Class Name */
            TEXT("test"),                   /* Title */
            WS_OVERLAPPEDWINDOW,            /* Style */
            CW_USEDEFAULT, CW_USEDEFAULT,   /* Position */
            Rect.right, Rect.bottom,        /* Size */
            nullptr,                        /* Parent */
            nullptr,                        /* No menu */
            hInstance,                      /* Instance */
            0);                             /* No special parameters */
        
        if (!hWnd)
        {
            DWORD Error = GetLastError();
            OutputDebugString(L"Failed to create window!\n");
            return Error;
        }
    }
    
    
    
    //
    // DirectX 10
    //
    
    //
    // Create the device
    ID3D10Device *Device;
    {
        // This flag adds support for surfaces with a color-channel ordering different
        // from the API default. It is required for compatibility with Direct2D.
        // @note According to documentation, this should be used with CreateDevice1 which creates a 10.1 device?
        // UINT DeviceFlags = D3D10_CREATE_DEVICE_BGRA_SUPPORT;
        // TODO(Marcus): Handle pixel format a bit more professionaly, check which formats that are
        //               supported and try to use BGRA -- it seems like this is the format used by Windows.
        //               -- 2018-12-16
        //
        UINT DeviceFlags = 0;
        
#ifdef DEBUG
        DeviceFlags = DeviceFlags | D3D10_CREATE_DEVICE_DEBUG;
#endif
        
        //
        // Create device (default GPU)
        HRESULT Result = D3D10CreateDevice(nullptr,                     // Adapter, nullptr = default
                                           D3D10_DRIVER_TYPE_HARDWARE,  // Driver type
                                           nullptr,                     // HMODULE
                                           DeviceFlags,                 // Flags
                                           D3D10_SDK_VERSION,           // Version of the SDK
                                           &Device);                 // Pointer to the device
        if (FAILED(Result) || !Device) 
        {
            OutputDebugString(L"Failed to create device!\n");
            return 1;
        }
    }
    
    
    
    //
    // Fill out the structure describing the SwapChain
    DXGI_SWAP_CHAIN_DESC SwapChainDesc;
    {
        ZeroMemory(&SwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
        
        SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        SwapChainDesc.BufferDesc.Width = Width;
        SwapChainDesc.BufferDesc.Height = Height;
        SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
        SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
        //SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        SwapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        SwapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        
        SwapChainDesc.SampleDesc.Count = 1;      // multisampling setting
        SwapChainDesc.SampleDesc.Quality = 0;    // vendor-specific flag
        
        SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        SwapChainDesc.BufferCount = 2;
        SwapChainDesc.OutputWindow = hWnd;
        SwapChainDesc.Windowed = true;           // Sets the initial state of full-screen mode.
        SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        SwapChainDesc.Flags = 0;
    }
    
    
    
    //
    // Create the SwapChain
    IDXGISwapChain *SwapChain;
    IDXGIFactory *Factory;
    {
        HRESULT Result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&Factory));
        if (FAILED(Result)) 
        {
            // TODO(Marcus): Add better error handling
            OutputDebugString(L"Failed to create DXGIFactory!\n");
            return 1;
        }
        
        Result = Factory->CreateSwapChain(Device, &SwapChainDesc, &SwapChain);
        if (FAILED(Result)) 
        {
            // TODO(Marcus): Add better error handling
            OutputDebugString(L"Failed to create the swap chain!\n");
            return 1;
        }
    }
    
    
    
    //
    // Get a pointer to the backbuffer
    ID3D10Texture2D *Backbuffer;
    {
        HRESULT Result = SwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), reinterpret_cast<void**>(&Backbuffer));
        if (FAILED(Result)) 
        {
            // TODO(Marcus): Add better error handling
            OutputDebugString(L"Failed to get the backbuffer!\n");
            return 1;
        }
    }
    
    
    //
    // Depth and stencil buffer
    ID3D10Texture2D *DepthStencilBuffer;
    ID3D10DepthStencilView *DepthStencilView;
    ID3D10DepthStencilState *DepthStencilState;
    {
        D3D10_TEXTURE2D_DESC TexDesc;
        Backbuffer->GetDesc(&TexDesc);
        TexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        TexDesc.Usage = D3D10_USAGE_DEFAULT;
        TexDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
        HRESULT Result = Device->CreateTexture2D(&TexDesc, nullptr, &DepthStencilBuffer);
        if (FAILED(Result)) 
        {
            // TODO(Marcus): Add better error handling
            OutputDebugString(L"Failed to create the depth-/stencil-buffer!\n");
            return 1;
        }
        
        
        D3D10_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc;
        DepthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        DepthStencilViewDesc.ViewDimension = D3D10_DSV_DIMENSION_TEXTURE2D;
        DepthStencilViewDesc.Texture2D.MipSlice = 0;
        Result = Device->CreateDepthStencilView(DepthStencilBuffer, &DepthStencilViewDesc, &DepthStencilView);
        if (FAILED(Result)) 
        {
            // TODO(Marcus): Add better error handling
            OutputDebugString(L"Failed to create the depth-/stencil-view!\n");
            return 1;
        }
        
        //
        // Depth-/stencil-state
        D3D10_DEPTH_STENCIL_DESC DepthStencilDesc;
        
        // Depth test parameters
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
        Result = Device->CreateDepthStencilState(&DepthStencilDesc, &DepthStencilState);
        if (FAILED(Result)) 
        {
            // TODO(Marcus): Add better error handling
            OutputDebugString(L"Failed to create the depth-/stencil-state!\n");
            return 1;
        }
        
        Device->OMSetDepthStencilState(DepthStencilState, 1);
    }
    
    
    //
    // Create the render target view
    ID3D10RenderTargetView *RenderTargetView;
    {
        HRESULT Result = Device->CreateRenderTargetView(Backbuffer, nullptr, &RenderTargetView);
        if (FAILED(Result)) 
        {
            // TODO(Marcus): Add better error handling
            OutputDebugString(L"Failed to create the RenderTargetView!\n");
            return 1;
        }
        Device->OMSetRenderTargets(1, &RenderTargetView, DepthStencilView);
    }
    
    
    
    //
    // Viewport
    D3D10_VIEWPORT ViewPort;
    {
        ViewPort.TopLeftX = 0;
        ViewPort.TopLeftY = 0;
        ViewPort.Width    = Width;
        ViewPort.Height   = Height;
        ViewPort.MinDepth = 0.0f;
        ViewPort.MaxDepth = 1.0f;
        Device->RSSetViewports(1, &ViewPort);
    }
    
    
    
    //
    // Vertexbuffer
    ID3D10Buffer *VertexBuffer;
    {
        // Assuming that the origin is in the center and the vertices are in clockwise order.
        v3 TriangleVertices[] = {{-1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}};
        
        D3D10_BUFFER_DESC BufferDesc;
        BufferDesc.ByteWidth = sizeof(TriangleVertices); // size of the buffer
        BufferDesc.Usage = D3D10_USAGE_DEFAULT;          // only usable by the GPU
        BufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER; // use in vertex shader
        BufferDesc.CPUAccessFlags = 0;                   // No CPU access to the buffer
        BufferDesc.MiscFlags = 0;                        // No other option
        
        D3D10_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = TriangleVertices;
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;
        
        HRESULT Result = Device->CreateBuffer(&BufferDesc, &InitData, &VertexBuffer);
        if (FAILED(Result)) 
        {
            // TODO(Marcus): Add better error handling
            OutputDebugString(L"Failed to create the Vertex buffer\n");
            return 1;
        }
        
        u32 const stride = sizeof(v3);
        u32 const offset = 0;
        Device->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
        Device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    }
    
    
    //
    // Shaders
    ID3D10VertexShader *VertexShader;
    ID3D10InputLayout *InputLayout;
    //
    // Vertex shader
    {
        // Open file
        FILE *ShaderFile;
        fopen_s(&ShaderFile, "shaders/BasicVertexShader.cso", "rb");
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
                                                    &VertexShader);
        if (FAILED(Result)) {
            if (Bytes)  free(Bytes);
            OutputDebugString(L"Failed to create vertex shader from file!\n");
            return 1;
        }
        Device->VSSetShader(VertexShader);
        
        
        // Input layout
        D3D10_INPUT_ELEMENT_DESC E = {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0};
        Result = Device->CreateInputLayout(&E, 1, Bytes, BytesRead, &InputLayout);
        
        if (Bytes)  free(Bytes);
        
        if (FAILED(Result)) {
            OutputDebugString(L"Failed to create element layout!\n");
            return 1;
        }
        Device->IASetInputLayout(InputLayout);
    }
    
    
    //
    // Pixel shader
    ID3D10PixelShader *PixelShader;
    {
        // Open file
        FILE *ShaderFile;
        fopen_s(&ShaderFile, "shaders/BasicPixelShader.cso", "rb");
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
                                                   &PixelShader);
        if (FAILED(Result)) {
            if (Bytes)  free(Bytes);
            OutputDebugString(L"Failed to create pixel shader from file!\n");
            return 1;
        }
        Device->PSSetShader(PixelShader);
    }
    
    
    
    //
    // Show and update window
    ShowWindow(hWnd, nShowCmd);
    UpdateWindow(hWnd);
    
    
    
    //
    // The main loop
    b32 ShouldRun = true;
    MSG msg;
    while (ShouldRun) 
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            
            if (msg.message == WM_QUIT) {
                ShouldRun = false;
            }
        }
        
        //
        // Clear
        Device->ClearDepthStencilView(DepthStencilView, D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1, 0);
        Device->ClearRenderTargetView(RenderTargetView, g_BackgroundColour);
        
        //
        // Render triangle
        Device->Draw(3, 0);
        
        //
        // Update
        SwapChain->Present(0, 0);
    }
    
    
    
    //
    // @debug
    IDXGIDebug *DebugInterface;
    {
        HMODULE hModule = GetModuleHandleA("Dxgidebug.dll");
        if (!hModule) {
            OutputDebugString(L"Failed to get module handle to Dxgidebug.dll\n");
            return 1;
        }
        
        typedef HRESULT (*GETDEBUGINTERFACE)(REFIID, void **);
        GETDEBUGINTERFACE GetDebugInterface;
        GetDebugInterface = (GETDEBUGINTERFACE)GetProcAddress(hModule, "DXGIGetDebugInterface");
        
        HRESULT Result = GetDebugInterface(__uuidof(IDXGIDebug), (void **)&DebugInterface);
        if (FAILED(Result)) 
        {
            // TODO(Marcus): Add better error handling
            OutputDebugString(L"Failed to get the debug interface!\n");
            return 1;
        }
    }
    
    
    
    //
    // Clean up
    if (PixelShader) {
        PixelShader->Release();
    }
    
    if (InputLayout) {
        InputLayout->Release();
    }
    
    if (VertexShader) {
        VertexShader->Release();
    }
    
    if (VertexBuffer) {
        VertexBuffer->Release();
    }
    
    if (DepthStencilState) {
        Device->OMSetDepthStencilState(nullptr, 1); // // TODO(Marcus): Find out if this is correct!
        DepthStencilState->Release();
    }
    
    if (DepthStencilView) {
        DepthStencilView->Release();
    }
    
    if (DepthStencilBuffer) {
        DepthStencilBuffer->Release();
    }
    
    if (RenderTargetView) {
        RenderTargetView->Release();
    }
    
    if (Backbuffer) {
        OutputDebugString(L"Releasing the backbuffer.\n");
        Backbuffer->Release();
    }
    
    if (SwapChain) {
        OutputDebugString(L"Releasing SwapChain.\n");
        SwapChain->Release();
    }
    
    if (Factory) {
        OutputDebugString(L"Releasing Factory.\n");
        Factory->Release();
    }
    
    if (Device) {
        OutputDebugString(L"Releasing D3DDevice.\n");
        Device->Release();
    }
    
    OutputDebugString(L"***** Begin ReportLiveObjects call *****\n");
    DebugInterface->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
    OutputDebugString(L"***** End   ReportLiveObjects call *****\n\n");
    
    if (DebugInterface) {
        DebugInterface->Release();
    }
    
    return msg.wParam;
}