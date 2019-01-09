//
//  Created by Marcus Larsson on 2018-11-21.
//  Copyright © 2018 Marcus Larsson. All rights reserved.
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
inline MarchingSquares::line_segment LineSegment(v2 const& A, v2 const& B) {
    MarchingSquares::line_segment Result;
    Result.P[0] = A;
    Result.P[1] = B;
    Result.IsProcessed = false;
    
    return Result;
}



//
// Operator overloading

/** @desc Returns the level with given index (0-based). Returns nullptr if index is out of bounds. */
MarchingSquares::level *MarchingSquares::operator [] (u32 Index) {
    if (Index >= Levels.size())  return nullptr;
    return &Levels[Index];
}

/** @desc Returns the level with given index (0-based). Returns nullptr if index is out of bounds. */
MarchingSquares::level const *MarchingSquares::operator [] (u32 Index) const {
    if (Index >= Levels.size())  return nullptr;
    return &Levels[Index];
}



//
// Constructors

/** @desc Will construct a new instance of MarchingSquares, the vector will be copied. */
MarchingSquares::MarchingSquares(std::vector<int> const &Heights, config C) :
CellSize(C.CellSize), CellCountX(C.CellCountX), CellCountY(C.CellCountY)
{
    if (C.SourceHasOriginUpperLeft)
    {
        Data.reserve(CellCountX * CellCountY);
        for (u32 row = 0; row < CellCountY; ++row)
        {
            copy(Heights.begin() + (row * CellCountX), 
                 Heights.begin() + (row * CellCountX) + CellCountX, 
                 Data.begin()    + ((CellCountY - 1 - row) * CellCountX));
        }
    }
    else
    {
        copy(Heights.begin(), Heights.end(), back_inserter(Data));
    }
}

MarchingSquares::MarchingSquares(u32 const *Heights, size_t HeightCount, config const C) :
CellSize(C.CellSize), CellCountX(C.CellCountX), CellCountY(C.CellCountY)
{
    if (C.SourceHasOriginUpperLeft)
    {
        for (s32 SourceRow = CellCountY - 1; SourceRow >= 0; --SourceRow)
        {
            copy(Heights + (SourceRow * CellCountX), 
                 Heights + (SourceRow * CellCountX) + CellCountX, 
                 back_inserter(Data));
        }
    }
    else
    {
        copy(Heights, Heights + HeightCount, back_inserter(Data));
    }
}



//
// Setters and getters

/** @desc Get the level with the given index (0-based). Returns nullptr if index is out of bounds. */
MarchingSquares::level *MarchingSquares::GetLevel(u32 Index) {
    if (Index >= Levels.size())  return nullptr;
    return &Levels[Index];
}



//
// The algorithm(TM)

