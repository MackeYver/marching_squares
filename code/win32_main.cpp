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


#define UNICODE
#define STRICT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "DirectXRenderer.h"

#include <assert.h>
#include <cctype>

#include "Mathematics.h"
#include "oop_std.h"


#include <stdio.h>
#define DebugPrint(...) {wchar_t cad[512]; swprintf_s(cad, sizeof(cad), __VA_ARGS__);  OutputDebugString(cad);}


#define kDataSet 3
// 0 test2
// 1 volcano
// 2 test3
// 3 test4
// 4 test6


//
// Todo:
// - Error checking!
// - Enumerate adapters
// - Get supported resolutions
// - Support full-screen
//



static f32 const g_BackgroundColour[] = {0.2f, 0.5f, 0.8f, 1.0f};
static b32 gLabelRender = false;
static u32 gLabelDistance = 1;
static b32 gLabelCoordinates = false;
static b32 gGridRender = true;

static v2 gPadding = V2(20.0f, 20.0f);


LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    switch (uMsg) {
        case WM_CLOSE: {
            OutputDebugString(L"WM_CLOSE\n");
        } break;
        
        case WM_ENTERSIZEMOVE: {
            static u64 SizeMoveCount = 0;
            DebugPrint(L"WM_ENTERSIZEMOVE, count = %lld\n", ++SizeMoveCount);
        } break;
        
        case WM_EXITSIZEMOVE: {
            static u64 ExitSizeMoveCount = 0;
            DebugPrint(L"WM_EXITSIZEMOVE, count = %lld\n", ++ExitSizeMoveCount);
        } break;
        
        case WM_SIZE: {
            OutputDebugString(L"WM_SIZE\n");
        } break;
        
        case WM_KEYDOWN: {
            if (wParam == 0x4C) // Key L
            {
                gLabelRender = !gLabelRender;
            }
            else if (wParam == 0x47) // Key G
            {
                gGridRender = !gGridRender;
            }
            else if (wParam == 0x43) // Key C
            {
                gLabelCoordinates = !gLabelCoordinates;
            }
            else if (wParam >= 0x31 && wParam <= 0x39) // 1-9
            {
                gLabelDistance = wParam - 0x31 + 1;
                //DebugPrint(L"gLabelDistance = %d\n", gLabelDistance);
            }
            else if (wParam >= 0x60 && wParam <= 0x69) // 1-9
            {
                gLabelDistance = wParam - 0x60 + 1;
                //DebugPrint(L"gLabelDistance = %d\n", gLabelDistance);
            }
        } break;
        
        case WM_DESTROY: {
            OutputDebugString(L"WM_DESTROY\n");
            PostQuitMessage(0);
            return 0;
        } break;
        
        default: {
            //DebugPrint(L"Message = %d\n", uMsg);
        } break;
    }
    
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hInstancePrev, LPSTR lpCmdLine, int nShowCmd) 
{
    //
    // Attach console
    FILE *FileStdOut;
    assert(AllocConsole());
    freopen_s(&FileStdOut, "CONOUT$", "w", stdout);
    
    
    // TODO(Marcus): Handle resolution in a proper way
    u32 const Width = 1024;
    u32 const Height = 1024;
    
    //
    // Create window
    HWND hWnd;
    {
        wchar_t const ClassName[] = L"MarchingSquares";
        
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
        WC.lpszClassName = ClassName;
        
        DWORD Style =  WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
        
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
            ClassName,                      /* Class Name */
            TEXT("Marching squares"),       /* Title */
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
#if kDataSet == 0
        char const *path = "c:\\developer\\Marching_squares\\data\\test.txt";
#elif kDataSet == 1
        char const *path = "c:\\developer\\Marching_squares\\data\\volcano.txt";
#elif kDataSet == 2
        char const *path = "c:\\developer\\Marching_squares\\data\\test3_123x61.txt";
#elif kDataSet == 3
        char const *path = "c:\\developer\\Marching_squares\\data\\test4_175x111.txt";
#elif kDataSet == 4
        char const *path = "c:\\developer\\Marching_squares\\data\\test6_11x11.txt";
#endif
        HANDLE File = CreateFileA(path,
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
            if (isspace(FileData[Index])) 
            {
                ++SpaceCount;
                while (Index < FileSize.QuadPart && isspace(FileData[Index]))
                {
                    ++Index;
                }
            }
        }
        
        DataCount = SpaceCount + 1; // We're assuming that there is only spaces in between the elements
        Data = (u32 *)malloc(DataCount * sizeof(u32));
        assert(Data);
        
        // Proccess the read data, i.e. convert to u32
        u32 DataIndex = 0;
        for (u32 Index = 0; Index < FileSize.QuadPart;)
        {
            if (isspace(FileData[Index]))
            {
                ++Index;
                continue;
            }
            
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
    MarchingSquares::config MSConfig;
#if kDataSet == 0
    std::vector<f32> Heights = {1, 2, 3, 4};
    MSConfig.CellCountX = 11;
    MSConfig.CellCountY = 11;
#elif kDataSet == 1
    std::vector<f32> Heights = {90, 100, 110, 120, 130, 140, 150, 160, 170};
    MSConfig.CellCountX = 61;
    MSConfig.CellCountY = 87;
#elif kDataSet == 2
    std::vector<f32> Heights = {10, 20, 40, 60, 80, 100, 120, 140, 160};
    MSConfig.CellCountX = 123;
    MSConfig.CellCountY = 61;
#elif kDataSet == 3
    std::vector<f32> Heights = {20, 40, 60, 80, 100, 120, 140};
    MSConfig.CellCountX = 175;
    MSConfig.CellCountY = 111;
#elif kDataSet == 4
    std::vector<f32> Heights = {0, 1, 2, 3, 4};
    MSConfig.CellCountX = 11;
    MSConfig.CellCountY = 11;
    
#endif
    {
        f32 SmallestSize = min((f32)(Width - 2*gPadding.x) / (f32)(MSConfig.CellCountX - 1), 
                               (f32)(Height -2*gPadding.y) / (f32)(MSConfig.CellCountY - 1));
        MSConfig.CellSize = V2(SmallestSize, SmallestSize);
    }
    
    MSConfig.SourceHasOriginUpperLeft = true;
    MarchingSquares MS(Data, DataCount, MSConfig);
    MS.MarchSquares(Heights);
    
    free(Data);
    Data = nullptr;
    DataCount = 0;
    
    
    
    //
    // DirectX 10
    //
    directx_state DirectXState = {};
    {
        directx_config Config;
        Config.Width = Width;
        Config.Height = Height;
        Config.hWnd = hWnd;
        
        CreateDevice(&DirectXState);
        CreateSwapChain(&DirectXState, &Config);
        CreateDepthAndStencilbuffers(&DirectXState);
        CreateRenderTargetView(&DirectXState);
        SetViewport(&DirectXState);
    }
    
    
    //
    // Vertexbuffers and indexbuffers
#if 0
    ID3D10Buffer *VertexBuffer;
    ID3D10Buffer *IndexBuffer;
    u32 IndexCount;
    
    {
        u32 TotalVertexCount = 0;
        u32 TotalIndexCount = 0;
        for (u32 Index = 0; Index < MS.GetLevelCount(); ++Index)
        {
            MarchingSquares::level *CurrLevel = MS[Index];
            TotalVertexCount += CurrLevel->Vertices.size();
            TotalIndexCount += CurrLevel->Indices.size();
        }
        
        ID3D10Device *Device = DirectXState.Device;
        
        //
        // Allocate space
        size_t Size = TotalVertexCount * sizeof(v2);
        D3D10_BUFFER_DESC BufferDesc;
        BufferDesc.ByteWidth = Size;                     // size of the buffer
        BufferDesc.Usage = D3D10_USAGE_DEFAULT;          // only usable by the GPU
        BufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER; // use in vertex shader
        BufferDesc.CPUAccessFlags = 0;                   // No CPU access to the buffer
        BufferDesc.MiscFlags = 0;                        // No other option
        
        HRESULT Result = Device->CreateBuffer(&BufferDesc, nullptr, &VertexBuffer);
        if (FAILED(Result)) 
        {
            // TODO(Marcus): Add better error handling
            OutputDebugString(L"Failed to create the Vertexbuffer\n");
            return 1;
        }
        
        Size = TotalIndexCount * sizeof(u16);
        BufferDesc.ByteWidth = Size;                     // size of the buffer
        BufferDesc.Usage = D3D10_USAGE_DEFAULT;          // only usable by the GPU
        BufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;  // use in vertex shader
        BufferDesc.CPUAccessFlags = 0;                   // No CPU access to the buffer
        BufferDesc.MiscFlags = 0;                        // No other option
        
        Result = Device->CreateBuffer(&BufferDesc, nullptr, &IndexBuffer);
        if (FAILED(Result)) 
        {
            // TODO(Marcus): Add better error handling
            OutputDebugString(L"Failed to create the Indexbuffer\n");
            return 1;
        }
        
        
        //
        // Upload data
        for (u32 Index = 0; Index < MS.GetLevelCount(); ++Index)
        {
            
        }
        
#else
        u32 BufferCount = MS.GetLevelCount();
        
        ID3D10Buffer **VertexBuffers;
        VertexBuffers = (ID3D10Buffer **)malloc(BufferCount * sizeof(ID3D10Buffer *));
        assert(VertexBuffers);
        
        ID3D10Buffer **IndexBuffers;
        IndexBuffers = (ID3D10Buffer **)malloc(BufferCount * sizeof(ID3D10Buffer *));
        assert(IndexBuffers);
        
        u32 *IndexCount;
        IndexCount = (u32 *)malloc(BufferCount * sizeof(u32));
        assert(IndexCount);
        
        {
            ID3D10Device *Device = DirectXState.Device;
            
            for (u32 Index = 0; Index < BufferCount; ++Index)
            {
                MarchingSquares::level *Level = MS[Index];
                assert(Level);
                
                //
                // Vertexbuffer
                size_t Size = Level->Vertices.size() * sizeof(v2);
                D3D10_BUFFER_DESC BufferDesc;
                BufferDesc.ByteWidth = Size;                     // size of the buffer
                BufferDesc.Usage = D3D10_USAGE_DEFAULT;          // only usable by the GPU
                BufferDesc.BindFlags = D3D10_BIND_VERTEX_BUFFER; // use in vertex shader
                BufferDesc.CPUAccessFlags = 0;                   // No CPU access to the buffer
                BufferDesc.MiscFlags = 0;                        // No other option
                
                D3D10_SUBRESOURCE_DATA InitData;
                InitData.pSysMem = Level->Vertices.data();
                InitData.SysMemPitch = 0;
                InitData.SysMemSlicePitch = 0;
                
                HRESULT Result = Device->CreateBuffer(&BufferDesc, &InitData, &VertexBuffers[Index]);
                if (FAILED(Result)) 
                {
                    // TODO(Marcus): Add better error handling
                    OutputDebugString(L"Failed to create the Vertexbuffer\n");
                    return 1;
                }
                
                
                //
                // Indexbuffer
                IndexCount[Index] = Level->Indices.size();
                
                Size = Level->Indices.size() * sizeof(u16);
                BufferDesc.ByteWidth = Size;                     // size of the buffer
                BufferDesc.Usage = D3D10_USAGE_DEFAULT;          // only usable by the GPU
                BufferDesc.BindFlags = D3D10_BIND_INDEX_BUFFER;  // use in vertex shader
                BufferDesc.CPUAccessFlags = 0;                   // No CPU access to the buffer
                BufferDesc.MiscFlags = 0;                        // No other option
                
                InitData.pSysMem = Level->Indices.data();
                InitData.SysMemPitch = 0;
                InitData.SysMemSlicePitch = 0;
                
                Result = Device->CreateBuffer(&BufferDesc, &InitData, &IndexBuffers[Index]);
                if (FAILED(Result)) 
                {
                    // TODO(Marcus): Add better error handling
                    OutputDebugString(L"Failed to create the Indexbuffer\n");
                    return 1;
                }
            }
        }
#endif
        
        
        
        //
        // Vertexbuffer, grid lines
        ID3D10Buffer *VertexBufferGridLines;
        u32 VertexCountGridLines = 0;
        {
            ID3D10Device *Device = DirectXState.Device;
            
            //
            // Generate the data
            u32 CellCountX = MS.GetCellCountX();
            u32 CellCountY = MS.GetCellCountY();
            v2 CellSize = MS.GetCellSize();
            
            VertexCountGridLines = 2 * (CellCountX + CellCountY);
            size_t Size = VertexCountGridLines * sizeof(v2);
            v2 *GridLinesVertices = (v2 *)malloc(Size);
            assert(GridLinesVertices);
            
            u32 DataIndex = 0;
            for (u32 y = 0; y < CellCountY; ++y)
            {
                GridLinesVertices[DataIndex++] = Hadamard(CellSize, V2(0, y));
                GridLinesVertices[DataIndex++] = Hadamard(CellSize, V2(CellCountX - 1, y));
            }
            
            for (u32 x = 0; x < CellCountX; ++x)
            {
                GridLinesVertices[DataIndex++] = Hadamard(CellSize, V2(x, 0));
                GridLinesVertices[DataIndex++] = Hadamard(CellSize, V2(x, CellCountY - 1));
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
            ID3D10Device *Device = DirectXState.Device;
            
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
            ID3D10Device *Device = DirectXState.Device;
            
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
            v3 Colour;
            f32 Z;
        };
        
        shader_constants ShaderConstants;
        {
            ID3D10Device *Device = DirectXState.Device;
            
            f32 Far = 1.0f;
            f32 Sx  = 2.0f / (f32)Width;
            f32 Sy  = 2.0f / (f32)Height;
            f32 Sz  = 1.0f / Far;
            
            ShaderConstants.TransformMatrix = M4Translate(-1.0f, -1.0f, 0.0f) * M4Scale(Sx, Sy, Sz);
            ShaderConstants.TransformMatrix = ShaderConstants.TransformMatrix * M4Translate(gPadding.x, gPadding.y, 0.0f);
            
            ShaderConstants.Colour = v3_one;
            ShaderConstants.Z = 0.0f;
            assert(sizeof(shader_constants) % 16 == 0);
            
            D3D10_BUFFER_DESC BufferDesc;
            BufferDesc.ByteWidth = sizeof(ShaderConstants);    
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
        // DirectWrite
        IDWriteFactory *DWriteFactory;
        ID2D1Device *D2DDevice;
        ID2D1DeviceContext *D2DDeviceContext;
        ID2D1Bitmap1 *D2DBitmap;
        ID2D1SolidColorBrush *D2DBrush;
        IDWriteTextFormat *D2DTextFormat;
        {
            HRESULT Result = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, 
                                                 __uuidof(IDWriteFactory), 
                                                 (IUnknown **)&DWriteFactory);
            if (FAILED(Result))
            {
                // TODO(Marcus): Add better error handling
                OutputDebugString(L"Failed to create the DirectWrite factory!\n");
                return 1;
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
                // TODO(Marcus): Add better error handling
                OutputDebugString(L"Failed to create the Direct2D factory!\n");
                return 1;
            }
            
            // Get the 
            IDXGIDevice *DXGIDevice;
            Result = DirectXState.Device->QueryInterface(__uuidof(IDXGIDevice), (void **)&DXGIDevice);
            if (FAILED(Result))
            {
                // TODO(Marcus): Add better error handling
                OutputDebugString(L"Failed to get the DXGIDevice from the D3D10Device1!\n");
                return 1;
            }
            
            Result = D2DFactory->CreateDevice(DXGIDevice, &D2DDevice);
            if (FAILED(Result))
            {
                // TODO(Marcus): Add better error handling
                OutputDebugString(L"Failed to create the D2DDevice!\n");
                return 1;
            }
            
            Result = D2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &D2DDeviceContext);
            if (FAILED(Result))
            {
                // TODO(Marcus): Add better error handling
                OutputDebugString(L"Failed to create the D2DDeviceContext!\n");
                return 1;
            }
            
            IDXGISurface *D2DBuffer;
            Result = DirectXState.SwapChain->GetBuffer(0, __uuidof(IDXGISurface), (void **)&D2DBuffer);
            if (FAILED(Result))
            {
                // TODO(Marcus): Add better error handling
                OutputDebugString(L"Failed to get the backbuffer as IDXGISurface!\n");
                return 1;
            }
            
            D2D1_BITMAP_PROPERTIES1 Properties;
            Properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
            Properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
            Properties.dpiX = 0;
            Properties.dpiX = 0;
            Properties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
            Properties.colorContext = nullptr;
            
            Result = D2DDeviceContext->CreateBitmapFromDxgiSurface(D2DBuffer, Properties, &D2DBitmap);
            if (FAILED(Result))
            {
                // TODO(Marcus): Add better error handling
                OutputDebugString(L"Failed to create the D2DBitmap!\n");
                return 1;
            }
            
            D2DDeviceContext->SetTarget(D2DBitmap);
            
            Result = D2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 1.0f), &D2DBrush);
            if (FAILED(Result))
            {
                // TODO(Marcus): Add better error handling
                OutputDebugString(L"Failed to create the D2DBrush!\n");
                return 1;
            }
            
            Result = DWriteFactory->CreateTextFormat(L"Microsoft New Tai Lue",
                                                     nullptr,
                                                     DWRITE_FONT_WEIGHT_NORMAL,
                                                     DWRITE_FONT_STYLE_NORMAL,
                                                     DWRITE_FONT_STRETCH_NORMAL,
                                                     96.0f/5.0f,
                                                     L"en-GB",
                                                     &D2DTextFormat);
            if (FAILED(Result))
            {
                // TODO(Marcus): Add better error handling
                OutputDebugString(L"Failed to create the D2DTextFormat!\n");
                return 1;
            }
            
            D2DTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            D2DTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
            
            
            D2DBuffer->Release();
            DXGIDevice->Release();
            D2DFactory->Release();
        }
        
        
        
        //
        // Show and update window
        ShowWindow(hWnd, nShowCmd);
        UpdateWindow(hWnd);
        
        
        
        
        
        //
        // The main loop
        u32 const stride = sizeof(v2);
        u32 const offset = 0;
        
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
            ID3D10Device *Device = DirectXState.Device;
            
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
            Device->ClearDepthStencilView(DirectXState.DepthStencilView, D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1, 0);
            static v4 const BackgroundColour = V4(0.0f, 0.0f, 0.0f, 1.0f);
            Device->ClearRenderTargetView(DirectXState.RenderTargetView, (float const *)&BackgroundColour);
            
            
            //
            // Render grid
            if (gGridRender)
            {
                ShaderConstants.Colour = V3(0.2f, 0.2f, 0.2f);
                ShaderConstants.Z = 0.3f;
                // Refresh the data in the constant buffer
                Device->UpdateSubresource(ConstantBuffer,   // Resource to update
                                          0,                // Subresource index
                                          nullptr,          // Destination box, nullptr for the entire buffer 
                                          &ShaderConstants, // Pointer to the data             
                                          0,                // Row pitch (only for textures?) 
                                          0);               // Depth pitch (only for textures?)
                Device->IASetVertexBuffers(0, 1, &VertexBufferGridLines, &stride, &offset);
                Device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
                Device->Draw(VertexCountGridLines, 0);
            }
            
            
            //
            // Render levels
            for (u32 Index = 0; Index < BufferCount; ++Index)
            {
                // Change the colour
                //u32 NewIndex = Index % ColourCount;
                //ShaderConstants.Colour = Colours[NewIndex];
                ShaderConstants.Colour = V3(1.0f, 1.0f, 1.0f);
                ShaderConstants.Z = 0.0f;
                
                // Refresh the data in the constant buffer
                Device->UpdateSubresource(ConstantBuffer,   // Resource to update
                                          0,                // Subresource index
                                          nullptr,          // Destination box, nullptr for the entire buffer 
                                          &ShaderConstants, // Pointer to the data             
                                          0,                // Row pitch (only for textures?) 
                                          0);               // Depth pitch (only for textures?)
                
                Device->IASetVertexBuffers(0, 1, &VertexBuffers[Index], &stride, &offset);
                Device->IASetIndexBuffer(IndexBuffers[Index], DXGI_FORMAT_R16_UINT, 0);
                Device->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP);
                Device->DrawIndexed(IndexCount[Index], 0, 0);
            }
            
            //
            // Render text
            if (gLabelRender)
            {
                D2DDeviceContext->BeginDraw();
                
                u32 CellX = MS.GetCellCountX();
                u32 CellY = MS.GetCellCountY();
                v2 CellSize = MS.GetCellSize();
                
                std::vector<int>::const_iterator Begin = MS.DataBegin();
                
                for (u32 y = 0; y < CellY; y += gLabelDistance)
                {
                    for (u32 x = 0; x < CellX; x += gLabelDistance)
                    {
                        WCHAR String[20];
                        if (gLabelCoordinates)
                        {
                            swprintf(String, 20, L"(%d, %d) %d", x, y, *(Begin + ((y * CellX) + x)));
                        }
                        else
                        {
                            swprintf(String, 20, L"%d", *(Begin + ((y * CellX) + x)));
                        }
                        
                        v2 P = gPadding + Hadamard(CellSize, V2(x, y));
                        P.y = Height - P.y;
                        
                        v2 Offset = V2(60.0f, 20.0f);
                        D2D1_RECT_F LayoutRect;
                        LayoutRect.top    = P.y - Offset.y;
                        LayoutRect.left   = P.x - Offset.x;
                        LayoutRect.bottom = P.y + Offset.y;
                        LayoutRect.right  = P.x + Offset.x;
                        
                        D2DDeviceContext->DrawText(String,
                                                   wcslen(String),
                                                   D2DTextFormat,
                                                   &LayoutRect,
                                                   D2DBrush,
                                                   D2D1_DRAW_TEXT_OPTIONS_NONE,
                                                   DWRITE_MEASURING_MODE_NATURAL);
                    }
                }
                
                D2D1_TAG Tag1, Tag2;
                HRESULT Result = D2DDeviceContext->EndDraw(&Tag1, &Tag2);
                if (FAILED(Result))
                {
                    OutputDebugString(L"EndDraw() failed!\n");
                }
            }
            
            //
            // Update
            DirectXState.SwapChain->Present(0, 0);
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
        D2DTextFormat->Release();
        D2DBrush->Release();
        D2DBitmap->Release();
        D2DDeviceContext->Release();
        D2DDevice->Release();
        DWriteFactory->Release();
        ConstantBuffer->Release();
        PixelShader->Release();
        InputLayout->Release();
        VertexShader->Release();
        VertexBufferGridLines->Release();
        
        for (u32 Index = 0; Index < BufferCount; ++Index)
        {
            VertexBuffers[Index]->Release();
            IndexBuffers[Index]->Release();
        }
        
        free(VertexBuffers);
        VertexBuffers = nullptr;
        
        free(IndexBuffers);
        IndexBuffers = nullptr;
        BufferCount = 0;
        
        free(IndexCount);
        IndexCount = nullptr;
        
        DirectXState.RenderTargetView->Release();
        DirectXState.DepthStencilState->Release();
        DirectXState.DepthStencilView->Release();
        DirectXState.DepthStencilBuffer->Release();
        DirectXState.RenderTargetView->Release();
        DirectXState.SwapChain->Release();
        DirectXState.Device->Release();
        
        OutputDebugString(L"***** Begin ReportLiveObjects call *****\n");
        DebugInterface->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
        OutputDebugString(L"***** End   ReportLiveObjects call *****\n\n");
        
        if (DebugInterface) {
            DebugInterface->Release();
        }
        
        FreeConsole();
        
        return msg.wParam;
    }