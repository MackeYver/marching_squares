//
// Main
//

#include "Mathematics.h"
#include "Graphics.h"
#include "oop_std_SDL.h"

#include <vector>
#include <stdio.h>

#define kData 0
#if kData == 1
#define kWidth 7
#define kHeight 7
#define kScale 60
const static int data_mirrored[] = {
    1, 0, 0, 0, 0, 0, 2,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 0, 0,
    0, 0, 1, 4, 1, 0, 0,
    0, 0, 1, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 4,
};

#elif kData == 2
#define kWidth 5
#define kHeight 5
#define kScale 100
const static int data_mirrored[] = {
    0, 0, 0, 0, 0,
    0, 1, 1, 1, 0,
    0, 1, 2, 1, 0,
    0, 1, 1, 1, 0,
    0, 0, 0, 0, 0,
};

#elif kData == 3
#define kWidth 11
#define kHeight 11
#define kScale 50
const static int data_mirrored[] = {
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 1, 2, 2, 2, 1, 0, 0, 0, 0, 0,
    0, 1, 3, 3, 2, 1, 1, 1, 1, 0, 0,
    0, 1, 2, 2, 2, 1, 0, 0, 1, 1, 0,
    0, 1, 1, 1, 1, 1, 0, 0, 2, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 0, 2, 1, 0,
    0, 0, 0, 0, 1, 1, 1, 0, 1, 3, 1,
    3, 0, 0, 1, 2, 3, 2, 0, 0, 1, 5,
};

#elif kData == 4
#define kWidth 16
#define kHeight 16
#define kScale 35
const static int data_mirrored[] = {
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 4, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
};

#endif



int main(int argc, char* ars[]) {
    //
    // Init SDL
    Window window = init_window("marching_squares", 1024, 800);
    SDL_Renderer *renderer = SDL_GetRenderer(window.handle);
    assert(renderer);
    
    
#if kData > 0 && kData < 5
    // Lets "mirror the data along x", so that the data we type in the
    // text-editor matches what we see on screen.
    size_t data_size = sizeof(data_mirrored) / sizeof(*data_mirrored);
    u32 data[data_size];
    {
        int counter = 0;
        for (int y = kHeight - 1; y >= 0; --y) {
            for (int x = 0; x < kWidth; ++x) {
                data[counter++] = data_mirrored[(y * kWidth) + x];
            }
        }
    }
    
    // Init the Marching_square class
    Marching_squares::Config config;
    config.cell_size = V2(kScale, kScale);
    config.cell_count_x = kWidth;
    config.cell_count_y = kHeight;
    Marching_squares_SDL ms(renderer, data, kWidth * kHeight, config);
    
#else
    //    #define kWidth 11
    //    #define kHeight 11
    //    #define kScale 50.0f
    //    const char *path = "../data/test.txt";
    
#define kWidth 61
#define kHeight 87
#define kScale 9.0f
    const char *path = "../data/volcano.txt";
    
    std::vector<int> heights;
    //
    // Read data from file
    {
        FILE *file = fopen(path, "r");
        assert(file);
        
        std::vector<int> temp_vector;
        int data;
        
        size_t counter = 0;
        
        while (1) {
            fscanf(file, "%d", &data);
            if(feof(file))  break;
            temp_vector.push_back(data);
            printf("pushing %d\n", data);
            ++counter;
        }
        fclose(file);
        
        //
        // We read a file that has the origin on the upper left, we need to transform it.
        // (((width - 1) - y) * width) + x
        heights.resize(temp_vector.size());
        
        typedef std::vector<int>::size_type size_type;
        size_type new_index;
        size_type curr_index = 0;
        
        printf("\n\n");
        
        for (u32 y = 0; y < kHeight; ++y) {
            for (u32 x = 0; x < kWidth; ++x) {
                new_index = (((kHeight - 1) - y) * kWidth) + x;
                printf("%lu: (%d, %d) -> %lu\n", curr_index, x, y, new_index);
                heights[new_index] = temp_vector[curr_index];
                ++curr_index;
            }
        }
    }
    
    // Init with vector
    Marching_squares::Config config;
    config.cell_size = V2(kScale, kScale);
    config.cell_count_x = kWidth;
    config.cell_count_y = kHeight;
    Marching_squares_SDL ms(renderer, heights, config);
#endif
    
    
    //
    // Do the work
    std::vector<f32> level_heights = {20.0f, 40.0f, 60.0f, 80.0f, 120.0f, 140.0f, 160.0f, 180.0f, 200.0f};
    Marching_squares::Result result = ms.march_squares(level_heights);
    if (result != Marching_squares::ok) {
        fprintf(stderr, "Failed with error: %d\n", result);
        return result;
    }
    
    
    //
    // Render the result
    // @note, @todo Render to texture/surface once, instead of rendering each frame
    bool running = true;
    SDL_Event e;
    
    int const offset[2] = {0, 0};
    
    while (running) {
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT: {
                    running = false;
                } break;
            }
        }
        
        // Render
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, SDL_ALPHA_OPAQUE);
        ms.render_cells(offset);
        ms.render_with_sdl(offset);
        //        ms.render_values(window.surface, offset);
        
        SDL_RenderPresent(renderer);
    }
    
    
    //
    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window.handle);
    SDL_Quit();
    
    return 0;
}
