#ifndef SPRITES_HPP
#define SPRITES_HPP

#include <world.hpp>
#include <coords.hpp>
#include <multithreaded_event_handler.hpp>
// struct Render_Engine;

#define SPRITES_WIDTH 16
#define SPRITES_HEIGHT 24

#define SPRITE_TO_SMALL_MOVE -1
#define SPRITE_OBSTACLE_FOND 0

extern Uint64 timems;

struct sprite_voxel
{
    int id;
    bool is_occluded;
    coord3D wcoord;
    // coord3D scoord;
};

struct animation_frame
{
    coord2D pos;
    Uint32 duration;
};

class animation_loop
{
    private :
        Uint64 duration; //(ms)
        Uint64 init_time; //(ms)
        int loop_nb;
        std::vector<animation_frame> frames;

    public :
        animation_loop();
        void init(int _loop_nb);
        void refresh_init_time();
        void addframe(animation_frame new_animframe);
        coord2D get_curr_frame();
};

class Agent;
class Ant;

class Sprite
{
    friend Agent;
    friend Ant;

    private :
        sprite_voxel voxels[SPRITES_WIDTH][SPRITES_WIDTH][SPRITES_HEIGHT];
        fcoord3D subvoxel_pos;
        fcoord3D velocity;
        Uint64 time_since_last_vel;

        World *world;
        Render_Engine *RE;
        Multithreaded_Event_Handler *GameEvent;

        World frames;

        coord3D pos;
        void compress_chunks();
        void set_frame(int x, int y);
        int rotation;

        animation_loop idle;
        animation_loop running;
        animation_loop *curr_anim;
        coord2D current_frame;

        bool _isinit;

        coord3D check_zone(int xbeg, 
                           int xend, 
                           int ybeg, 
                           int yend, 
                           int zbeg, 
                           int zend, 
                           bool reversex = false,
                           bool reversey = false);


        void convert_wvp_vel(fcoord3D &vel);

    public :

        Sprite();

        bool isinit();

        void init_sprite(Multithreaded_Event_Handler *_GameEvent, 
                         World *_world, 
                         Render_Engine *_RE,
                         std::string& name);

        int move(fcoord3D vel, bool sprite_rotation = false, 
                                bool clipping = false, 
                                bool wvp_movement = false,
                                bool apply_movement = true,
                                bool apply_timed_velocity = true);

        int tp(fcoord3D fcoord);
        bool refresh_animation();

        void update();
        void remove();
        void refresh_display();

        void print_debug_info();

        void rotate_left();
        void rotate_right();
};


#endif