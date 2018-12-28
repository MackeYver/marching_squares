//
//  Extends the base class Marhing_squares with SDL rendering capabilities.
//  This is the version that uses OOP and the standard library.
//
//  Created by Marcus Larsson on 2018-11-21.
//  Copyright © 2018 Marcus Larsson. All rights reserved.
//

#ifndef Marching_squares_SDL_h
#define Marching_squares_SDL_h

#include "oop_std.h"
struct SDL_Renderer;
struct SDL_Surface;
struct SDL_Texture;
struct SDL_Renderer;


class Marching_squares_SDL : public Marching_squares {
public:
    //
    // Operator overloading
    Marching_squares_SDL& operator = (Marching_squares_SDL const&);
    
    
    //
    // Destructor
    ~Marching_squares_SDL();
    
    
    //
    // Constructors
    Marching_squares_SDL(SDL_Renderer *_renderer = nullptr); // default
    Marching_squares_SDL(Marching_squares_SDL const&);       // copy-constructor
    Marching_squares_SDL(SDL_Renderer *renderer, std::vector<int> const& data, Config const c = {});
    Marching_squares_SDL(SDL_Renderer *renderer, u32 const *heights, size_t height_count,
                         Config const config = {});
    
    
    //
    // Setters and getters
    void set_renderer(SDL_Renderer *new_renderer)   {renderer = new_renderer;}
    SDL_Renderer *get_renderer()                    {return renderer;}
    
    
    //
    // Rendering methods
    Result render_cells(int const offset[2] = {}) const;
    Result render_with_sdl(int const offset[2]= {}) const;
    Result render_values(SDL_Surface *target, int const offset[2] = {}) const;
    
protected:
    void destroy_SDL();
    void copy_from(Marching_squares_SDL const& m);
    
    SDL_Renderer *renderer;
    SDL_Surface *font_surface;
    SDL_Texture *font_texture;
};


#endif /* Marching_squares_h */
