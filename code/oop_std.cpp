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


#include "oop_std.h"

#include <assert.h>
#include <stdio.h>

using namespace std;


//
// Line points
inline u32 CalculateKey(v2 const& P)
{
    // 3847 is a prime, choosen due to it being a bit larger than 3840 which is the width of 4k display.
    // Admittedly a bit arbitrary, but we need to generate a key to use for sorting somehow and this may
    // be good enough.
    return 3847*(u32)P.y + (u32)P.x;
}

b32 CompareLinePoints(MarchingSquares::line_point const& a, MarchingSquares::line_point const& b)
{
    return CalculateKey(a.P) < CalculateKey(b.P);
}

b32 Equal(MarchingSquares::line_point const& a, MarchingSquares::line_point const& b)
{
    if (AlmostEqualRelative(a.P.x, b.P.x) && AlmostEqualRelative(a.P.y, b.P.y))  return true;
    return false;
}



//
// Line segments
inline MarchingSquares::line_segment LineSegment(v2 const& A, v2 const& B)
{
    MarchingSquares::line_segment Result;
    Result.P[0] = A;
    Result.P[1] = B;
    Result.IsProcessed = false;
    
    return Result;
}



//
// Constructor
// 
MarchingSquares::MarchingSquares()
: CellCountX(0), CellCountY(0), CellSize({}), DataPtr(nullptr)
{
}



// Setters
//
void MarchingSquares::SetConfig(config const *Config)
{
    CellCountX = Config->CellCountX;
    CellCountY = Config->CellCountY;
    CellSize = Config->CellSize;
}

void MarchingSquares::SetDataPtr(std::vector<u32> *Heights, config const *Config)
{
    assert(Heights);
    Data.clear();
    DataPtr = Heights->data();
    SetConfig(Config);
}

void MarchingSquares::SetDataPtr(u32 *Heights, config const *Config)
{
    assert(Heights);
    Data.clear();
    DataPtr = Heights;
    SetConfig(Config);
}

void MarchingSquares::CopyData(std::vector<u32> *Heights, config const *Config)
{
    assert(Heights);
    Data.clear();
    copy(Heights->begin(), Heights->end(), back_inserter(Data));
    DataPtr = Data.data();
    SetConfig(Config);
}

void MarchingSquares::CopyData(u32 *Heights, u32 HeightsCount, config const *Config)
{
    assert(Heights);
    Data.clear();
    copy(Heights, Heights + HeightsCount, back_inserter(Data));
    DataPtr = Data.data();
    SetConfig(Config);
}



//
// The algorithm(TM)
//

/** @desc Utility function, used by "march_squares()" in order to add Line_segments. */
inline void Add(std::vector<MarchingSquares::line_segment>& LineSegments, 
                std::map<u32, vector<MarchingSquares::line_point> >& Points, 
                v2 Po, // Origo (or offset, depends on how you look at it I guess...)
                v2 P0, // First point of the line
                v2 P1) // Second point of the line
{
    P0 += Po;
    P1 += Po;
    
    u32 Index = LineSegments.size();
    LineSegments.push_back(LineSegment(P0, P1));
    
    MarchingSquares::line_point LP;
    LP.P = P0;
    LP.LineIndex = Index;
    u32 Key = CalculateKey(LP.P);
    Points[Key].push_back(LP);
    
    LP.P = P1;
    LP.LineIndex = Index;
    Key = CalculateKey(LP.P);
    Points[Key].push_back(LP);
}