/** @desc Utility function, used by "march_squares()" in order to add Line_segments. */
inline void Add(vector<MarchingSquares::line_segment> *LineSegments, 
                map<u32, vector<MarchingSquares::line_point> > *Points, 
                v2 Po, // Origo (or offset, depends on how you look at it I guess...)
                v2 P0, // First point of the line
                v2 P1) // Second point of the line
{
    P0 += Po;
    P1 += Po;
    
    u32 Index = LineSegments->size();
    LineSegments->push_back(LineSegment(P0, P1));
    
    MarchingSquares::line_point LP;
    LP.P = P0;
    LP.LineIndex = Index;
    u32 Key = CalculateKey(LP.P);
    (*Points)[Key].push_back(LP);
    
    LP.P = P1;
    LP.LineIndex = Index;
    Key = CalculateKey(LP.P);
    (*Points)[Key].push_back(LP);
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


/** @desc Executes the algorithm. Requires that data_ptr != nullptr, width >= 2, height >= 2.
The resultant line segments will be in a "unit square", i.e. x : [0, 1], y : [0, 1] */
MarchingSquares::result MarchingSquares::MarchSquares(std::vector<f32> const &LevelHeights) {
    if (Data.size() <= 0)          return NoData;
    if (CellCountX < 2)            return InvalidCellCountX;
    if (CellCountY < 2)            return InvalidCellCountY;
    if (CellSize.x <= 0.0f)        return InvalidCellSize;
    if (CellSize.y <= 0.0f)        return InvalidCellSize;
    if (LevelHeights.size() == 0)  return InvalidLevelHeight;
    
    Levels.clear();
    
    for (auto& CurrHeight : LevelHeights) {
        level CurrLevel;
        CurrLevel.Height = CurrHeight;
        
        map<u32, vector<line_point> > *LP = &CurrLevel.LinePoints;
        vector<line_segment> *LS = &CurrLevel.LineSegments;
        
        vector<int>::const_iterator Begin = Data.begin();
        vector<int>::const_iterator It;
        
        
        //
        // March the Squares!
        for (u32 x = 0; x < CellCountX - 1; ++x) {
            for (u32 y = 0; y < CellCountY - 1; ++y) {
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
                
                //
                // Vertices, on for each edge of the cell
                v2 Bottom = V2(Lerp(CellSize.x, BottomLeft, BottomRight, CurrHeight), 0.0f);
                v2 Right  = V2(CellSize.x, Lerp(CellSize.y, BottomRight, TopRight, CurrHeight));
                v2 Top    = V2(Lerp(CellSize.x, TopLeft, TopRight, CurrHeight), CellSize.y);
                v2 Left   = V2(0.0f, Lerp(CellSize.y, BottomLeft, TopLeft, CurrHeight));
                
                v2 const P = Hadamard(CellSize, V2((f32)x, (f32)y));
                
                switch (Sum) {
                    case 1: Add(LS, LP, P, Left  , Bottom);  break;
                    case 2: Add(LS, LP, P, Bottom, Right);   break;
                    case 3: Add(LS, LP, P, Left  , Right);   break;
                    case 4: Add(LS, LP, P, Right , Top);     break;
                    case 5: {
                        Add(LS, LP, P, Right , Bottom);
                        Add(LS, LP, P, Left  , Top);}        break;
                    case 6: Add(LS, LP, P, Bottom, Top);     break;
                    case 7: Add(LS, LP, P, Left  , Top);     break;
                    case 8: Add(LS, LP, P, Top   , Left);    break;
                    case 9: Add(LS, LP, P, Top   , Bottom);  break;
                    case 10: { 
                        Add(LS, LP, P, Bottom, Left);
                        Add(LS, LP, P, Top   , Right);}      break;
                    case 11: Add(LS, LP, P, Top   , Right);  break;
                    case 12: Add(LS, LP, P, Right , Left);   break;
                    case 13: Add(LS, LP, P, Right , Bottom); break;
                    case 14: Add(LS, LP, P, Bottom, Left);   break;
                }
            }
        }
        
        if (CurrLevel.LineSegments.size() > 0)
        {
            for (line_segment& CurrLine : *LS)
            {
                CurrLevel.Indices.push_back((u16)CurrLevel.Vertices.size());
                CurrLevel.Vertices.push_back(CurrLine.P[0]);
                
                CurrLevel.Indices.push_back((u16)CurrLevel.Vertices.size());
                CurrLevel.Vertices.push_back(CurrLine.P[1]);
                
                CurrLevel.Indices.push_back((u16)0xFFFF);
            }
            CurrLevel.LineCount = CurrLevel.LineSegments.size();
            Levels.push_back(CurrLevel);
        }
    }
    
    return Ok;
}



//
// Reduce the primitive soup
// i.e. meld vertices
//
MarchingSquares::line_segment *GetNextLineSegment(MarchingSquares::level& CurrLevel, 
                                                  MarchingSquares::line_segment *CurrLine,
                                                  u32 DirectionIndex)
{
    assert(CurrLine);
    u32 Key = CalculateKey(CurrLine->P[DirectionIndex]);
    
    u32 Count = CurrLevel.LinePoints.count(Key);
    assert(Count > 0); // There will always exist at least one point that generates this key
    
    std::vector<MarchingSquares::line_point> *Points = &CurrLevel.LinePoints[Key];
    assert(Points->size() != 0); // There will always exist at least one point that generates this key
    for (auto& Point : *Points)
    {
        assert(Point.LineIndex >= 0);
        assert(Point.LineIndex < CurrLevel.LineSegments.size());
        
        MarchingSquares::line_segment *NextLine = &CurrLevel.LineSegments[Point.LineIndex];
        if (NextLine != CurrLine)
        {
            return NextLine;
        }
    }
    
    return nullptr; // No connected lines in the direction of the given index!
}

void GetLineChain(MarchingSquares::level& CurrLevel, 
                  MarchingSquares::line_segment& LineInChain, 
                  vector<MarchingSquares::line_segment>& Chain)
{
    Chain.clear();
    
    MarchingSquares::line_segment *LineSegment = &LineInChain;
    
    //
    // "Forward in the chain"
    // Check in direction of P[1]
    while (LineSegment && !LineSegment->IsProcessed)
    {
        Chain.push_back(*LineSegment);
        
        LineSegment->IsProcessed = true;
        LineSegment = GetNextLineSegment(CurrLevel, LineSegment, 1);
    }
    
    
    //
    // Backwards in the chain
    // Check in direction of P[0], i.e. backwards
    // - this is the slow and stupid verions, we'll look at perfomance as soon as it's working
    LineSegment = &Chain[0];
    LineSegment = GetNextLineSegment(CurrLevel, LineSegment, 0);
    
    while (LineSegment && !LineSegment->IsProcessed)
    {
        Chain.insert(Chain.begin(), *LineSegment); // SLOOOOOOOOW!
        
        LineSegment->IsProcessed = true;
        LineSegment = GetNextLineSegment(CurrLevel, LineSegment, 0);
    }
}

MarchingSquares::result MarchingSquares::Simplify()
{
    if (Levels.size() == 0)  return NoLineSegments;
    
    vector<v2>::size_type TotVertexDiff = 0;
    vector<u16>::size_type TotIndexDiff = 0;
    u32 TotLineDiff = 0;
    
    vector<v2>::size_type TotOldVertexCount = 0;
    vector<u16>::size_type TotOldIndexCount = 0;
    u32 TotOldLineCount = 0;
    
    vector<line_segment> CurrChain;
    
    for (level& CurrLevel : Levels)
    {
        vector<v2>::size_type OldVertexCount = CurrLevel.Vertices.size();
        vector<u16>::size_type OldIndexCount = CurrLevel.Indices.size();
        u32 OldLineCount = CurrLevel.LineCount;
        
        TotOldVertexCount += OldVertexCount;
        TotOldIndexCount += OldIndexCount;
        TotOldLineCount += OldLineCount;
        
#if 0
        printf("Level %.1f\n", CurrLevel.Height);
        printf("  From: line segment count = %d, vertex count = %d, index count = %d\n", 
               OldLineCount, OldVertexCount, OldIndexCount);
#endif
        
        CurrLevel.Vertices.clear();
        CurrLevel.Indices.clear();
        CurrLevel.LineCount = 0;
        
        f32 PrevSlope;
        f32 CurrSlope = 0.0f;
        
        for (auto& CurrLine : CurrLevel.LineSegments)
        {
            if (CurrLine.IsProcessed)  continue;
            
            GetLineChain(CurrLevel, CurrLine, CurrChain);
            
            for (vector<line_segment>::size_type LineIndex = 0;
                 LineIndex < CurrChain.size();
                 ++LineIndex)
            {
                line_segment *Line = &CurrChain[LineIndex];
                
                v2 P0 = Line->P[0];
                v2 P1 = Line->P[1];
                
                PrevSlope = CurrSlope;
                CurrSlope = (P1.y - P0.y) / (P1.x - P0.x);
                
                if (LineIndex > 0 && LineIndex < CurrChain.size() - 1 &&
                    AlmostEqualRelative(PrevSlope, CurrSlope))
                {
                    //printf("Skipping due to similar slope!\n");
                    continue;
                }
                
                CurrLevel.Indices.push_back((u16)CurrLevel.Vertices.size());
                CurrLevel.Vertices.push_back(P0);
                
                if (LineIndex == CurrChain.size() - 1)
                {
                    CurrLevel.Indices.push_back((u16)CurrLevel.Vertices.size());
                    CurrLevel.Vertices.push_back(P1);
                    
                    CurrLevel.Indices.push_back((u16)0xFFFF);
                }
                
                ++CurrLevel.LineCount;
            }
        }
        
        //
        // Some metrics
        vector<v2>::size_type NewVertexCount = CurrLevel.Vertices.size();
        vector<u16>::size_type NewIndexCount = CurrLevel.Indices.size();
        u32 NewLineCount = CurrLevel.LineCount;
        
#if 0
        printf("    To: line segment count = %d, vertex count = %d, index count = %d\n", 
               NewLineCount, NewVertexCount, NewIndexCount);
        printf("        line diff. = %d (%5.1f%%), vertex diff. = %d (%5.1f%%), index diff. = %d (%5.1f%%)\n",
               OldLineCount   - NewLineCount  , 100.0f * (f32)(OldLineCount - NewLineCount) / (f32)OldLineCount,
               OldVertexCount - NewVertexCount, 100.0f * (f32)(OldVertexCount - NewVertexCount) / (f32)OldVertexCount,
               OldIndexCount  - NewIndexCount , 100.0f * (f32)(OldIndexCount - NewIndexCount) / (f32)OldIndexCount);
#endif
        
        TotLineDiff += OldLineCount - NewLineCount;
        TotVertexDiff += OldVertexCount - NewVertexCount;
        TotIndexDiff += OldIndexCount - NewIndexCount;
    }
    
    //
    // More metrics
    printf(" \nTotal: line segments = %d, vertices = %d, indices = %d\n", 
           TotOldLineCount - TotLineDiff, TotOldVertexCount - TotVertexDiff, TotOldIndexCount - TotIndexDiff);
    printf("       line segment diff = %-4.1f%%, vertex diff = %-4.1f%%, index diff = %-4.1f%%\n", 
           100.0f * (f32)TotLineDiff / (f32)TotOldLineCount, 
           100.0f * (f32)TotVertexDiff / (f32)TotOldVertexCount, 
           100.0f * (f32)TotIndexDiff / (f32)TotOldIndexCount);
    printf(" Vertex size reduced with %d kb, index data reduced with %d kb\n",
           (TotVertexDiff * sizeof(v2)) / 1024, (TotIndexDiff * sizeof(u16)) / 1024);
    
    return Ok;
}