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

#ifndef c_style__h
#define c_style__h

#include "Timing.h"
#include "Mathematics.h"
#include "v2_darray.h"
#include "u16_darray.h"

struct f32_darray;



struct c_style_state
{
    v2_darray Vertices;
    u16_darray Indices;  
    
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
    
    u32 CellCountX = 0;
    u32 CellCountY = 0;
    v2 CellSize = v2_zero;
};

//
// Note: Heights is a stretchy buffer!
b32 MarchSquares(c_style_state *State, u32 const *DataPtr, f32_darray const *Heights);
void Free(c_style_state *State);

#endif