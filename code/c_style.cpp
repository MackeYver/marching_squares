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

#include "c_style.h"
#include "line_point_darrays.h"
#include "line_segment_darray.h"
#include "f32_darray.h"
#include "u16_darray.h"
#include "v2_darray.h"
#include "line_points_bt.h"

#if 0
#ifdef DEBUG
#include <assert.h>
#else
#define assert(x)
#endif
#endif



//
// Utility functions
//
inline u32 CalculateKey(v2 const& P)
{
    // 3847 is a prime, choosen due to it being a bit larger than 3840 which is the width of 4k display.
    // Admittedly a bit arbitrary, but we need to generate a key to use for sorting somehow and this may
    // be good enough.
    return 3847*(u32)P.y + (u32)P.x;
}



inline void Add(line_segment_darray *LineSegments, 
                line_points_bt **Points, 
                v2 Po, // Origo (or offset, depends on how you look at it I guess...)
                v2 P0, // First point of the line
                v2 P1) // Second point of the line
{
    P0 += Po;
    P1 += Po;
    
    u32 Index = LineSegments->Used;
    Push(LineSegments, P0, P1);
    
    line_point LP;
    LP.P = P0;
    LP.LineIndex = Index;
    u32 Key = CalculateKey(LP.P);
    line_point_darray *DArray = Insert(Points, Key, &LP);
    
    printf("%f, %f, %u\n", LP.P.x, LP.P.y, LP.LineIndex);
    
    LP.P = P1;
    LP.LineIndex = Index;
    Key = CalculateKey(LP.P);
    DArray = Insert(Points, Key, &LP);
    
    printf("%f, %f, %u\n", LP.P.x, LP.P.y, LP.LineIndex);
}


/** Linear interpolation between two heights */
inline f32 Lerp(f32 Length, f32 H0, f32 H1, f32 CurrentHeight) {
    f32 Result = 0.0f;
    
    f32 Numerator = CurrentHeight - H0;
    f32 Denominator = H1 - H0;
    // @note The denominator can be zero, no worries, according to IEEE 754 this
    //       will give +INF/-INF as a result but will not crash the program.
    //       We know that this will not happen in the cases we are interested in.
    f32 Factor = Numerator / Denominator;
    Result = Factor * Length;
    
    return Result;
}

/** @desc Utility function, linear interpolation between two heights */
inline f32 Lerp(f32 Length, u32 H0, u32 H1, f32 CurrentHeight) {
    f32 Result = Lerp(Length, (f32)H0, (f32)H1, CurrentHeight);
    return Result;
}



line_segment *GetNextLineSegment(line_segment_darray *LineSegments, 
                                 line_points_bt *LinePoints,
                                 line_segment *CurrLine,
                                 u32 DirectionIndex,
                                 time_measurements *M)
{
    assert(LineSegments);
    assert(LinePoints);
    assert(CurrLine);
    
    StartMeasure(&M->CalculateKey);
    u32 Key = CalculateKey(CurrLine->P[DirectionIndex]);
    StopMeasure(&M->CalculateKey);
    
    StartMeasure(&M->Find);
    line_point_darray *Points = Find(LinePoints, Key);
    StopMeasure(&M->Find);
    
    if (!Points)
    {
        return nullptr;
    }
    
    StartMeasure(&M->Loop);
    for (u32 Index = 0; Index < Points->Used; ++Index)
    {
        line_point *Point = &Points->Data[Index];
        assert(Point->LineIndex < LineSegments->Used);
        
        line_segment *NextLine = &LineSegments->Data[Point->LineIndex];
        if (NextLine != CurrLine && !NextLine->IsProcessed)
        {
            return NextLine;
        }
    }
    StopMeasure(&M->Loop);
    
    return nullptr; // No connected lines in the direction of the given index!
}



