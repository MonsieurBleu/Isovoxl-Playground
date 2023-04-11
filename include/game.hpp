#ifndef GAME_HPP
#define GAME_HPP

#include <iostream>
#include <memory>
#include <math.h>
#include <chrono>
#include <list>
#include <filesystem>
#include <algorithm>
#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>

// struct Render_Engine;
// #include <world.hpp>
#include <render_engine.hpp>
#include <ui_engine.hpp>
#include <multithreaded_event_handler.hpp>
#include <meteo.hpp>
#include <containers.hpp>
#include <world_generator.hpp>
#include <audio_engine.hpp>

#include <Agent.hpp>


extern pixel_coord mouse;

extern Uint64 timems_start;
extern Uint64 timems;

struct world_extras
{
    pixel_coord camera_pos;
    float scale;
    int world_view_position;
    int meteoid;
};

struct world_extras_select {
    bool camera_pos;
    bool scale;
    bool world_view_position;
    bool meteo;

    world_extras_select(bool value = false) {
        memset(this, (int)value, sizeof(*this)); // set all to value, cursed level 7/10
    }

    operator bool() { // check if any is true
        world_extras_select tmp(false);
        return memcmp(this, &tmp, sizeof(*this)); 
    }
};

typedef bool (*event_timer_callback)(Uint64, Uint64); // timer callback function pointer

struct repeat_event_timer {
    Uint64 start; // start time in ms
    Uint64 last; // last time the event was repeated
    bool enabled; // is the timer enabled
    event_timer_callback callback; // function to call every tick to check if the event should be repeated, argument is the start time in ms and the last time the event was repeated

    repeat_event_timer() : start(0), last(0), enabled(false), callback(nullptr) {}
    repeat_event_timer(event_timer_callback cb) : start(0), last(0), enabled(false), callback(cb) {}
    void start_timer() {
        start = Get_time_ms();
        last = start;
        enabled = true;
    }

    void stop_timer() {
        enabled = false;
    }

    bool tick() {
        if (enabled) {
            bool val = callback(start, last);
            if (val) last = Get_time_ms();
            return val;
        }
        return false;
    }
};

bool sigmoid_callback(Uint64 start, Uint64 last);

class Game
{
    private :

        int wvp_tmp;

        // -1 for automatic desktop resolution
        int8_t current_restmp;
        int8_t current_res;
        int8_t vsync;
        float max_framerate;
        float max_frametime;

        Uint16 state;

        int new_current_block;
        int new_hl_type;
        int bs_max_line;
        std::list<int> unlocked_blocks;
        std::list<int> unlocked_meteo;
        
        int currentblocks[8];
        int cb_id;
        int new_cb_id;

        int scrollmenu_id;
        int curr_scrollmenu_id;

        int menu_selectionables[16];
        int *menu_selected[16];

        std::list<int>::iterator Current_meteo;
        int Current_HL_type;

        std::string New_world_name;     // the world name under the cursor
        std::string To_load_World_name; // the last world name the player have selected
        std::string Current_world_name; // the current world loaded in the game

        std::string Menu_hl_name;

        bool Show_HUD;
        bool Show_FPS;

        std::unique_ptr<Agent> Agent_test; 
        Sprite player;

        Construction_cinputs Cinpt;

        World world;
        UI_Engine UI;
        UndoManager undo_manager;
        Audio_Engine AE;
        Render_Engine RE;
        World_Generator WG;
        Multithreaded_Event_Handler GameEvent;
        PhysicsEngine physics_engine;

        repeat_event_timer timer_input_undo;
        repeat_event_timer timer_input_redo;
        
        void init(GPU_Target*);
        void init_Render_Engine(GPU_Target*);

        void init_resolution(bool custom_res = false, Uint16 x = 800, Uint16 y = 600);

        void generate_debug_world();

        int save_settings();
        int load_settings();

        // Loads a world from a savefile and refreshes/initializes everything
        // Argument new_size signals whether or not the projection grid need to be re-allocated,
        // only set it to false if you want fast loading and know what you are doing, true by default
        // if world_extras is not null, it will be filled with the world_extras from the corresponding file
        // if apply_extras is true, the world_extras will be applied to the world
        int load_world(std::string filename,
                       bool new_size = true, 
                       bool recenter_camera = false, 
                       world_extras* extras = nullptr, 
                       world_extras_select extras_select = world_extras_select(false),
                       bool clear_pg = false);

        // saves a world extra given as a parameter to a file
        int save_world_extras(std::string filename, world_extras& extras);

        // loads a world extra from a file and fill the given parameter
        int load_world_extras(std::string filename, world_extras* extras);

        // applies a world_extras to the world / Render Engine / whatever else
        void world_extras_apply(world_extras& extras,
                                world_extras_select extras_select = world_extras_select(true));

        // fills a world_extras with the corresponding data
        void world_extras_fill(world_extras& extras);

        void create_new_world(coord3D size, std::string &name);

        void refresh_world_render();
        void refresh_world_render_fast();
        void refresh_world_render_visible();
        void refresh_world_render_visible_fast();
        
        int input_utils(SDL_Event &event, SDL_Keymod km);
        int input_main_menu(SDL_Event &event, SDL_Keymod km);
        int input_options(SDL_Event &event, SDL_Keymod km);
        int input_controlmenu(SDL_Event &event , SDL_Keymod km);
        int input_controlselect(SDL_Event &event , SDL_Keymod km);
        int input_world_selection(SDL_Event &event, SDL_Keymod km);
        int input_new_map(SDL_Event &event, SDL_Keymod km);
        int input_block_selection(SDL_Event &event, SDL_Keymod km);
        int input_construction(SDL_Event &event, SDL_Keymod km);
        int input_adventure(SDL_Event &event , SDL_Keymod km);
        void input();

        void switch_fullscreen(); //old
        void switch_AO();
        void switch_shadows();
        void switch_borders();

        const Uint8 max_undo_worlds = 10;

        FileCircularBuffer undo_buffer;
        FileCircularBuffer redo_buffer;

        void init_framerate();
        void handle_framerate();

        void refresh_meteo();
        bool set_meteo(int val);
        void switch_to_construction();

        void undo();
        void redo();        

    public :
        Game(GPU_Target*);
        int mainloop();
};

#endif