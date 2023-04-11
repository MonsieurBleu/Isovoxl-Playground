#ifndef PROJECTION_GRID_HPP
#define PROJECTION_GRID_HPP

#include <SDL2/SDL.h>
#include <coords.hpp>
#include <blocks.hpp>

#ifdef LOW_GEOMETRY_MOD
    #define IDENDICAL_LINE_MAX 20
#else
    #define IDENDICAL_LINE_MAX 15
#endif

void set_in_interval(int& x, const int min, const int max);

struct screen_block
{
    block transparent_block;
    block opaque_block;

    Uint16 height;
    Uint16 x;
    Uint16 y;
    Uint16 x_transparent;
    Uint16 y_transparent;
    Uint16 height_transparent;

    bool is_on_screen;

    SDL_Color render_flags;
    SDL_Color render_flags_transparent;

    Uint16 identical_line_counter;
    Uint16 identical_line_counter_transparent;
};

struct interval
{
    int beg;
    int end;
};

struct Projection_grid
{
    public :
        int size[3][2];
        interval visible_frags[3][2];
        interval visible_frags_save[3][2];
        screen_block **pos[3];

        Projection_grid();
        ~Projection_grid();

        void init_pos(const int, const int, const int);
        void free_pos();
        screen_block* get_pos(Uint8, Uint32, Uint32);
        screen_block* get_pos(chunk_coordonate, int, int , int);
        screen_block* get_pos_world(int, int , int);

        chunk_coordonate convert_wcoord(int x, int y, int z);

        void refresh_visible_frags(pixel_coord, Uint16, Uint16, long double);

        void refresh_all_identical_line();

        void save_curr_interval();

        void clear();
};

#endif