void GetLineChain(line_segment_darray *LineSegments, 
                  line_points_bt *LinePoints,
                  line_segment *LineInChain, 
                  line_segment_darray *Chain,
                  time_measurements *M)
{
    assert(LineSegments);
    assert(LinePoints);
    assert(LineInChain);
    assert(Chain);
    
    Clear(Chain);
    
    line_segment *Line = LineInChain;
    
    
    //
    // "Forward in the chain"
    // Check in direction of P[1]
    StartMeasure(&M->Forward);
    
    line_segment_darray Forwards;
    Init(&Forwards, 100);
    while (Line)
    {
        Push(&Forwards, Line);
        
        Line->IsProcessed = true;
        Line = GetNextLineSegment(LineSegments, LinePoints, Line, 1, M);
    }
    StopMeasure(&M->Forward);
    
    
    //
    // Backwards in the chain
    // Check in direction of P[0], i.e. backwards
    StartMeasure(&M->Backward);
    
    line_segment_darray Backwards;
    Init(&Backwards, 100);
    
    Line = &Forwards.Data[0];
    Line = GetNextLineSegment(LineSegments, LinePoints, Line, 0, M);
    
    while (Line)
    {
        Push(&Backwards, Line);
        
        Line->IsProcessed = true;
        Line = GetNextLineSegment(LineSegments, LinePoints, Line, 0, M);
    }
    StopMeasure(&M->Backward);
    
    //
    // Merge the two arrays
    StartMeasure(&M->Concatenate);
    
    u32 TotalSize = (Forwards.Used + Backwards.Used);
    s32 Diff = TotalSize - Chain->Capacity;
    if (Diff > 0)
    {
        Grow(Chain, Diff);
    }
    
    for (u32 Index = 0; Index < Backwards.Used; ++Index)
    {
        Push(Chain, &Backwards.Data[Backwards.Used - Index - 1]);
    }
    
    for (u32 Index = 0; Index < Forwards.Used; ++Index)
    {
        Push(Chain, &Forwards.Data[Index]);
    }
    
    StopMeasure(&M->Concatenate);
    
    Free(&Forwards);
    Free(&Backwards);
}



