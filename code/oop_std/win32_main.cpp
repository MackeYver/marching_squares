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

#include <stdio.h>
#include <cctype>

#include "..\DirectXRenderer.h"
#include "..\Types.h"
#include "..\Mathematics.h"

#include "oop_std.h"

#ifdef DEBUG
#define DebugPrint(...) {wchar_t cad[512]; swprintf_s(cad, sizeof(cad), __VA_ARGS__);  OutputDebugString(cad);}
//#include <assert.h>
#else
#define DebugPrint(...)
//#define assert(x)
#endif

#define SavePerfToFile


//
// Todo:
// - Error checking!
// - Enumerate adapters
// - Get supported resolutions
// - Support full-screen
//



//
// Globals
//
static f32 const gBackgroundColour[] = {0.2f, 0.5f, 0.8f, 1.0f};
static b32 gLabelRender = false;
static u32 gLabelDistance = 1;
static b32 gLabelCoordinates = false;
static b32 gGridRender = true;
static u32 gRenderIndex = 0;
static u32 gDataCount = 0;

static v2 gPadding = V2(20.0f, 20.0f);



//
// File reading using the win32 API.
//
b32 ReadHeightsFromFileWin32(const char *PathAndName, u32 **Data, u32 *DataSize)
{
    assert(PathAndName);
    assert(Data);
    assert(DataSize);
    
    HANDLE File = CreateFileA(PathAndName,
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
        return Error;
    }
    
    LARGE_INTEGER FileSize;
    GetFileSizeEx(File, &FileSize);
    assert(FileSize.QuadPart > 0);
    
    char *FileData = (char *)malloc((size_t)FileSize.QuadPart);
    assert(FileData);
    
    DWORD BytesRead = 0;
    if (!ReadFile(File, FileData, (DWORD)FileSize.QuadPart, &BytesRead, nullptr) || BytesRead != FileSize.QuadPart)
    {
        if (FileData)  free(FileData);
        
        CloseHandle(File);
        DWORD Error = GetLastError();
        DebugPrint(L"Failed to read from file, error: %d", Error);
        return Error;
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
    
    *DataSize = SpaceCount + 1; // We're assuming that there is only spaces in between the elements
    *Data = (u32 *)malloc(*DataSize * sizeof(u32));
    assert(*Data);
    
    // Proccess the read data, i.e. convert to u32
    u32 DataIndex = 0;
    for (u32 Index = 0; Index < FileSize.QuadPart;)
    {
        if (isspace(FileData[Index]))
        {
            ++Index;
            continue;
        }
        
        char SmallBuffer[20] = {};
        
        u32 EndIndex = Index;
        while (EndIndex < FileSize.QuadPart && FileData[EndIndex] != ' ' && FileData[EndIndex] != '\n') {
            ++EndIndex;
        }
        
        assert(EndIndex != Index);
        assert(EndIndex - Index < 20);
        
        memcpy(SmallBuffer, &FileData[Index], EndIndex - Index);
        sscanf_s(SmallBuffer, "%d", &(*Data)[DataIndex++]);
        
        Index = EndIndex + 1;
    }
    assert(DataIndex == *DataSize);
    
    free(FileData);
    
    return true;
}




//
// Call the init code in the DirectX file
//
void SetupDirectX(directx_state *State, directx_config *Config)
{
    assert(State);
    
    CreateDevice(State);
    CreateSwapChain(State, Config);
    CreateDepthAndStencilbuffers(State);
    CreateRenderTargetView(State);
    SetViewport(State);
    
    //
    // Vertex shader
    CreateShader(State, "..\\shaders\\BasicVertexShader.cso", &State->VertexShader);
    UseShader(State, &State->VertexShader);
    
    //
    // Pixel shader
    CreateShader(State, "..\\shaders\\BasicPixelShader.cso", &State->PixelShader);
    UseShader(State, &State->PixelShader);
}



//
// Create buffers from the generated lines, also generate grid lines
// 
b32 SetupBuffers(directx_state *State,
                 void *VertexDataPtr, u32 VertexCount,
                 void *IndexDataPtr, u32 IndexCount,
                 directx_renderable_indexed *ContourLines)
{
    //
    // Contour lines
    b32 Result = CreateRenderable(State,
                                  ContourLines,
                                  VertexDataPtr, sizeof(v2) , VertexCount,
                                  IndexDataPtr , sizeof(u16), IndexCount,
                                  D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP);
    
    return Result;
}


b32 SetupGridLines(directx_state *State,
                   u32 CellCountX, u32 CellCountY, v2 CellSize,
                   directx_renderable *GridLines)
{
    //
    // Gridlines
    u32 VertexCountGridLines = 2 * (CellCountX + CellCountY);
    size_t Size = VertexCountGridLines * sizeof(v2);
    v2 *GridLineVertices = (v2 *)malloc(Size);
    assert(GridLineVertices);
    
    u32 DataIndex = 0;
    for (u32 y = 0; y < CellCountY; ++y)
    {
        GridLineVertices[DataIndex++] = Hadamard(CellSize, V2(0, y));
        GridLineVertices[DataIndex++] = Hadamard(CellSize, V2(CellCountX - 1, y));
    }
    
    for (u32 x = 0; x < CellCountX; ++x)
    {
        GridLineVertices[DataIndex++] = Hadamard(CellSize, V2(x, 0));
        GridLineVertices[DataIndex++] = Hadamard(CellSize, V2(x, CellCountY - 1));
    }
    assert(DataIndex == VertexCountGridLines);
    
    CreateRenderable(State, GridLines, 
                     GridLineVertices, sizeof(v2), VertexCountGridLines,
                     D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
    
    free(GridLineVertices);
    
    return true;
}




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
            else if (wParam == 0x49) // Key I
            {
                gRenderIndex = ++gRenderIndex % gDataCount;
            }
            else if (wParam == 0x43) // Key C
            {
                gLabelCoordinates = !gLabelCoordinates;
            }
            else if (wParam >= 0x31 && wParam <= 0x39) // 1-9
            {
                gLabelDistance = wParam - 0x31 + 1;
            }
            else if (wParam >= 0x60 && wParam <= 0x69) // 1-9
            {
                gLabelDistance = wParam - 0x60 + 1;
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
    // Create and attach console
    //
    {
        FILE *FileStdOut;
        b32 Result = AllocConsole();
        assert(Result);
        freopen_s(&FileStdOut, "CONOUT$", "w", stdout);
        Result;
    }
    
    
    // TODO(Marcus): Handle resolution in a proper way
    u32 const Width = 1024;
    u32 const Height = 1024;
    
    
    //
    // Create window
    //
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
    // Init DirectX 10
    //
    directx_state DirectXState = {};
    {
        directx_config Config;
        Config.Width = Width;
        Config.Height = Height;
        Config.hWnd = hWnd;
        
        SetupDirectX(&DirectXState, &Config);
    }
    
    
    
    //
    // Prepare some storage for the data
    //
    char const *AllPaths[] = 
    {
        "..\\..\\data\\volcano.txt",
        "..\\..\\data\\test3_123x61.txt",
        "..\\..\\data\\test4_175x111.txt",
        "..\\..\\data\\data_200x200.txt",
        "..\\..\\data\\data_1000x1000.txt",
    };
    
    std::vector<f32> Heights[] =
    {
        {90, 100, 110, 120, 130, 140, 150, 160, 170},
        {20, 40, 60, 80, 100, 120, 140, 160},
        {20, 40, 60, 80, 100, 120, 140},
        {40, 60, 80, 100, 120, 140, 160, 180, 200, 220, 240, 260, 280, 300, 320, 340, 360},
        {2, 3, 4, 5, 6, 7, 8, 9, 10},// 11, 12, 13, 14},
    };
    
    u32 CellCounts[] =
    {
        61, 87,
        123, 61,
        175, 111,
        200, 200,
        1000, 1000,
    };
    
    gDataCount = sizeof(AllPaths) / sizeof(*AllPaths);
    
    v2 *CellSizes = (v2 *)malloc(gDataCount * sizeof(v2));
    assert(CellSizes);
    
    u32 **Data = nullptr;
    Data = (u32 **)malloc(gDataCount * sizeof(u32));
    assert(Data);
    
    u32 *DataSize = 0;
    DataSize = (u32 *)malloc(gDataCount * sizeof(u32));
    assert(DataSize);
    
    
    
    //
    // Read heights from text file
    //
    for (u32 Index = 0; Index < gDataCount; ++Index)
    {
        b32 Result = ReadHeightsFromFileWin32(AllPaths[Index], &Data[Index], &DataSize[Index]);
        assert(Result);
        Result;
    }
    
    
#ifdef SavePerfToFile
    //
    // Open/create text files used as output for performance data
    //
    FILE *FilePerf = nullptr;
    CreateFile(&FilePerf, "..\\..\\perf\\perf_oop_std.txt");
    WritePerfHeadersToFile(FilePerf);
    
    FILE *FileData = nullptr;
    CreateFile(&FileData, "..\\..\\perf\\data_oop_std.txt");
    WriteDataHeadersToFile(FileData);
#endif
    
    
    
    //
    // Memory for the renderables
    // 
    directx_renderable *GridLines;
    GridLines = (directx_renderable *)calloc(gDataCount, sizeof(directx_renderable));
    assert(GridLines);
    
    directx_renderable_indexed *ContourLines;
    ContourLines = (directx_renderable_indexed *)calloc(gDataCount, sizeof(directx_renderable_indexed));
    assert(ContourLines);
    
    
    
    ///////////////////////////////////////////////////////////////////////////////////////
    //
    // OOP version
    // - Generate lines from data using Marching Squares, OOP version
    // - Note: we're also generating the vertex- and index-buffers here.
    //
    {
        MarchingSquares MS;
        MarchingSquares::config MSConfig;
        
        LARGE_INTEGER Frequency;
        QueryPerformanceFrequency(&Frequency);
        
        for (u32 Index = 0; Index < gDataCount; ++Index)
        {
            MSConfig.CellCountX = CellCounts[2 * Index];
            MSConfig.CellCountY = CellCounts[2 * Index + 1];
            
            f32 SmallestSize = min((f32)(Width - 2*gPadding.x) / (f32)(MSConfig.CellCountX - 1), 
                                   (f32)(Height -2*gPadding.y) / (f32)(MSConfig.CellCountY - 1));
            MSConfig.CellSize = V2(SmallestSize, SmallestSize);
            CellSizes[Index] = MSConfig.CellSize;
            
            ClearTimeMeasurements(&MS.Measures);
            MS.SetDataPtr(Data[Index], &MSConfig);
            
            int Result = MS.MarchSquares(Heights[Index]);
            assert(Result == MarchingSquares::Ok);
            
#ifdef SavePerfToFile
            u32 Count = sizeof(time_measurements) / sizeof(time_measure);
            for (u32 MeasureIndex = 0; MeasureIndex < Count; ++MeasureIndex)
            {
                WritePerfToFile(FilePerf, "oop_std", OPTIMIZATION, Index, 0, 
                                TimingParentNames[MeasureIndex], 
                                TimingNames[MeasureIndex], 
                                MS.Measures.Array[MeasureIndex].TotalCycleCount,
                                MS.Measures.Array[MeasureIndex].Count);
            }
            
            //
            // Write info about data reduction
            WriteDataToFile(FileData, "oop_std", OPTIMIZATION, Index, 0,
                            2*MS.LineCountAnte,
                            MS.GetVertexCount());
#endif
            
            {
                //
                // Buffers, create them from this version (buffer creation not included in the 
                // perfomance measurement).
                Result = SetupBuffers(&DirectXState, 
                                      MS.GetVertexData()->data(), MS.GetVertexCount(),
                                      MS.GetIndexData()->data() , MS.GetIndexCount(),
                                      &ContourLines[Index]);
                assert(Result);
                
                Result = SetupGridLines(&DirectXState, 
                                        MS.GetCellCountX(), MS.GetCellCountY(), MS.GetCellSize(), 
                                        &GridLines[Index]);
                assert(Result);
            }
        }
    }
    
    
    
    //
    // Constant buffer, used by the shaders
    //
    struct shader_constants
    {
        m4 TransformMatrix;
        v3 Colour;
        f32 Z;
    };
    
    directx_buffer ConstantBuffer;
    shader_constants ShaderConstants;
    {
        f32 Far = 1.0f;
        f32 Sx  = 2.0f / (f32)Width;
        f32 Sy  = 2.0f / (f32)Height;
        f32 Sz  = 1.0f / Far;
        
        ShaderConstants.TransformMatrix = M4Translate(-1.0f, -1.0f, 0.0f) * M4Scale(Sx, Sy, Sz);
        ShaderConstants.TransformMatrix = ShaderConstants.TransformMatrix * M4Translate(gPadding.x, gPadding.y, 0.0f);
        
        ShaderConstants.Colour = v3_one;
        ShaderConstants.Z = 0.0f;
        assert(sizeof(shader_constants) % 16 == 0);
        
        b32 Result = CreateConstantBuffer(&DirectXState, &ShaderConstants, sizeof(shader_constants), &ConstantBuffer);
        assert(Result);
        UseConstantBuffer(&DirectXState, &ConstantBuffer);
        
        Result;
    }
    
    
    
    //
    // DirectWrite
    //
    directwrite_state DirectWriteState;
    {
        InitDirectWrite(&DirectXState, &DirectWriteState);
    }
    
    
    
    //
    // Show and update window
    // Note: we're waiting until here with displaying the window, this will maybe avoid a blank
    //       and unresponsive window showing for a while (while the program is calculating).
    //
    ShowWindow(hWnd, nShowCmd);
    UpdateWindow(hWnd);
    
    
    
    //
    // The main loop
    //
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
        Device->ClearRenderTargetView(DirectXState.RenderTargetView, (float const *)&gBackgroundColour);
        
        
        //
        // Render grid
        if (gGridRender)
        {
            ShaderConstants.Colour = V3(0.2f, 0.2f, 0.2f);
            ShaderConstants.Z = 0.3f;
            UpdateBuffer(&DirectXState, &ConstantBuffer, &ShaderConstants);
            
            RenderRenderable(&DirectXState, &GridLines[gRenderIndex]);
        }
        
        
        //
        // Render levels
        {
            ShaderConstants.Colour = V3(1.0f, 1.0f, 1.0f);
            ShaderConstants.Z = 0.0f;
            UpdateBuffer(&DirectXState, &ConstantBuffer, &ShaderConstants);
            
            RenderRenderable(&DirectXState, &ContourLines[gRenderIndex]);
        }
        
        
        //
        // Render text
        BeginDraw(&DirectWriteState);
        WCHAR String[20];
        swprintf(String, 20, L"RenderIndex = %d", gRenderIndex);
        DrawText(&DirectWriteState, String, V2(100.0f, 0.0f));
        
        if (gLabelRender)
        {
            u32 CellX = CellCounts[2 * gRenderIndex];
            u32 CellY = CellCounts[2 * gRenderIndex + 1];
            v2 CellSize = CellSizes[gRenderIndex];
            
            u32 *Begin = Data[gRenderIndex];
            
            for (u32 y = 0; y < CellY; y += gLabelDistance)
            {
                for (u32 x = 0; x < CellX; x += gLabelDistance)
                {
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
                    DrawText(&DirectWriteState, String, P);
                    
                }
            }
        }
        
        HRESULT Result = EndDraw(&DirectWriteState);
        if (FAILED(Result))
        {
            OutputDebugString(L"EndDraw() failed!\n");
        }
        
        
        //
        // Update
        DirectXState.SwapChain->Present(0, 0);
    }
    
    
    
    //
    // @debug
#ifdef DEBUG
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
#endif
    
    
    
    //
    // Clean up
    //
    if (DataSize)
    {
        free(DataSize);
    }
    
    if (CellSizes)
    {
        free(CellSizes);
    }
    
    if (Data)
    {
        for (u32 Index = 0; Index < gDataCount; ++Index)
        {
            if (Data[Index])
            {
                free(Data[Index]);
            }
        }
        free(Data);
    }
    
    FreeConsole();
    
    ReleaseDirectWrite(&DirectWriteState);
    
    ReleaseBuffer(&ConstantBuffer);
    
    if (GridLines)
    {
        for (u32 Index = 0; Index < gDataCount; ++Index)
        {
            ReleaseRenderable(&GridLines[Index]);
        }
        free(GridLines);
    }
    
    if (ContourLines)
    {
        for (u32 Index = 0; Index < gDataCount; ++Index)
        {
            ReleaseRenderable(&ContourLines[Index]);
        }
        free(ContourLines);
    }
    
    ReleaseDirectXState(&DirectXState);
    
#ifdef DEBUG
    OutputDebugString(L"***** Begin ReportLiveObjects call *****\n");
    DebugInterface->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
    OutputDebugString(L"***** End   ReportLiveObjects call *****\n\n");
    
    if (DebugInterface) {
        DebugInterface->Release();
    }
#endif
    
    return msg.wParam;
}