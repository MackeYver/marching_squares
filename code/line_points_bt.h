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

#ifndef line_points_bt__h
#define line_points_bt__h

#include <stdlib.h>
#include <stdio.h>

#include "Types.h"
#include "line_point_darray.h"

#ifdef DEBUG
#include <assert.h>
#else
#define assert(x)
#endif



struct line_points_bt_node
{
    line_points_bt_node *Left = nullptr;
    line_points_bt_node *Right = nullptr;
    line_point_darray Data;
    u32 Key;
};

typedef line_points_bt_node line_points_bt;



static line_point_darray *Insert(line_points_bt **Node, u32 Key, line_point *Data)
{
    assert(Node);
    
    if (!(*Node))
    {
        *Node = (line_points_bt *)calloc(1, sizeof(line_points_bt));
        assert(*Node);
        
        (*Node)->Key = Key;
        
        Init(&(*Node)->Data, 1);
        Push(&(*Node)->Data, Data);
        
        return &(*Node)->Data;
    }
    else if (Key == (*Node)->Key)
    {
        Push(&(*Node)->Data, Data);
        return &(*Node)->Data;
    }
    else if(Key < (*Node)->Key)
    {
        return Insert(&(*Node)->Left, Key, Data);
    }
    else
    {
        return Insert(&(*Node)->Right, Key, Data);
    }
}


static line_point_darray *Find(line_points_bt *Node, u32 Key)
{
    if (!Node)
    {
        return nullptr;
    }
    else if (Key == Node->Key)
    {
        return &Node->Data;
    }
    else if(Key < Node->Key)
    {
        return Find(Node->Left, Key);
    }
    else
    {
        return Find(Node->Right, Key);
    }
}


static void Free(line_points_bt **Node)
{
    if (!(*Node))
    {
        return;
    }
    
    Free(&(*Node)->Left);
    Free(&(*Node)->Right);
    
    Free(&(*Node)->Data);
    free(*Node);
    *Node = nullptr;
}


#endif