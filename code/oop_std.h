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
#include <list>
#include "Mathematics.h"



//
// Base class
class MarchingSquares {
    public:
    //
    // Types
    struct level {
        f32 Height;
        std::vector<line_segment> LineSegments;
    };
    
    enum result {
        ok,
        no_data,
        invalid_cell_count_x,
        invalid_cell_count_y,
        invalid_cell_size,
        invalid_renderer,
        invalid_level_height,
    };
    
    struct config {
        v2  CellSize;
        u32 CellCountX;
        u32 CellCountY;
        b32 SourceHasOriginUpperLeft;
    };
    
    typedef std::vector<level>::size_type        level_count;
    typedef std::list<line_segment>::size_type   segment_count;
    
    
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
    // The synthesized destructor will be fine, the memory are managed by std::vector and std::list,
    // which will free the allocated memory in their destructors.
    
    
    //
    // Setters and getters
    result SetCellSize(v2 NewSize);
    
    u32 GetCellCountX() const        {return CellCountX;}
    u32 GetCellCountY() const        {return CellCountY;}
    v2  GetCellSize()   const        {return CellSize;}
    
    level_count GetLevelCount() const {return Levels.size();}
    level *GetLevel(u32 Index);
    
    
    //
    // Run the algorithm
    result MarchSquares(std::vector<f32> const &LevelHeights);
    
    protected:
    std::vector<level> Levels;
    std::vector<int> Data;
    v2 CellSize;
    u32 CellCountX = 0;
    u32 CellCountY = 0;
    b32 OriginBottomLeft = true;
};
#endif /* Marching_squares_h */
