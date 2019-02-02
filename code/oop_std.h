//
//  An implementation of the algorithm Marching squares,
//  using OOP and the c standard library
//
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



#ifndef Marching_squares_h
#define Marching_squares_h

#include <map>
#include <vector>

#include "Mathematics.h"
#include "Timing.h"



//
// Base class
class MarchingSquares {
    public:
    
    //
    // Types
    struct line_segment {
        v2 P[2];
        b32 IsProcessed;
    };
    
    struct line_point
    {
        v2 P;
        u32 LineIndex;
    };
    
    struct config {
        v2  CellSize;
        u32 CellCountX;
        u32 CellCountY;
    };
    
    enum result {
        Ok = 0,
        NoData,
        InvalidCellCountX,
        InvalidCellCountY,
        InvalidCellSize,
        InvalidRenderer,
        InvalidLevelHeight,
        NoLineSegments,
    };
    
    
    //
    // Constructor
    MarchingSquares();
    
    
    //
    // Destructor
    // The synthesized destructor will be fine, the memory are managed by std::vector which will 
    // free the allocated memory in their destructors.
    
    
    //
    // Setters and getters
    void SetDataPtr(std::vector<u32> *Heights, config const *Config);
    void SetDataPtr(u32 *Heights, config const *Config);
    
    void CopyData(std::vector<u32> *Heights, config const *Config);
    void CopyData(u32 *Heights, u32 HeightsCount, config const *Config);
    
    
    u32 GetCellCountX() const                    {return CellCountX;}
    u32 GetCellCountY() const                    {return CellCountY;}
    v2  GetCellSize()   const                    {return CellSize;}
    
    std::vector<v2>  *GetVertexData()            {return &Vertices;}
    std::vector<u16> *GetIndexData()             {return &Indices;}
    
    std::vector<v2>::size_type GetVertexCount()  {return Vertices.size();}
    std::vector<v2>::size_type GetIndexCount()   {return Indices.size();}
    
    u32 *DataBegin()                             {return DataPtr;}
    
    //
    // Run the algorithm
    result MarchSquares(std::vector<f32> const &LevelHeights);
    
    
    //
    // @debug
    union
    {
        struct
        {
            time_measure TTotal;
            
            time_measure TMarching;
            time_measure TBinarySum;
            time_measure TLerp;
            time_measure TAdd;
            
            time_measure TSimplify;
            time_measure TGetLineChain;
            time_measure TMergeLines;
        };
        
        time_measure Measures[8]; 
    };
    
    void ClearTimeMeasures()
    {
        ZeroMemory(Measures, 8 * sizeof(time_measure));
    }
    
    
    //
    // Data
    protected:
    void SetConfig(config const *Config);
    
    std::vector<v2> Vertices;
    std::vector<u16> Indices; // 0xFFFF is used as primitive restart index
    
    std::vector<u32> Data;
    u32 *DataPtr;
    
    v2 CellSize;
    
    u32 CellCountX = 0;
    u32 CellCountY = 0;
};
#endif /* Marching_squares_h */
