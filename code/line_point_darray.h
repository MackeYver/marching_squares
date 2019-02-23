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

#ifndef line_point_darray__h
#define line_point_darray__h

#include "Mathematics.h"
#include <assert.h>
#include <stdlib.h>



//
// Structs
// 
struct line_point
{
    v2 P;
    u32 LineIndex;
};

struct line_point_darray
{
    line_point *Data = nullptr;
    u32 Capacity = 0;
    u32 Used = 0;
};



//
// Declarations
//
static void Init(line_point_darray *Array, u32 Capacity = 0);
static void Grow(line_point_darray *Array);
static void Push(line_point_darray *Array, line_point *LinePoint);
static void Free(line_point_darray *Array);
static void Clear(line_point_darray *Array);



//
// Definitions
//
static void Init(line_point_darray *Array, u32 Capacity)
{
    assert(Array);
    assert(!Array->Data);
    
    Array->Capacity = Capacity;
    Array->Used = 0;
    Array->Data = (line_point *)malloc(Array->Capacity * sizeof(line_point));
    assert(Array->Data);
}

static void Grow(line_point_darray *Array)
{
    assert(Array);
    assert(Array->Data);
    
    u32 NewCapacity = Array->Capacity == 0 ? 1 : 2*Array->Capacity;
    line_point *NewPtr = (line_point *)realloc(Array->Data, NewCapacity * sizeof(line_point));
    
    if (NewPtr)
    {
        Array->Data = NewPtr; // realloc will free the old pointer if NewPtr is at another location in memory.
        Array->Capacity = NewCapacity;
    }
    else
    {
        // Note: we will probably never hit this case, not sure of how to handle it.
        free(Array->Data);
        assert(0); // Failed to grow!
    }
}

static void Push(line_point_darray *Array, line_point *LinePoint)
{
    assert(Array);
    assert(LinePoint);
    
    if (Array->Used + 1 > Array->Capacity)
    {
        Grow(Array);
    }
    Array->Data[Array->Used++] = *LinePoint;
}


static void Free(line_point_darray *Array)
{
    if (Array)
    {
        if (Array->Data)
        {
            free(Array->Data);
            Array->Data = nullptr;
        }
        Array->Used = 0;
        Array->Capacity = 0;
    }
}


static void Clear(line_point_darray *Array)
{
    assert(Array);
    Array->Used = 0;
}

#endif