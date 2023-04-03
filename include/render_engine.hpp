#ifndef RENDER_ENGINE_HPP
#define RENDER_ENGINE_HPP

#include <memory>
#include <queue>

#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>

struct World;

#include <texture.hpp>
#include <world.hpp>
#include <projection_grid.hpp>
#include <Shader.hpp>
#include <coords.hpp>
#include <constants.hpp>

extern Uint64 timems;

Uint64 Get_time_ms();

class Multithreaded_Event_Handler;

struct Window_Variables
{
        uint16_t FPS_Lock;
        double FrameTime; 
        pixel_coord size;
        float scale;
        SDL_Window *ptr;
};

struct Render_Engine
{   
    Render_Engine(World& _world);
    ~Render_Engine();

    Uint16 const *state;

    SDL_mutex *campg_mut = SDL_CreateMutex();

    SDL_Thread *SecondaryThread;
    Multithreaded_Event_Handler *GameEvent;

    Window_Variables window;

    GPU_Target *screen;

    GPU_Target *DFIB_screen;
    GPU_Image  *DFIB_FBO;

    GPU_Target *Color_screen;
    GPU_Image  *Color_FBO;

    GPU_Target *light_screen;
    GPU_Image *light_FBO;

    GPU_Target *transparent_screen;
    GPU_Image  *transparent_FBO;

    GPU_Target *hl_screen = NULL;
    GPU_Image  *hl_FBO = NULL;

    GPU_Target *hld_screen = NULL;
    GPU_Image  *hld_FBO = NULL;

    // GPU_Target

    World &world;

    pixel_coord target;

    block_coordonate highlight_coord; // old
    world_coordonate highlight_wcoord;
    world_coordonate highlight_wcoord2;

    chunk_coordonate max_render_coord;

    int highlight_mode;
    int highlight_type;

    bool grid_enable;
    bool low_gpu_mode_enable;
    bool abort_rendrefresh;
    bool force_nonpanorama_mode;
    bool test_panorama_mode();

    Shader DFIB_shader;
    Shader transparent_shader;
    Shader world_render_shader;
    Shader post_process_shader;
    Shader light_shader;
    std::shared_ptr<Shader> background_shader;
    int background_shader_data;

    std::queue<screen_block*> lights;
    void render_lights();

    float global_illumination[4];
    float gi_direction[3];

    std::shared_ptr<Texture> Textures[TEXTURE_MAX_NUMBER] = {NULL};

    void refresh_sprite_size();
    long double block_onscreen_size;
    long double block_onscreen_half;
    long double block_onscreen_quarter;
    
    int height_volume_tool;
    int max_height_render;
    
    Projection_grid projection_grid;

    bool shader_enable;
    unsigned int shader_features;
    unsigned int sprite_counter;
    unsigned int world_sprite_counter;
    unsigned int transparent_sprite_counter;
    unsigned int light_counter;

    /******** RENDER ***********************/
    // old
    void render_grid();

    void render_world(bool use_visible_frags = false);
    Uint8 render_block
         (const chunk_coordonate &);

    void render_transparent_world(bool use_visible_frags = false);
    Uint8 render_transparent_block
         (const chunk_coordonate &);


    void render_hl_block(coord3D pos, 
                         bool hide_left = false, 
                         bool hide_right = false,
                         bool hide_top = false);
                         
    void render_all_highlighted_blocks();

    void send_primitives_buffer(GPU_Target *screen, 
                                GPU_Image *img, 
                                Uint32* primbuff,
                                Uint32 & sprite_cnt);

    void render_frame(bool);
    /***************************************/

    /******** RENDER FLAGS *****************/
    void refresh_all_render_flags2();
    void set_block_renderflags(char, int, int);
    void refresh_block_render_flags
         (const chunk_coordonate&, 
          const int, const int, const int);
    /***************************************/
    
    /******** BLOCK VISIBLE ****************/
    void refresh_pg_block_visible();

    void refresh_all_block_visible2();
    void refresh_line_visible2(int, int, int);
    void refresh_block_visible
         (const chunk_coordonate&, 
          const int, const int, const int);
    /***************************************/

    /******** SHADOWS **********************/
    void set_shadow_context(SDL_Color&, int, int, int);
    void set_block_shadow_context2(int, int, int);

    void refresh_line_shadows(int, int, int, int);
    void refresh_line_shadows(coord3D beg, coord3D end);
    /***************************************/

    void highlight_block2();
    // old
    void refresh_pg_MHR();

    // entierly refresh the projection grid (pg) only for the blocks on screen
    // pg isn't clear before doing anything
    void refresh_pg_onscreen();

    // Go thought all the projection grid, find the screens blocks with an height higher than the max height and refresh them
    void refresh_height_render();
    bool reverse_rhr = false;

    void refresh_cuboid(world_coordonate start, world_coordonate end);

    // setup everything for the rotation of the camera
    // give it +1 or -1 depending of the direction of the rotation
    // does not refresh the projection grid!
    void rotate_camera(int);
    void center_camera();

    void set_global_illumination_direction();
    void set_global_illumination(float[4]);

    void refresh_shaders();
};


#endif