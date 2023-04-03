#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>

// #include <game.hpp>
#include <coords.hpp>

struct Texture
{
    GPU_Image *ptr;
    GPU_Rect src;
    
    uint32_t id;

    Texture(uint32_t);
    Texture();
    ~Texture();

    void init_from_file(const char*);
    void set_atlas_srcrect(int nbcol, int nbline, int id);
};

#endif