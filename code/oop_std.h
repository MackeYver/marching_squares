//
//  An implementation of the algorithm Marching squares,
//  using OOP and the c standard library
//
//  Created by Marcus Larsson on 2018-11-21.
//  Copyright © 2018 Marcus Larsson. All rights reserved.
//

#ifndef Marching_squares_h
#define Marching_squares_h

#include <vector>
#include <map>
#include "Mathematics.h"



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
    
    struct level {
        std::vector<v2> Vertices;
        std::vector<u16> Indices; // 0xFFFF is used as primitive restart index
        
        f32 Height;
        u32 LineCount;
    };
    
    struct config {
        v2  CellSize;
        u32 CellCountX;
        u32 CellCountY;
        b32 SourceHasOriginUpperLeft;
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
    
    typedef std::vector<level>::size_type level_count;
    
    
    
    //
    // Operator overloading, returns the level at the given index
    level      * operator [] (u32 Index);
    level const* operator [] (u32 Index) const;
    
    
    
    //
    // Constructors
    MarchingSquares() : CellCountX(0), CellCountY(0), CellSize({}) {}
    MarchingSquares(u32 const *Heights, size_t HeightCount, config const Config = {});
    MarchingSquares(std::vector<int> const &Heights, config const Config = {}); // copy constructor
    
    
    
    //
    // Destructor
    // The synthesized destructor will be fine, the memory are managed by std::vector which will 
    // free the allocated memory in their destructors.
    
    
    
    //
    // Setters and getters
    u32 GetCellCountX() const        {return CellCountX;}
    u32 GetCellCountY() const        {return CellCountY;}
    v2  GetCellSize()   const        {return CellSize;}
    
    level_count GetLevelCount() const {return Levels.size();}
    level *GetLevel(u32 Index);
    
    std::vector<int>::const_iterator DataBegin() {return Data.begin();}
    std::vector<int>::const_iterator DataEnd()   {return Data.end();}
    std::vector<int>::size_type      DataSize()  {return Data.size();}
    
    
    
    //
    // Run the algorithm
    result MarchSquares(std::vector<f32> const &LevelHeights);
    
    
    //
    // Data
    protected:
    std::vector<level> Levels;
    std::vector<int> Data;
    v2 CellSize;
    u32 CellCountX = 0;
    u32 CellCountY = 0;
};
#endif /* Marching_squares_h */
