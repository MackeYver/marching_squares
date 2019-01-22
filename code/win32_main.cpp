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
#include "oop_timer.h"

#include "std.h"


#include <stdio.h>
#define DebugPrint(...) {wchar_t cad[512]; swprintf_s(cad, sizeof(cad), __VA_ARGS__);  OutputDebugString(cad);}



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
        return false;
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
        return false;
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
        
        char SmallBuffer[10] = {};
        
        u32 EndIndex = Index;
        while (EndIndex < FileSize.QuadPart && FileData[EndIndex] != ' ' && FileData[EndIndex] != '\n') {
            ++EndIndex;
        }
        
        assert(EndIndex != Index);
        assert(EndIndex - Index < 10);
        
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
    CreateShader(State, "shaders\\BasicVertexShader.cso", &State->VertexShader);
    UseShader(State, &State->VertexShader);
    
    //
    // Pixel shader
    CreateShader(State, "shaders\\BasicPixelShader.cso", &State->PixelShader);
    UseShader(State, &State->PixelShader);
}



//
// Create buffers from the generated lines, also generate grid lines
// 
b32 SetupBuffers(directx_state *State,
                 MarchingSquares *MS,
                 directx_renderable_indexed *ContourLines, 
                 directx_renderable *GridLines)
{
    assert(ContourLines);
    assert(GridLines);
    
    //
    // Contour lines
    b32 Result = CreateRenderable(State,
                                  ContourLines,
                                  MS->GetVertexData()->data(), sizeof(v2) , MS->GetVertexCount(),
                                  MS->GetIndexData()->data() , sizeof(u16), MS->GetIndexCount(),
                                  D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP);
    assert(Result);
    
    
    //
    // Gridlines
    u32 CellCountX = MS->GetCellCountX();
    u32 CellCountY = MS->GetCellCountY();
    v2  CellSize = MS->GetCellSize();
    
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
    // Creata and attach console
    //
    FILE *FileStdOut;
    assert(AllocConsole());
    freopen_s(&FileStdOut, "CONOUT$", "w", stdout);
    
    
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
        "c:\\developer\\Marching_squares\\data\\test.txt",
        "c:\\developer\\Marching_squares\\data\\volcano.txt",
        "c:\\developer\\Marching_squares\\data\\test3_123x61.txt",
        "c:\\developer\\Marching_squares\\data\\test4_175x111.txt",
        "c:\\developer\\Marching_squares\\data\\test6_11x11.txt"
    };
    
    std::vector<f32> Heights[] =
    {
        {1, 2, 3, 4},
        {90, 100, 110, 120, 130, 140, 150, 160, 170},
        {10, 20, 40, 60, 80, 100, 120, 140, 160},
        {20, 40, 60, 80, 100, 120, 140},
        {0, 1, 2, 3, 4}
    };
    
    u32 CellCounts[] =
    {
        11, 11,
        61, 87,
        123, 61,
        175, 111,
        11, 11
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
    
    u32 *CompareVertexCount = (u32 *)calloc(gDataCount, sizeof(u32));
    assert(CompareVertexCount);
    
    u32 *CompareIndexCount = (u32 *)calloc(gDataCount, sizeof(u32));
    assert(CompareIndexCount);
    
    
    //
    // Read heights from text file
    //
    for (u32 Index = 0; Index < gDataCount; ++Index)
    {
        b32 Result = ReadHeightsFromFileWin32(AllPaths[Index], &Data[Index], &DataSize[Index]);
        assert(Result);
    }
    
    
    
    
    ///////////////////////////////////////////////////////////////////////////////////////
    //
    // OOP version
    // - Generate lines from data using Marching Squares, OOP version
    // - Note: we're also generating the vertex- and index-buffers here.
    //
    directx_renderable *GridLines;
    GridLines = (directx_renderable *)calloc(gDataCount, sizeof(directx_renderable));
    assert(GridLines);
    
    directx_renderable_indexed *ContourLines;
    ContourLines = (directx_renderable_indexed *)calloc(gDataCount, sizeof(directx_renderable_indexed));
    assert(ContourLines);
    {
        oop_timer *Timer = new oop_timer();
        assert(Timer);
        
        MarchingSquares MS(Timer);
        MarchingSquares::config MSConfig;
        
        for (u32 Index = 0; Index < gDataCount; ++Index)
        {
            MSConfig.CellCountX = CellCounts[2 * Index];
            MSConfig.CellCountY = CellCounts[2 * Index + 1];
            
            f32 SmallestSize = min((f32)(Width - 2*gPadding.x) / (f32)(MSConfig.CellCountX - 1), 
                                   (f32)(Height -2*gPadding.y) / (f32)(MSConfig.CellCountY - 1));
            MSConfig.CellSize = V2(SmallestSize, SmallestSize);
            CellSizes[Index] = MSConfig.CellSize;
            
            MSConfig.SourceHasOriginUpperLeft = true;
            
            MS.SetDataPtr(Data[Index], &MSConfig);
            int Result = MS.MarchSquares(Heights[Index]);
            assert(Result == MarchingSquares::Ok);
            
            // Store the vertex and the index count, this will be used for comparing the different solutions
            // with each other (in other words, we're assuming that this solution is correct).
            CompareVertexCount[Index] = MS.GetVertexCount();
            CompareIndexCount[Index]  = MS.GetIndexCount();
            
            
            //
            // Buffers, create them from this version (buffer creation not included in the perfomance measurement).
            //
            SetupBuffers(&DirectXState, &MS, &ContourLines[Index], &GridLines[Index]);
        }
        
        printf("\n************** START, OOP, std **************\n");
        Timer->PrintTimesCSV();
        printf("************** END,   OOP, std **************\n\n");
        delete Timer;
    }
    
    
    
    
    ///////////////////////////////////////////////////////////////////////////////////////
    //
    // Non-OOP but std version
    // - Generate lines from data using Marching Squares, non-OOP but using standard library version
    //
    {
        oop_timer *Timer = new oop_timer();
        assert(Timer);
        
        std_state State;
        
        for (u32 Index = 0; Index < gDataCount; ++Index)
        {
            State.CellSize = CellSizes[Index];
            State.CellCountX = CellCounts[2 * Index];
            State.CellCountY = CellCounts[2 * Index + 1];
            State.DataPtr = Data[Index];
            
            // TODO(Marcus): hey, are we handling this correctly? MSConfig.SourceHasOriginUpperLeft = true;
            
            b32 Result = MarchSquares(&State, Heights[Index], Timer);
            assert(Result);
            
            // Let's compare the result againts the std-OOP version.
            assert(State.Vertices.size() == CompareVertexCount[Index]);
            assert(State.Indices.size()  == CompareIndexCount[Index]);
        }
        
        printf("\n************ START, Non-OOP, std ************\n");
        Timer->PrintTimesCSV();
        printf("************ END,   Non-OOP, std ************\n\n");
        delete Timer;
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
        
        assert(CreateConstantBuffer(&DirectXState, &ShaderConstants, sizeof(shader_constants), &ConstantBuffer));
        UseConstantBuffer(&DirectXState, &ConstantBuffer);
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
    u32 const stride = sizeof(v2);
    u32 const offset = 0;
    
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
    ReleaseDirectWrite(&DirectWriteState);
    ReleaseBuffer(&ConstantBuffer);
    ReleaseDirectXState(&DirectXState);
    
#ifdef DEBUG
    OutputDebugString(L"***** Begin ReportLiveObjects call *****\n");
    DebugInterface->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
    OutputDebugString(L"***** End   ReportLiveObjects call *****\n\n");
    
    if (DebugInterface) {
        DebugInterface->Release();
    }
#endif
    
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
    
    if (Data)
    {
        for (u32 Index = 0; Index < gDataCount; ++Index)
        {
            if (Data[Index])  free(Data[Index]);
        }
    }
    
    free(Data);
    
    if (DataSize)            free(DataSize);
    if (CellSizes)           free(CellSizes);
    if (CompareVertexCount)  free(CompareVertexCount);
    if (CompareIndexCount)   free(CompareIndexCount);
    
    FreeConsole();
    
    return msg.wParam;
}