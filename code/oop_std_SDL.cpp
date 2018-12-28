//
//  Created by Marcus Larsson on 2018-11-21.
//  Copyright © 2018 Marcus Larsson. All rights reserved.
//

#include "oop_std_SDL.h"
#include "Graphics.h"

#include <string>

using namespace std;


static const SDL_Colour colours[] = {
    {255, 0  ,   0, 255},
    {0  , 255,   0, 255},
    {0  , 0  , 255, 255},
    
    {255, 255,   0, 255},
    {255,   0, 255, 255},
    {255, 255, 255, 255},
    
    {  0, 255, 255, 255},
    
    {125, 0  ,   0, 255},
    {0  , 125,   0, 255},
    {0  , 0  , 125, 255},
    
    {125, 125,   0, 255},
    {125,   0, 125, 255},
    {125, 125, 125, 255},
    
    {  0, 125, 125, 255},
};



//
// Utility method
#pragma mark - Utility method

void Marching_squares_SDL::destroy_SDL() {
    if (font_texture) {
        SDL_DestroyTexture(font_texture);
        font_texture = nullptr;
    }
    
    if (font_surface) {
        SDL_FreeSurface(font_surface);
        font_surface = nullptr;
    }
}

void Marching_squares_SDL::copy_from(const Marching_squares_SDL &m) {
    renderer = m.renderer;
    
    // Copy surface and texture
    if (m.font_surface) {
        font_surface = SDL_DuplicateSurface(m.font_surface);
    } else {
        font_surface = nullptr;
    }
    
    if (m.font_texture) {
        font_texture = SDL_CreateTextureFromSurface(renderer, font_surface);
    } else {
        font_texture = nullptr;
    }
    
    // Copy data
    if (m.data.size() > 0) {
        copy(m.data.begin(), m.data.end(), back_inserter(data));
    }
    
    if (m.levels.size() > 0) {
        copy(m.levels.begin(), m.levels.end(), back_inserter(levels));
    }
    
    cell_size = m.cell_size;
    cell_count_x = m.cell_count_x;
    cell_count_y = m.cell_count_y;
}


//
// Operator overloading
#pragma mark - Operator overloading

/** @note We implement this and the copy-constructor due to that we use a destructor in order to manage
          memory (the SDL_texture and the SDL_surface) it indicates that we also want to control those
          operations (basically following "The rule of three"). */
Marching_squares_SDL& Marching_squares_SDL::operator = (Marching_squares_SDL const &rhs) {
    // In this case we're assigning new value to this instance, i.e. wer're getting rid of the old
    // values and assigning new ones.
    destroy_SDL();
    data.clear();
    levels.clear();
    
    copy_from(rhs);
    
    return *this;
}



//
// Destructor
#pragma mark - Destructor

Marching_squares_SDL::~Marching_squares_SDL() {
    destroy_SDL();
}



//
// Constructors
#pragma mark - Constructors

Marching_squares_SDL::Marching_squares_SDL(SDL_Renderer *_renderer) : Marching_squares(),
renderer(_renderer), font_surface(nullptr), font_texture(nullptr) {
}

Marching_squares_SDL::Marching_squares_SDL(SDL_Renderer *_renderer, std::vector<int> const& data, Config const c) :
Marching_squares(data, c), renderer(_renderer)
{
    // Load "font"
    font_surface = load_bmp("../data/font.bmp");
    font_texture = SDL_CreateTextureFromSurface(renderer, font_surface);
}

Marching_squares_SDL::Marching_squares_SDL(SDL_Renderer *r, u32 const *h, size_t hcount, Config const c) :
Marching_squares(h, hcount, c), renderer(r)
{
    // Load "font"
    font_surface = load_bmp("../data/font.bmp");
    font_texture = SDL_CreateTextureFromSurface(renderer, font_surface);
}

Marching_squares_SDL::Marching_squares_SDL(Marching_squares_SDL const &rhs) {
    copy_from(rhs);
}



//
// Rendering methods
#pragma mark - Rendering methods

/** @desc Renders the cells (i.e a grid pattern, with the data values as its corners). */
Marching_squares::Result Marching_squares_SDL::render_cells(const int offset[2]) const {
    if (!renderer)  return invalid_renderer;
    
    int w = (cell_count_x - 1) * cell_size.x;
    for (int y = 0; y < cell_count_y; ++y) {
        SDL_RenderDrawLine(renderer,
                           offset[0]    , offset[1] + y * cell_size.y,
                           offset[0] + w, offset[1] + y * cell_size.y);
    }
    
    int h = (cell_count_y - 1) * cell_size.y;
    for (int x = 0; x < cell_count_x; ++x) {
        SDL_RenderDrawLine(renderer,
                           offset[0] + x * cell_size.x, offset[1],
                           offset[0] + x * cell_size.x, offset[1] + h);
    }
    
    return ok;
}

/** @desc Renders the values of each corner of the grid. Will lazily create a font texture. */
Marching_squares::Result Marching_squares_SDL::render_values(SDL_Surface *target, int const offset[2]) const {
    if (!renderer)  return invalid_renderer;
    
    //
    // The font texture contains glyphs for ASCII [32, ..., 96]
    // Each glyph is 10 x 21 and contains 10 glyphs per row and
    // 10 rows => a bitmap of the size 100 x 210.
    // The origin is in the upper left.
    //
    
    u32 glyph_width = 10;
    u32 glyph_height = 21;
 
    SDL_Rect src_rect = {0, 0, 10, 21};
    SDL_Rect dst_rect = {0, 0, 10, 21};
    
    char text[7];
    
    vector<int>::const_iterator it = data.begin();
    
    for (int y = 0; y < cell_count_y; ++y) {
        for (int x = 0; x < cell_count_x; ++x) {
            dst_rect.x = cell_size.x * x;
            dst_rect.y = (cell_size.y * (cell_count_y - 1)) - (cell_size.y * y);
            
            u32 value = *(it + ((y * cell_count_x) + x));
            snprintf(text, 7, "%d", value);

            for (int i = 0; text[i] != '\0'; ++i) {
                if (text[i]!= ' ') {
                    int glyph_index = text[i] - 32;
                    src_rect.x = ((glyph_index % 10) - 1) * glyph_width;
                    src_rect.y = (glyph_index / 10) * glyph_height;

                    SDL_RenderCopy(renderer, font_texture, &src_rect, &dst_rect);
                    dst_rect.x += glyph_width;
                }
            }
        }
    }
    
    return ok;
}


/** @desc Renders the result of the algorithm (if any).
          Uses scale as distance between data points. */
Marching_squares::Result Marching_squares_SDL::render_with_sdl(int const offset[2]) const {
    if (!renderer)  return invalid_renderer;
    
    //
    // @note The coordinate system of SDL2 has the origin in the upper left and increases
    //       to the bottom right. That's why we have to trix with the y coordinate.
    //
    
    //
    // @note: this is highly inefficient!
    u32 colour_array_size = sizeof(colours) / sizeof(*colours);
    u32 level_counter = 0;
    
    v2 const Po = V2(offset[0], offset[1] + (cell_count_y - 1) * cell_size.y);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    for (auto& level : levels) {
//        set_colour(renderer, &colours[level_counter % colour_array_size]);
        ++level_counter;
        
        for (auto& line : level.line_segments) {
            v2 P0 = Po + V2(line.P0.x, -line.P0.y);
            v2 P1 = Po + V2(line.P1.x, -line.P1.y);
            SDL_RenderDrawLine(renderer, P0.x, P0.y, P1.x, P1.y);
        }
    }
    
    return ok;
}
