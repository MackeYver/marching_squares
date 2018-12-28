//
//  Created by Marcus Larsson on 2018-11-21.
//  Copyright © 2018 Marcus Larsson. All rights reserved.
//

#include "oop_std.h"
#include "Graphics.h"

using namespace std;



//
// Operator overloading
#pragma mark - Operator overloading

/** @desc Returns the level with given index (0-based). Returns nullptr if index is out of bounds. */
Marching_squares::Level *Marching_squares::operator [] (u32 index) {
    if (index >= levels.size())  return nullptr;
    return &levels[index];
}

/** @desc Returns the level with given index (0-based). Returns nullptr if index is out of bounds. */
Marching_squares::Level const *Marching_squares::operator [] (u32 index) const {
    if (index >= levels.size())  return nullptr;
    return &levels[index];
}



//
// Constructors
#pragma mark - Constructors

/** @desc Will construct a new instance of Marching_squares, the vector will be copied. */
Marching_squares::Marching_squares(std::vector<int> const &heights, Config c) :
cell_size(c.cell_size), cell_count_x(c.cell_count_x), cell_count_y(c.cell_count_y)
{
    copy(heights.begin(), heights.end(), back_inserter(data));
}

Marching_squares::Marching_squares(u32 const *heights, size_t height_count, Config const c) :
cell_size(c.cell_size), cell_count_x(c.cell_count_x), cell_count_y(c.cell_count_y)
{
    copy(heights, heights + height_count, back_inserter(data));
}



//
// Setters and getters
#pragma mark - Setters and getters

/** @desc Sets the size of each cell (i.e. distance between points).
          Will not recalculate any line_segments by itself, call march_squares() to recalculate.
          If the new size is 0.0f or smaller it will do nothing and return appropiate error code. */
Marching_squares::Result Marching_squares::set_cell_size(v2 new_size) {
    if (new_size.x <= 0.0f)  return invalid_cell_count_x;
    if (new_size.y <= 0.0f)  return invalid_cell_count_y;
        
    cell_size = new_size;
    return ok;
}

/** @desc Get the level with the given index (0-based). Returns nullptr if index is out of bounds. */
Marching_squares::Level *Marching_squares::get_level(u32 index) {
    if (index >= levels.size())  return nullptr;
    return &levels[index];
}



//
// The algorithm
#pragma mark - The algorithm

/** @desc Utility function, used by "march_squares()" in order to add Line_segments. */
inline void add(std::vector<Line_segment> *line_segments, v2 Po, v2 P0, v2 P1) {
    // @debug {
    if (isnan(P0.x) || isnan(P0.y) || isinf(P0.x) || isinf(P0.y)) {
        int a = 0;
        a++;
    }
    if (isnan(P1.x) || isnan(P1.y) || isinf(P1.x) || isinf(P1.y)) {
        int a = 0;
        a++;
    } // } @debug
    line_segments->push_back(line_segment(Po + P0, Po + P1));
}


/** @desc Utility function, linear interpolation between two heights */
inline f32 lerp(f32 length, f32 h0, f32 h1, f32 current_height) {
    f32 result = 0.0f;

    f32 numerator = current_height - h0;
    f32 denominator = h1 - h0;
    // @note The denominator can be zero, no worries, according to IEEE 754 this
    //       will yield +INF/-INF as a result and not crash the program.
    //       We know that this will not happen in the cases we are interested in.
    f32 factor = numerator / denominator;
    result = factor * length;

    return result;
}


/** @desc Executes the algorithm. Requires that data_ptr != nullptr, width >= 2, height >= 2.
          The resultant line segments will be in a "unit square", i.e. x : [0, 1], y : [0, 1] */
Marching_squares::Result Marching_squares::march_squares(std::vector<f32> const &level_heights) {
    if (data.size() <= 0)           return no_data;
    if (cell_count_x < 2)           return invalid_cell_count_x;
    if (cell_count_y < 2)           return invalid_cell_count_y;
    if (cell_size.x <= 0.0f)        return invalid_cell_size;
    if (cell_size.y <= 0.0f)        return invalid_cell_size;
    if (level_heights.size() == 0)  return invalid_level_height;
    
    levels.clear();

    for (auto& curr_height : level_heights) {
        Level curr_level;
        curr_level.height = curr_height;
        
        std::vector<Line_segment> *ls = &curr_level.line_segments;
        vector<int>::const_iterator begin = data.begin();
        vector<int>::const_iterator it;
        
        //
        // @note We are implicit placing the origin in the bottom left.
        for (u32 y = 0; y < cell_count_y - 1; ++y) {
            for (u32 x = 0; x < cell_count_x - 1; ++x) {
                it = begin + ((y * cell_count_x) + x);
                u8 sum = 0;
                
                //
                // Value of data points
                u32 const bottom_left  = *it;
                u32 const bottom_right = *(it + 1);
                u32 const top_right    = *(it + 1 + cell_count_x);
                u32 const top_left     = *(it + cell_count_x);
                
                //
                // Total sum
                sum += bottom_left  >= curr_height ? 1 : 0;
                sum += bottom_right >= curr_height ? 2 : 0;
                sum += top_right    >= curr_height ? 4 : 0;
                sum += top_left     >= curr_height ? 8 : 0;
                
                //
                // Vertices, on for each edge of the cell
                v2 const bottom = V2(lerp(cell_size.x, bottom_left, bottom_right, curr_height), 0.0f);
                v2 const right  = V2(cell_size.x, lerp(cell_size.y, bottom_right, top_right, curr_height));
                v2 const top    = V2(lerp(cell_size.x, top_left, top_right, curr_height), cell_size.y);
                v2 const left   = V2(0.0f, lerp(cell_size.y, bottom_left, top_left, curr_height));
                
                v2 const P = hadamard(cell_size, V2((f32)x, (f32)y));
                
                switch (sum) {
                    case 1:  add(ls, P, left  , bottom); break;
                    case 2:  add(ls, P, bottom, right);  break;
                    case 3:  add(ls, P, left  , right);  break;
                    case 4:  add(ls, P, right , top);    break;
                    case 5:  add(ls, P, right , bottom);
                             add(ls, P, left  , top);    break;
                    case 6:  add(ls, P, bottom, top);    break;
                    case 7:  add(ls, P, left  , top);    break;
                    case 8:  add(ls, P, top   , left);   break;
                    case 9:  add(ls, P, top   , bottom); break;
                    case 10: add(ls, P, bottom, left);
                             add(ls, P, top   , right);  break;
                    case 11: add(ls, P, top   , right);  break;
                    case 12: add(ls, P, right , left);   break;
                    case 13: add(ls, P, right , bottom); break;
                    case 14: add(ls, P, bottom, left);   break;
                }
            }
        }
        if (curr_level.line_segments.size() > 0) {
            levels.push_back(curr_level);
        }
    }
    
    return ok;
}