//
// ThE aLGorItHM (tM)
// 
b32 MarchSquares(c_style_state *State, u32 const *DataPtr, f32_darray const *Heights)
{
    assert(State);
    assert(Heights);
    
    if (State->CellCountX == 0)  return false;
    if (State->CellCountY == 0)  return false;
    if (AlmostEqualRelative(State->CellSize, v2_zero))  return false;
    if (!DataPtr)  return false;
    
    u32 CellCountX = State->CellCountX;
    u32 CellCountY = State->CellCountY;
    v2  CellSize   = State->CellSize;
    
    v2_darray *Vertices = &State->Vertices;
    Init(Vertices, 100);
    
    u16_darray *Indices = &State->Indices;
    Init(Indices, 100);
    
    line_segment_darray LineSegments;
    Init(&LineSegments, 100);
    
    line_segment_darray CurrChain;
    Init(&CurrChain, 100);
    
    line_points_bt *LinePoints = nullptr;
    
    State->LineCountAnte = 0;
    
    
    //
    // Iterate through all the heights
    for (u32 HeightIndex = 0; HeightIndex < Heights->Used; ++HeightIndex) {
        f32 CurrHeight = Heights->Data[HeightIndex];
        
        Clear(&LineSegments);
        Free(&LinePoints);
        
        u32 *Begin  = (u32 *)DataPtr;
        u32 *It;
        
        //
        // March the Squares!
        for (u32 x = 0; x < CellCountX - 1; ++x) {
            for (u32 y = 0; y < CellCountY - 1; ++y) {
                StartMeasure(&State->Measures.BinarySum);
                It = Begin + ((y * CellCountX) + x);
                u8 Sum = 0;
                
                //
                // Value of data points
                u32 BottomLeft  = *(It); 
                u32 BottomRight = *(It + 1);
                u32 TopRight    = *(It + 1 + CellCountX);
                u32 TopLeft     = *(It + CellCountX);
                
                //
                // Total sum
                Sum += BottomLeft  >= CurrHeight ? 1 : 0;
                Sum += BottomRight >= CurrHeight ? 2 : 0;
                Sum += TopRight    >= CurrHeight ? 4 : 0;
                Sum += TopLeft     >= CurrHeight ? 8 : 0;
                StopMeasure(&State->Measures.BinarySum);
                
                
                //
                // Vertices, on for each edge of the cell
                StartMeasure(&State->Measures.Lerp);
                v2 Bottom = V2(Lerp(CellSize.x, BottomLeft, BottomRight, CurrHeight), 0.0f);
                v2 Right  = V2(CellSize.x, Lerp(CellSize.y, BottomRight, TopRight, CurrHeight));
                v2 Top    = V2(Lerp(CellSize.x, TopLeft, TopRight, CurrHeight), CellSize.y);
                v2 Left   = V2(0.0f, Lerp(CellSize.y, BottomLeft, TopLeft, CurrHeight));
                StopMeasure(&State->Measures.Lerp);
                
                v2 const P = Hadamard(CellSize, V2((f32)x, (f32)y));
                
                StartMeasure(&State->Measures.Add);
                switch (Sum) {
                    case 1: Add(&LineSegments, &LinePoints, P, Left  , Bottom);  break;
                    case 2: Add(&LineSegments, &LinePoints, P, Bottom, Right);   break;
                    case 3: Add(&LineSegments, &LinePoints, P, Left  , Right);   break;
                    case 4: Add(&LineSegments, &LinePoints, P, Right , Top);     break;
                    
                    case 5: {
                        Add(&LineSegments, &LinePoints, P, Right , Bottom);
                        Add(&LineSegments, &LinePoints, P, Left  , Top);}        break;
                    
                    case 6: Add(&LineSegments, &LinePoints, P, Bottom, Top);     break;
                    case 7: Add(&LineSegments, &LinePoints, P, Left  , Top);     break;
                    case 8: Add(&LineSegments, &LinePoints, P, Top   , Left);    break;
                    case 9: Add(&LineSegments, &LinePoints, P, Top   , Bottom);  break;
                    
                    case 10: { 
                        Add(&LineSegments, &LinePoints, P, Bottom, Left);
                        Add(&LineSegments, &LinePoints, P, Top   , Right);}      break;
                    
                    case 11: Add(&LineSegments, &LinePoints, P, Top   , Right);  break;
                    case 12: Add(&LineSegments, &LinePoints, P, Right , Left);   break;
                    case 13: Add(&LineSegments, &LinePoints, P, Right , Bottom); break;
                    case 14: Add(&LineSegments, &LinePoints, P, Bottom, Left);   break;
                }
                StopMeasure(&State->Measures.Add);
            }
        }
        
        u32 LineSegmentsCount = LineSegments.Used;
        State->LineCountAnte += LineSegmentsCount;
        
        if (LineSegmentsCount > 0)
        {
            f32 PrevSlope;
            f32 CurrSlope = 0.0f;
            
            for (u32 LineSegmentIndex = 0; LineSegmentIndex < LineSegmentsCount; ++LineSegmentIndex)
            {
                line_segment *CurrLine = &LineSegments.Data[LineSegmentIndex];
                if (CurrLine->IsProcessed)  continue;
                
                GetLineChain(&LineSegments, LinePoints, CurrLine, &CurrChain, &State->Measures);
                
                u32 CurrChainCount = CurrChain.Used;
                for (u32 LineChainIndex = 0; LineChainIndex < CurrChainCount; ++LineChainIndex) 
                {
                    StartMeasure(&State->Measures.MergeLines);
                    
                    line_segment *Line = &CurrChain.Data[LineChainIndex];
                    
                    v2 P0 = Line->P[0];
                    v2 P1 = Line->P[1];
                    
                    PrevSlope = CurrSlope;
                    CurrSlope = (P1.y - P0.y) / (P1.x - P0.x);
                    
                    // "Vertex melding", if the slopes of two consecutive lines are
                    // the same (or close enough), then we skip adding the vertices
                    // of this line and focus on the next one
                    if (LineChainIndex > 0 && LineChainIndex < CurrChainCount - 1 &&
                        AlmostEqualRelative(PrevSlope, CurrSlope))
                    {
                        continue;
                    }
                    
                    Push(Indices, (u16)Vertices->Used);
                    Push(Vertices, P0);
                    
                    if (LineChainIndex == CurrChainCount - 1)
                    {
                        Push(Indices, (u16)Vertices->Used);
                        Push(Vertices, P1);
                        
                        Push(Indices, (u16)0xFFFF);
                    }
                    StopMeasure(&State->Measures.MergeLines);
                }
            }
        }
    }
    
    
    //
    // Free memory
    Free(&LineSegments);
    Free(&CurrChain);
    Free(&LinePoints);
    
    return true;
}


void Free(c_style_state *State)
{
    if (State)
    {
        Free(&State->Vertices);
        Free(&State->Indices);
    }
}
