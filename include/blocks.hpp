#ifndef BLOCKS_HPP
#define BLOCKS_HPP

#include <SDL2/SDL.h>

#include <constants.hpp>

struct block
{
    Uint8 id;
};

struct chunk
{
    int compress_value;
    block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
};

#endif