/** @desc Utility function, linear interpolation between two heights */
inline f32 Lerp(f32 Length, f32 H0, f32 H1, f32 CurrentHeight) {
    f32 Result = 0.0f;
    
    f32 Numerator = CurrentHeight - H0;
    f32 Denominator = H1 - H0;
    // @note The denominator can be zero, no worries, according to IEEE 754 this
    //       will yield +INF/-INF as a result and not crash the program.
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



//
// Reduce the primitive soup
// i.e. meld vertices
//
MarchingSquares::line_segment *GetNextLineSegment(std::vector<MarchingSquares::line_segment>& LineSegments, 
                                                  std::map<u32, std::vector<MarchingSquares::line_point> >& LinePoints,
                                                  MarchingSquares::line_segment *CurrLine,
                                                  u32 DirectionIndex)
{
    assert(CurrLine);
    
    u32 Key = CalculateKey(CurrLine->P[DirectionIndex]);
    
    u32 Count = LinePoints.count(Key);
    assert(Count > 0); // There will always exist at least one point that generates this key
    
    std::vector<MarchingSquares::line_point> *Points = &LinePoints[Key];
    assert(Points->size() != 0); // There will always exist at least one point that generates this key
    for (auto& Point : *Points)
    {
        assert(Point.LineIndex >= 0);
        assert(Point.LineIndex < LineSegments.size());
        
        MarchingSquares::line_segment *NextLine = &LineSegments[Point.LineIndex];
        if (NextLine != CurrLine)
        {
            return NextLine;
        }
    }
    
    return nullptr; // No connected lines in the direction of the given index!
}

void GetLineChain(std::vector<MarchingSquares::line_segment>& LineSegments, 
                  std::map<u32, std::vector<MarchingSquares::line_point> >& LinePoints,
                  MarchingSquares::line_segment& LineInChain, 
                  vector<MarchingSquares::line_segment>& Chain)
{
    Chain.clear();
    
    MarchingSquares::line_segment *Line = &LineInChain;
    
    //
    // "Forward in the chain"
    // Check in direction of P[1]
    while (Line && !Line->IsProcessed)
    {
        Chain.push_back(*Line);
        
        Line->IsProcessed = true;
        Line = GetNextLineSegment(LineSegments, LinePoints, Line, 1);
    }
    
    
    //
    // Backwards in the chain
    // Check in direction of P[0], i.e. backwards
    // - this is the slow and stupid verions, we'll look at perfomance as soon as it's working
    Line = &Chain[0];
    Line = GetNextLineSegment(LineSegments, LinePoints, Line, 0);
    
    while (Line && !Line->IsProcessed)
    {
        Chain.insert(Chain.begin(), *Line); // // TODO(Marcus): SLOOOOOOOOW!
        
        Line->IsProcessed = true;
        Line= GetNextLineSegment(LineSegments, LinePoints, Line, 0);
    }
}



/** @desc Executes the algorithm. Requires that data_ptr != nullptr, width >= 2, height >= 2.
The resultant line segments will be in a "unit square", i.e. x : [0, 1], y : [0, 1] */
MarchingSquares::result MarchingSquares::MarchSquares(std::vector<f32> const &LevelHeights) {
    if (!DataPtr)                 return NoData;
    if (CellCountX < 2)           return InvalidCellCountX;
    if (CellCountY < 2)           return InvalidCellCountY;
    if (CellSize.x <= 0.0f)       return InvalidCellSize;
    if (CellSize.y <= 0.0f)       return InvalidCellSize;
    if (LevelHeights.size() == 0) return InvalidLevelHeight;
    
    Vertices.clear();
    Indices.clear();
    
    std::vector<line_segment> LineSegments;
    std::map<u32, std::vector<line_point> > LinePoints;
    
    StartMeasure(&TTotal);
    for (auto& CurrHeight : LevelHeights) {
        LineSegments.clear();
        LinePoints.clear();
        
        u32 *Begin  = DataPtr;
        u32 *It;
        
        //
        // March the Squares!
        StartMeasure(&TMarching);
        for (u32 x = 0; x < CellCountX - 1; ++x) {
            for (u32 y = 0; y < CellCountY - 1; ++y) {
                StartMeasure(&TBinarySum);
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
                StopMeasure(&TBinarySum);
                
                
                
                //
                // Vertices, on for each edge of the cell
                StartMeasure(&TLerp);
                v2 Bottom = V2(Lerp(CellSize.x, BottomLeft, BottomRight, CurrHeight), 0.0f);
                v2 Right  = V2(CellSize.x, Lerp(CellSize.y, BottomRight, TopRight, CurrHeight));
                v2 Top    = V2(Lerp(CellSize.x, TopLeft, TopRight, CurrHeight), CellSize.y);
                v2 Left   = V2(0.0f, Lerp(CellSize.y, BottomLeft, TopLeft, CurrHeight));
                StopMeasure(&TLerp);
                
                v2 const P = Hadamard(CellSize, V2((f32)x, (f32)y));
                
                StartMeasure(&TAdd);
                switch (Sum) {
                    case 1: Add(LineSegments, LinePoints, P, Left  , Bottom);  break;
                    case 2: Add(LineSegments, LinePoints, P, Bottom, Right);   break;
                    case 3: Add(LineSegments, LinePoints, P, Left  , Right);   break;
                    case 4: Add(LineSegments, LinePoints, P, Right , Top);     break;
                    
                    case 5: {
                        Add(LineSegments, LinePoints, P, Right , Bottom);
                        Add(LineSegments, LinePoints, P, Left  , Top);}        break;
                    
                    case 6: Add(LineSegments, LinePoints, P, Bottom, Top);     break;
                    case 7: Add(LineSegments, LinePoints, P, Left  , Top);     break;
                    case 8: Add(LineSegments, LinePoints, P, Top   , Left);    break;
                    case 9: Add(LineSegments, LinePoints, P, Top   , Bottom);  break;
                    
                    case 10: { 
                        Add(LineSegments, LinePoints, P, Bottom, Left);
                        Add(LineSegments, LinePoints, P, Top   , Right);}      break;
                    
                    case 11: Add(LineSegments, LinePoints, P, Top   , Right);  break;
                    case 12: Add(LineSegments, LinePoints, P, Right , Left);   break;
                    case 13: Add(LineSegments, LinePoints, P, Right , Bottom); break;
                    case 14: Add(LineSegments, LinePoints, P, Bottom, Left);   break;
                }
                StopMeasure(&TAdd);
            }
        }
        StopMeasure(&TMarching);
        
        
        if (LineSegments.size() > 0)
        {
            StartMeasure(&TSimplify);
            
            f32 PrevSlope;
            f32 CurrSlope = 0.0f;
            
            for (auto& CurrLine : LineSegments)
            {
                if (CurrLine.IsProcessed)  continue;
                
                vector<line_segment> CurrChain;
                
                StartMeasure(&TGetLineChain);
                GetLineChain(LineSegments, LinePoints, CurrLine, CurrChain);
                StopMeasure(&TGetLineChain);
                
                for (vector<line_segment>::size_type LineIndex = 0;
                     LineIndex < CurrChain.size();
                     ++LineIndex)
                {
                    StartMeasure(&TMergeLines);
                    
                    line_segment *Line = &CurrChain[LineIndex];
                    
                    v2 P0 = Line->P[0];
                    v2 P1 = Line->P[1];
                    
                    PrevSlope = CurrSlope;
                    CurrSlope = (P1.y - P0.y) / (P1.x - P0.x);
                    
                    // "Vertex melding", if the slopes of two consecutive lines are
                    // the same (or close enough), then we skip adding the vertices
                    // of this line and focus on the next one
                    if (LineIndex > 0 && LineIndex < CurrChain.size() - 1 &&
                        AlmostEqualRelative(PrevSlope, CurrSlope))
                    {
                        continue;
                    }
                    
                    Indices.push_back((u16)Vertices.size());
                    Vertices.push_back(P0);
                    
                    if (LineIndex == CurrChain.size() - 1)
                    {
                        Indices.push_back((u16)Vertices.size());
                        Vertices.push_back(P1);
                        
                        Indices.push_back((u16)0xFFFF);
                    }
                    StopMeasure(&TMergeLines);
                }
            }
            StopMeasure(&TSimplify);
        }
    }
    StopMeasure(&TTotal);
    
    
    return Ok;
}