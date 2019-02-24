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

#ifndef f32_darray__h
#define f32_darray__h

#ifdef DEBUG
#include <assert.h>
#else
#define assert(x)
#endif



//
// Structs
//
struct f32_darray
{
    f32*Data = nullptr;
    u32 Capacity = 0;
    u32 Used = 0;
};



//
// Declarations
//
static void Init(f32_darray *Array, u32 Capacity);
static void Push(f32_darray *Array, f32 Data);
static void Clear(f32_darray *Array);
static void Free(f32_darray *Array);




//
// Definitions
//
static void Init(f32_darray *Array, u32 Capacity)
{
    assert(Array);
    
    if (Array->Data)
    {
        Free(Array);
    }
    
    Array->Used = 0;
    Array->Capacity = Capacity;
    Array->Data = (f32 *)malloc(Array->Capacity * sizeof(f32));
    assert(Array->Data);
}


static void Grow(f32_darray *Array)
{
    assert(Array);
    assert(Array->Data);
    
    u32 NewCapacity = Array->Capacity == 0 ? 1 : 2*Array->Capacity;
    f32 *NewPtr = (f32 *)realloc(Array->Data, NewCapacity * sizeof(f32));
    
    if (NewPtr)
    {
        Array->Data = NewPtr; // realloc will free the old pointer if NewPtr is at another location in mermory.
        Array->Capacity = NewCapacity;
    }
    else
    {
        // Note: we will probably never hit this case, not sure of how to handle it.
        free(Array->Data);
        assert(0); // Failed to grow!
    }
}


static void Push(f32_darray *Array, f32 Data)
{
    assert(Array);
    
    if (Array->Used + 1 > Array->Capacity)
    {
        Grow(Array);
    }
    Array->Data[Array->Used++] = Data;
}


static void Free(f32_darray *Array)
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


static void Clear(f32_darray *Array)
{
    assert(Array);
    Array->Used = 0;
}

#endif