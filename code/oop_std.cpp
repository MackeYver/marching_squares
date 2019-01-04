//
//  Created by Marcus Larsson on 2018-11-21.
//  Copyright © 2018 Marcus Larsson. All rights reserved.
//

#include "oop_std.h"

using namespace std;



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

/** @desc Sets the size of each cell (i.e. distance between points).
          Will not recalculate any line_segments by itself, call march_squares() to recalculate.
          If the new size is 0.0f or smaller it will do nothing and return appropiate error code. */
MarchingSquares::result MarchingSquares::SetCellSize(v2 NewSize) {
    if (NewSize.x <= 0.0f)  return invalid_cell_count_x;
    if (NewSize.y <= 0.0f)  return invalid_cell_count_y;
    
    CellSize = NewSize;
    return ok;
}

/** @desc Get the level with the given index (0-based). Returns nullptr if index is out of bounds. */
MarchingSquares::level *MarchingSquares::GetLevel(u32 Index) {
    if (Index >= Levels.size())  return nullptr;
    return &Levels[Index];
}



//
// The algorithm

/** @desc Utility function, used by "march_squares()" in order to add Line_segments. */
inline void Add(std::vector<line_segment> *LineSegments, v2 Po, v2 P0, v2 P1) {
    LineSegments->push_back(LineSegment(Po + P0, Po + P1));
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
    if (Data.size() <= 0)          return no_data;
    if (CellCountX < 2)           return invalid_cell_count_x;
    if (CellCountY < 2)           return invalid_cell_count_y;
    if (CellSize.x <= 0.0f)        return invalid_cell_size;
    if (CellSize.y <= 0.0f)        return invalid_cell_size;
    if (LevelHeights.size() == 0)  return invalid_level_height;
    
    Levels.clear();
    
    for (auto& CurrHeight : LevelHeights) {
        level CurrLevel;
        CurrLevel.Height = CurrHeight;
        
        std::vector<line_segment> *LS = &CurrLevel.LineSegments;
        vector<int>::const_iterator Begin = Data.begin();
        vector<int>::const_iterator It;
        
        //
        // Mr. Bus driver, March the Squares!
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
                v2 Bottom;
                v2 Right;
                v2 Top;
                v2 Left;
                
                Bottom = V2(Lerp(CellSize.x, BottomLeft, BottomRight, CurrHeight), 0.0f);
                Right  = V2(CellSize.x, Lerp(CellSize.y, BottomRight, TopRight, CurrHeight));
                Top    = V2(Lerp(CellSize.x, TopLeft, TopRight, CurrHeight), CellSize.y);
                Left   = V2(0.0f, Lerp(CellSize.y, BottomLeft, TopLeft, CurrHeight));
                
                v2 const P = Hadamard(CellSize, V2((f32)x, (f32)y));
                
                switch (Sum) {
                    case 1: { 
                        Add(LS, P, Left  , Bottom); 
                    } break;
                    
                    case 2: { 
                        Add(LS, P, Bottom, Right);  
                    } break;
                    
                    case 3: { 
                        Add(LS, P, Left  , Right);  
                    } break;
                    
                    case 4: { 
                        Add(LS, P, Right , Top);    
                    } break;
                    
                    case 5: { 
                        Add(LS, P, Right , Bottom);
                        Add(LS, P, Left  , Top);    
                    } break;
                    
                    case 6: { 
                        Add(LS, P, Bottom, Top);    
                    } break;
                    
                    case 7: { 
                        Add(LS, P, Left  , Top);    
                    } break;
                    
                    case 8: { 
                        Add(LS, P, Top   , Left);   
                    } break;
                    
                    case 9: { 
                        Add(LS, P, Top   , Bottom); 
                    } break;
                    
                    case 10: { 
                        Add(LS, P, Bottom, Left);
                        Add(LS, P, Top   , Right);  
                    } break;
                    
                    case 11: {  
                        Add(LS, P, Top   , Right);  
                    } break;
                    
                    case 12: { 
                        Add(LS, P, Right , Left);   
                    } break;
                    
                    case 13: { 
                        Add(LS, P, Right , Bottom); 
                    } break;
                    
                    case 14: { 
                        Add(LS, P, Bottom, Left);   
                    } break;
                }
            }
        }
        if (CurrLevel.LineSegments.size() > 0) {
            Levels.push_back(CurrLevel);
        }
    }
    
    return ok;
}



