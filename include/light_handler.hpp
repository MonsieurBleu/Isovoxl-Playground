#ifndef LIGHT_HANDLER_HPP
#define LIGHT_HANDLER_HPP

#define MAX_LIGHT_NUMBER 2048 //512

#include <blocks.hpp>
#include <coords.hpp>
#include <list>

#define LIGHT_BUFFER_FULL 1
#define LIGHT_DUPLICATE   2

extern Uint64 timems;

struct light_block
{
    Uint8 id;
    block_coordonate pos;
};

// typedef light_block[MAX_LIGHT_NUMBER] LightBuffer;

struct LightHandler
{
    light_block buff[MAX_LIGHT_NUMBER];
    std::list<light_block> trash_bin;

    bool is_full = false;

    Uint64 time_since_last_full;

    int add(light_block);
};

#endif