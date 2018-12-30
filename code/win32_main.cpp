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
#include <cctype>

#include "Mathematics.h"
#include "oop_std.h"


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
        
        DWORD Style =  WS_CAPTION | WS_SYSMENU;
        
        RECT Rect;
        Rect.left = 0;
        Rect.top = 0;
        Rect.right = Width;
        Rect.bottom = Height;
        AdjustWindowRect(&Rect, Style, false);
        
        if (!RegisterClass(&WC)) 
        {
            return 1;
        }
        
        hWnd = CreateWindow(
            TEXT("test"),                   /* Class Name */
            TEXT("test"),                   /* Title */
            Style,                          /* Style */
            CW_USEDEFAULT, CW_USEDEFAULT,   /* Position */
            Rect.right - Rect.left, 
            Rect.bottom - Rect.top ,        /* Size */
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
    // Read data from file (using Win32)
    u32 *Data = nullptr;
    u32 DataCount = 0;
    {
        //HANDLE File = CreateFileA("c:\\developer\\Marching_squares\\data\\volcano.txt",
        HANDLE File = CreateFileA("c:\\developer\\Marching_squares\\data\\test2.txt",
                                  GENERIC_READ,
                                  FILE_SHARE_READ,
                                  nullptr,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  nullptr);
        if (File == INVALID_HANDLE_VALUE)
        {
            DWORD Error = GetLastError();
            DebugPrint(L"Failed to open file, error: %d\n", Error);
            return 1;
        }
        
        LARGE_INTEGER FileSize;
        GetFileSizeEx(File, &FileSize);
        assert(FileSize.QuadPart > 0);
        
        char *FileData = (char *)malloc((size_t)FileSize.QuadPart);
        assert(FileData);
        
        DWORD BytesRead = 0;
        if (!ReadFile(File, FileData, (DWORD)FileSize.QuadPart, &BytesRead, nullptr) || BytesRead != FileSize.QuadPart)
        {
            CloseHandle(File);
            DWORD Error = GetLastError();
            DebugPrint(L"Failed to read from file, error: %d", Error);
            return 1;
        }
        
        CloseHandle(File);
        
        // Count the number of elements
        u32 SpaceCount = 0;
        for (u32 Index = 0; Index < FileSize.QuadPart; ++Index)
        {
            if (FileData[Index] == ' ' || FileData[Index] == '\n')  ++SpaceCount;
        }
        
        DataCount = SpaceCount + 1; // We're assuming that there is only spaces in between the elements
        Data = (u32 *)malloc(DataCount * sizeof(u32));
        assert(Data);
        
        // Proccess the read data, i.e. convert to u32
        u32 DataIndex = 0;
        for (u32 Index = 0; Index < FileSize.QuadPart;)
        {
            if (!isdigit(FileData[Index]))  continue;
            
            char SmallBuffer[10] = {};
            
            u32 EndIndex = Index;
            while (EndIndex < FileSize.QuadPart && FileData[EndIndex] != ' ' && FileData[EndIndex] != '\n') {
                ++EndIndex;
            }
            
            assert(EndIndex != Index);
            assert(EndIndex - Index < 10);
            
            memcpy(SmallBuffer, &FileData[Index], EndIndex - Index);
            sscanf_s(SmallBuffer, "%d", &Data[DataIndex++]);
            
            Index = EndIndex + 1;
        }
        assert(DataIndex == DataCount);
        
        free(FileData);
        FileData = nullptr;
    }
    
    
    //
    // Generate lines from data using Marching Squares
    MarchingSquares::config Config;
    {
        Config.CellCountX = 5;
        Config.CellCountY = 5;
        f32 SmallestSize = min((f32)Width / (f32)(Config.CellCountX - 1), (f32)Height / (f32)(Config.CellCountY - 1));
        Config.CellSize = V2(SmallestSize, SmallestSize);
        Config.SourceHasOriginUpperLeft = false;
    }
    
    MarchingSquares MS(Data, DataCount, Config);
    //std::vector<f32> Heights = {80, 100, 120, 140, 160, 180};
    std::vector<f32> Heights = {2, 3};
    MS.MarchSquares(Heights);
    
    free(Data);
    Data = nullptr;
    DataCount = 0;
    
    
    //
    // DirectX 10
    //
    
    //
    // Create the device
    ID3D10Device *Device;
    {
        // This flag adds support for surfaces with a color-channel ordering different
        // from the API default. It is required for compatibility with Direct2D.
        // According to documentation, this should be used with CreateDevice1 which creates a 10.1 device?
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
    u32 VertexBufferCount = MS.GetLevelCount();
    assert(VertexBufferCount > 0);
    
    ID3D10Buffer **VertexBuffers;
    VertexBuffers = (ID3D10Buffer **)malloc(VertexBufferCount * sizeof(ID3D10Buffer *));
    assert(VertexBuffers);
    
    u32 *VertexCount;
    VertexCount = (u32 *)malloc(VertexBufferCount * sizeof(u32));
    assert(VertexCount);
    
    for (u32 Index = 0; Index < VertexBufferCount; ++Index)
    {
        MarchingSquares::level *Level = MS[Index];
        assert(Level);
        size_t Size = Level->LineSegments.size() * sizeof(line_segment);
        
        VertexCount[Index] = 2 * Level->LineSegments.size();
        
        D3D10_BUFFER_DESC BufferDesc;
        BufferDesc.ByteWidth = Size;                     // size of the buffer
        BufferDesc.Usage = D3D10_USAGE_DEFAULT;          // only usable by the GPU
        BufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER; // use in vertex shader
        BufferDesc.CPUAccessFlags = 0;                   // No CPU access to the buffer
        BufferDesc.MiscFlags = 0;                        // No other option
        
        D3D10_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = Level->LineSegments.data();
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;
        
        HRESULT Result = Device->CreateBuffer(&BufferDesc, &InitData, &VertexBuffers[Index]);
        if (FAILED(Result)) 
        {
            // TODO(Marcus): Add better error handling
            OutputDebugString(L"Failed to create the Vertex buffer\n");
            return 1;
        }
    }
    
    
    
    //
    // Vertexbuffer, grid lines
    ID3D10Buffer *VertexBufferGridLines;
    u32 VertexCountGridLines = 0;
    {
        //
        // Generate the data
        u32 CellCountX = MS.GetCellCountX();
        u32 CellCountY = MS.GetCellCountY();
        v2 CellSize = MS.GetCellSize();
        
        VertexCountGridLines = 2 * ((CellCountX + 1) + (CellCountY + 1));
        size_t Size = VertexCountGridLines * sizeof(v2);
        v2 *GridLinesVertices = (v2 *)malloc(Size);
        assert(GridLinesVertices);
        
        u32 DataIndex = 0;
        for (u32 y = 0; y <= CellCountY; ++y)
        {
            GridLinesVertices[DataIndex++] = Hadamard(CellSize, V2(0, y));
            GridLinesVertices[DataIndex++] = Hadamard(CellSize, V2(CellCountX, y));
        }
        
        for (u32 x = 0; x <= CellCountX; ++x)
        {
            GridLinesVertices[DataIndex++] = Hadamard(CellSize, V2(x, 0));
            GridLinesVertices[DataIndex++] = Hadamard(CellSize, V2(x, CellCountY));
        }
        assert(DataIndex == VertexCountGridLines);
        
        
        //
        // Create the buffer
        D3D10_BUFFER_DESC BufferDesc;
        BufferDesc.ByteWidth = Size;                     // size of the buffer
        BufferDesc.Usage = D3D10_USAGE_DEFAULT;          // only usable by the GPU
        BufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER; // use in vertex shader
        BufferDesc.CPUAccessFlags = 0;                   // No CPU access to the buffer
        BufferDesc.MiscFlags = 0;                        // No other option
        
        D3D10_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = GridLinesVertices;
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;
        
        HRESULT Result = Device->CreateBuffer(&BufferDesc, &InitData, &VertexBufferGridLines);
        if (FAILED(Result)) 
        {
            // TODO(Marcus): Add better error handling
            OutputDebugString(L"Failed to create the Vertex buffer for the grid lines\n");
            return 1;
        }
        
        free(GridLinesVertices);
        GridLinesVertices = nullptr;
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
        fopen_s(&ShaderFile, "shaders\\BasicVertexShader.cso", "rb");
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
        D3D10_INPUT_ELEMENT_DESC E = {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D10_INPUT_PER_VERTEX_DATA, 0};
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
        fopen_s(&ShaderFile, "shaders\\BasicPixelShader.cso", "rb");
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
    // Constant buffer, used by the shaders
    ID3D10Buffer *ConstantBuffer;
    struct shader_constants
    {
        m4 TransformMatrix;
        v4 Colour;
        v3 Padding;
        f32 Z;
    };
    
    shader_constants ShaderConstants;
    {
        f32 Far = 1.0f;
        f32 Sx = 2.0f / (f32)Width;
        f32 Sy = 2.0f / (f32)Height;
        f32 Sz = 1.0f / Far;
        
        v4 X = {   Sx,  0.0f, 0.0f, 0.0f};
        v4 Y = { 0.0f,    Sy, 0.0f, 0.0f};
        v4 Z = { 0.0f,  0.0f,   Sz, 0.0f};
        v4 W = {-1.0f, -1.0f, 0.0f, 1.0f};
        
        ShaderConstants.TransformMatrix = M4(X, Y, Z, W);
        ShaderConstants.Colour = v4_one;
        ShaderConstants.Z = 0.0f;
        assert((sizeof(shader_constants) / 16) % 2 == 0);
        
        D3D10_BUFFER_DESC BufferDesc;
        BufferDesc.ByteWidth = sizeof(ShaderConstants);    
        //BufferDesc.Usage = D3D10_USAGE_DYNAMIC;             // We will write stuff to the buffer from the CPU
        //BufferDesc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;  // constant buffer
        //BufferDesc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE; // CPU write access to the buffer
        BufferDesc.Usage = D3D10_USAGE_DEFAULT;             
        BufferDesc.BindFlags = D3D10_BIND_CONSTANT_BUFFER;  // constant buffer
        BufferDesc.CPUAccessFlags = 0; 
        BufferDesc.MiscFlags = 0;                           
        
        D3D10_SUBRESOURCE_DATA InitData;
        InitData.pSysMem = &ShaderConstants;
        InitData.SysMemPitch = 0;
        InitData.SysMemSlicePitch = 0;
        
        HRESULT Result = Device->CreateBuffer(&BufferDesc, &InitData, &ConstantBuffer);
        if (FAILED(Result)) 
        {
            // TODO(Marcus): Add better error handling
            OutputDebugString(L"Failed to create the ConstantBuffer\n");
            return 1;
        }
        
        Device->VSSetConstantBuffers(0, 1, &ConstantBuffer);
        Device->PSSetConstantBuffers(0, 1, &ConstantBuffer);
    }
    
    
    
    //
    // Show and update window
    ShowWindow(hWnd, nShowCmd);
    UpdateWindow(hWnd);
    
    
    
    //
    // The main loop
    u32 const stride = sizeof(v2);
    u32 const offset = 0;
    Device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
    
#if 0
    static v4 const Colours[] = 
    {
        {0.0f, 0.0f, 1.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 0.0f, 0.0f, 1.0f},
        {1.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        
        {0.0f, 0.0f, 0.5f, 0.5f},
        {0.0f, 0.5f, 0.0f, 0.5f},
        {0.0f, 0.5f, 0.5f, 0.5f},
        {0.5f, 0.0f, 0.0f, 0.5f},
        {0.5f, 0.0f, 0.5f, 0.5f},
        {0.5f, 0.5f, 0.0f, 0.5f},
        {0.5f, 0.5f, 0.5f, 0.5f},
    };
#else
    static v4 const Colours[] = 
    {
        {0.4f, 0.4f, 0.4f, 1.0f},
        {0.5f, 0.5f, 0.5f, 1.0f},
        {0.6f, 0.6f, 0.6f, 1.0f},
        {0.7f, 0.7f, 0.7f, 1.0f},
        {0.8f, 0.8f, 0.8f, 1.0f},
        {0.9f, 0.9f, 0.9f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f},
    };
#endif
    static u32 const ColourCount = sizeof(Colours) / sizeof(*Colours);
    
    b32 ShouldRun = true;
    MSG msg;
    while (ShouldRun) 
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) 
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            
            if (msg.message == WM_QUIT) 
            {
                ShouldRun = false;
            }
        }
        
        //
        // Clear
        Device->ClearDepthStencilView(DepthStencilView, D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1, 0);
        static v4 const BackgroundColour = V4(0.0f, 0.0f, 0.0f, 1.0f);
        Device->ClearRenderTargetView(RenderTargetView, (float const *)&BackgroundColour);
        
        
        //
        // Render grid
        ShaderConstants.Colour = V4(0.2f, 0.2f, 0.2f, 1.0f);
        ShaderConstants.Z = 0.3f;
        // Refresh the data in the constant buffer
        Device->UpdateSubresource(ConstantBuffer,   // Resource to update
                                  0,                // Subresource index
                                  nullptr,          // Destination box, nullptr for the entire buffer 
                                  &ShaderConstants, // Pointer to the data             
                                  0,                // Row pitch (only for textures?) 
                                  0);               // Depth pitch (only for textures?)
        Device->IASetVertexBuffers(0, 1, &VertexBufferGridLines, &stride, &offset);
        Device->Draw(VertexCountGridLines, 0);
        
        
        //
        // Render levels
        for (u32 Index = 0; Index < VertexBufferCount; ++Index)
        {
            // Change the colour
            //u32 NewIndex = Index % ColourCount;
            //ShaderConstants.Colour = Colours[NewIndex];
            ShaderConstants.Colour = V4(1.0f, 1.0f, 1.0f, 1.0f);
            ShaderConstants.Z = 0.0f;
            
            // Refresh the data in the constant buffer
            Device->UpdateSubresource(ConstantBuffer,   // Resource to update
                                      0,                // Subresource index
                                      nullptr,          // Destination box, nullptr for the entire buffer 
                                      &ShaderConstants, // Pointer to the data             
                                      0,                // Row pitch (only for textures?) 
                                      0);               // Depth pitch (only for textures?)
            
            Device->IASetVertexBuffers(0, 1, &VertexBuffers[Index], &stride, &offset);
            Device->Draw(VertexCount[Index], 0);
        }
        
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
    if (ConstantBuffer)
    {
        ConstantBuffer->Release();
    }
    
    if (PixelShader) {
        PixelShader->Release();
    }
    
    if (InputLayout) {
        InputLayout->Release();
    }
    
    if (VertexShader) {
        VertexShader->Release();
    }
    
    if (VertexBuffers) {
        for (u32 Index = 0; Index < VertexBufferCount; ++Index)
        {
            if (VertexBuffers[Index])
            {
                VertexBuffers[Index]->Release();
            }
        }
        
        free(VertexBuffers);
        VertexBuffers = nullptr;
        VertexBufferCount = 0;
    }
    
    if (DepthStencilState) {
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