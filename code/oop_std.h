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
class Marching_squares {
public:
    //
    // Types
    struct Level {
        f32 height;
        std::vector<Line_segment> line_segments;
    };
    
    enum Result {
        ok,
        no_data,
        invalid_cell_count_x,
        invalid_cell_count_y,
        invalid_cell_size,
        invalid_renderer,
        invalid_level_height,
    };
    
    struct Config {
        v2  cell_size;
        u32 cell_count_x;
        u32 cell_count_y;
        b32 source_has_origin_upper_left;
    };
    
    typedef std::vector<Level>::size_type      level_count;
    typedef std::list<Line_segment>::size_type segment_count;
    
    
    //
    // Operator overloading
    Level      * operator [] (u32 index);
    Level const* operator [] (u32 index) const;
    
    
    //
    // Constructors
    Marching_squares() : cell_count_x(0), cell_count_y(0), cell_size({}) {}
    Marching_squares(u32 const *heights, size_t height_count, Config const config = {});
    Marching_squares(std::vector<int> const &heights, Config const config = {});
    
    
    //
    // Setters and getters
    Result set_cell_size(v2 new_size);
    
    u32 get_cell_count_x() const        {return cell_count_x;}
    u32 get_cell_count_y() const        {return cell_count_y;}
    v2  get_cell_size()    const        {return cell_size;}
    
    level_count get_level_count() const {return levels.size();}
    Level *get_level(u32 index);

    
    //
    // Run the algorithm
    Result march_squares(std::vector<f32> const &level_heights);
    
protected:
    std::vector<Level> levels;
    std::vector<int> data;
    v2 cell_size;
    u32 cell_count_x = 0;
    u32 cell_count_y = 0;
};
#endif /* Marching_squares_h */
