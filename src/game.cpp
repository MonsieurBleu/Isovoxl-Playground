#include <game.hpp>
#include <filesystem>

#include <dirent.h>

bool is_all_numbers(std::string str) {
    for (char c : str) {
        if (c < '0' || c > '9')
            return false; 
    }
    return true;
}

std::list<int>::iterator circularPrev(std::list<int> &l, std::list<int>::iterator &it)
{
    return it == l.begin() ? std::prev(l.end()) : std::prev(it);
}

std::list<int>::iterator circularNext(std::list<int> &l, std::list<int>::iterator &it)
{
    return next(it) == l.end() ? l.begin() : std::next(it);
}

bool sigmoid_callback(Uint64 start, Uint64 last) {
    const Uint64 delay_min = 700; // ms // vitesse de départ quand tlast = tstart => weight = 0
    const Uint64 delay_max = 50; // ms // vitesse de fin quand weight = 1


    const double M = 0.3;
    const double A = 0;
    const double K = 1;
    const double C = 1;
    const double Q = 1;
    const double B = 10;
    const double v = 0.4;
    const double V = 1/v;

    // cf https://www.desmos.com/calculator/wcy0dcf3fx

    const double acceleration_factor = 0.0005; // vitesse a laquelle on augmente la valeur sur l'axe x


    double time_passed = (double)(Get_time_ms() - start) * acceleration_factor;
    double weight = A + (K - A) / pow(C + Q * exp(-B * (time_passed - M)), V);
    
    Uint64 delay = (Uint64)(delay_min - (delay_min - delay_max) * weight);
    
    // std::cout << weight << "\n";
    // std::cout << Get_time_ms() - last << "\n";
    // std::cout << delay << "\n";
    return (Get_time_ms() - last) >= delay;
}

Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp)
    {
        case 1:
            return *p;
            break;

        case 2:
            return *(Uint16 *)p;
            break;

        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;
            break;

            case 4:
                return *(Uint32 *)p;
                break;

            default:
                return 0;       /* shouldn't happen, but avoids warnings */
        }
}

pixel_coord mouse = {0};

Game::Game(GPU_Target* _screen) : 
                                  world(true), 
                                  undo_manager(MAX_UNDO_MEMORY_SIZE_DEFAULT), 
                                  RE(world), 
                                  GameEvent(RE, undo_manager, AE), 
                                  physics_engine(&world, &GameEvent), 
                                  timer_input_undo(sigmoid_callback), 
                                  timer_input_redo(sigmoid_callback) 
{
    world.SetPhysicsEngine(&physics_engine);
    GameEvent.SetPhysicsEngine(&physics_engine); 
    state = STATE_QUIT;
    init(_screen);
}

void Game::init_Render_Engine(GPU_Target* _screen)
{
    // GPU_AddDepthBuffer(_screen);
    // GPU_SetDepthFunction(_screen, GPU_LEQUAL);

    RE.state = &state;

    RE.highlight_mode = HIGHLIGHT_MOD_NONE;
    RE.screen = _screen;
    RE.window.size.x = DEFAULT_WINDOWS_W;
    RE.window.size.y = DEFAULT_WINDOWS_H;
    RE.GameEvent = &GameEvent;
    RE.height_volume_tool = -1;
    RE.window.scale = DEFAULT_SCALE;

    RE.world = world;

    RE.center_camera();
    RE.window.scale = DEFAULT_SCALE;
    RE.refresh_sprite_size();

    RE.max_render_coord.z = (world.max_chunk_coord.z+1)*CHUNK_SIZE;

    // init_resolution();

    // RE.DFIB_FBO = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    // GPU_SetAnchor(RE.DFIB_FBO, 0, 0);
    // RE.DFIB_screen = GPU_LoadTarget(RE.DFIB_FBO);

    // RE.Color_FBO = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    // GPU_SetAnchor(RE.Color_FBO, 0, 0);
    // RE.Color_screen = GPU_LoadTarget(RE.Color_FBO);

    // RE.light_FBO = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    // GPU_SetAnchor(RE.light_FBO, 0, 0);
    // RE.light_screen = GPU_LoadTarget(RE.light_FBO);

    // RE.transparent_FBO = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    // GPU_SetAnchor(RE.transparent_FBO, 0, 0);
    // RE.transparent_screen = GPU_LoadTarget(RE.transparent_FBO);

    // UI.UI_image = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    // GPU_SetAnchor(UI.UI_image, 0, 0);
    // UI.UI = GPU_LoadTarget(UI.UI_image);

    RE.highlight_mode = HIGHLIGHT_MOD_NONE;
    RE.highlight_type = HIGHLIGHT_BLOCKS;

    RE.DFIB_shader.activate();
    GPU_SetShaderImage(RE.Textures[BLOCK_AO]->ptr, RE.DFIB_shader.get_location("ao"), 2);
    GPU_SetShaderImage(RE.Textures[BLOCK_BORDER]->ptr, RE.DFIB_shader.get_location("border"), 5);
    GPU_SetShaderImage(RE.Textures[BLOCK_NORMAL]->ptr, RE.DFIB_shader.get_location("normal"), 6);
    RE.DFIB_shader.deactivate();

    RE.transparent_shader.activate();
    GPU_SetShaderImage(RE.Textures[SHADERTEXT_WATER]->ptr, RE.transparent_shader.get_location("water"), 4);
    GPU_SetShaderImage(RE.Textures[BLOCK_NORMAL]->ptr, RE.transparent_shader.get_location("normal"), 6);
    GPU_SetShaderImage(RE.DFIB_FBO, RE.transparent_shader.get_location("DFIB"), 3);
    RE.transparent_shader.deactivate();

    RE.max_height_render = world.max_block_coord.z;

    AE.start_playlist();
}

void Game::generate_debug_world()
{
    return;
    std::cout << "creating heightmap of size 1024x1024 on GPU : ";
    startbenchrono();

    WG.init_shaders();
    WG.generate_pnoise(0, 1024, 1024);

    endbenchrono();
}

void Game::init(GPU_Target* _screen)
{
#ifdef LOW_RAM_MOD
    std::cout << "\n ##### ISO VOX [LOW RAM MOD] #####\n";
#else 
    std::cout << "\n ##### ISO VOX #####\n";
#endif

    timems = Get_time_ms();

    GameEvent.WG = &WG;

    state = STATE_MAIN_MENU;
    Current_world_name = "/noworld";
    New_world_name = "/noworld";
    To_load_World_name = "/noworld";

    Agent_test = NULL;
    Show_FPS = false;

    curr_scrollmenu_id = -1;
    scrollmenu_id = -1;

    WG.abord_operations = &RE.abort_rendrefresh;

    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    max_framerate = DM.refresh_rate;
    max_frametime = 1000.0/max_framerate;

    Show_HUD = true;
    UI.generate_tiles(-2, _screen->w, _screen->h);
    UI.generate_tiles(-1, _screen->w, _screen->h);
    // UI.generate_tiles(STATE_CONSTRUCTION, _screen->w, _screen->h);
    // UI.generate_tiles(STATE_WORLD_SELECTION, _screen->w, _screen->h);
    // UI.generate_tiles(STATE_MAIN_MENU, _screen->w, _screen->h);
    // UI.generate_tiles(STATE_OPTIONS, _screen->w, _screen->h);

    for(int i = TEXTURE_MIN_ID; i < TEXTURE_MAX_NUMBER; i++)
        RE.Textures[i] = std::make_shared<Texture>(i);

    generate_debug_world();

    init_Render_Engine(_screen);
    load_settings();
    UI.set_ui_hl_mode(HIGHLIGHT_MOD_NONE);

    while(!unlocked_blocks.empty())
        unlocked_blocks.pop_back();

    // unlocked_blocks.push_front(229);
    // unlocked_blocks.push_front(228);
    // unlocked_blocks.push_front(227);
    // unlocked_blocks.push_front(226);
    // unlocked_blocks.push_front(39);
    // unlocked_blocks.push_front(38);
    // unlocked_blocks.push_front(37);
    // unlocked_blocks.push_front(36);
    // unlocked_blocks.push_front(35);
    // unlocked_blocks.push_front(34);
    // unlocked_blocks.push_front(33);
    // unlocked_blocks.push_front(32);
    // unlocked_blocks.push_front(31);
    // unlocked_blocks.push_front(BLOCK_WATER+7);
    // unlocked_blocks.push_front(BLOCK_WATER+6);
    // unlocked_blocks.push_front(BLOCK_WATER+5);
    // unlocked_blocks.push_front(BLOCK_WATER+4);
    // unlocked_blocks.push_front(BLOCK_WATER+3);
    // unlocked_blocks.push_front(BLOCK_WATER+2);
    // unlocked_blocks.push_front(BLOCK_WATER+1);
    // unlocked_blocks.push_front(BLOCK_WATER);
    // unlocked_blocks.push_front(BLOCK_SAND+6);
    // unlocked_blocks.push_front(BLOCK_SAND+5);
    // unlocked_blocks.push_front(BLOCK_SAND+4);
    // unlocked_blocks.push_front(BLOCK_SAND+3);
    // unlocked_blocks.push_front(BLOCK_SAND+2);
    // unlocked_blocks.push_front(BLOCK_SAND+1);
    // unlocked_blocks.push_front(BLOCK_SAND);
    // unlocked_blocks.push_front(1);
    // unlocked_blocks.push_front(2);
    // unlocked_blocks.push_front(3);
    // unlocked_blocks.push_front(4);
    // unlocked_blocks.push_front(5);

    // transparent blocks
    for(int i = 241; i < 256; i++)
        unlocked_blocks.push_front(i);

    // lights & flurescent blocks
    for(int i = 209; i < 241; i++)
        unlocked_blocks.push_front(i);

    // basic blocks + grey/brown, green, blue, red shades
    for(int i = 17; i < 97; i++)
        unlocked_blocks.push_front(i);

    // 
    // for(int i = 1; i < 9; i++)
        // unlocked_blocks.push_front(i);

    bs_max_line = 0;

    auto Current_block = unlocked_blocks.begin();

    for(int i = 0; i < 8; i++)
    {
        Current_block = circularNext(unlocked_blocks, Current_block);
        currentblocks[i] = *Current_block;
        UI.set_ui_current_blocks(i, currentblocks[i]);
    }

    cb_id = 0;

    init_meteos();

    // unlocked_meteo.push_front(METEO_MAIN_MENU);
    unlocked_meteo.push_front(METEO_ANIMATED_SKY);
    unlocked_meteo.push_front(METEO_ANIMATED_DUSK);
    unlocked_meteo.push_front(METEO_NEBULA);
    unlocked_meteo.push_front(METEO_AZUR_AURORA);
    unlocked_meteo.push_front(METEO_JADE_AURORA);
    unlocked_meteo.push_front(METEO_SCARLET_AURORA);
    unlocked_meteo.push_front(METEO_ORCHID_AURORA);
    Current_meteo = unlocked_meteo.begin();

    RE.set_global_illumination(meteos[METEO_MAIN_MENU].global_illumination);
    RE.background_shader = meteos[METEO_MAIN_MENU].background_shader;
    RE.background_shader_data = meteos[METEO_MAIN_MENU].add_data;

    // GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
    // GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);

    RE.projection_grid.refresh_all_identical_line();

    float basic_gi[4] = {1, 1, 1, 0.60};
    // float basic_gi[4] = {0.25, 0.25, 0.35, 0.60};
    RE.set_global_illumination(basic_gi);

    GameEvent.add_event(GAME_EVENT_CAMERA_MOUVEMENT, (pixel_coord){-50000, -50000});

    world.world_view_position = 0;
}

void Game::init_resolution(bool custom_res, Uint16 x, Uint16 y)
{
    // bool borderless = false;

    if(!custom_res)
    {
    if(current_res == -1)
        {
            GPU_SetFullscreen(false, false);
            SDL_DisplayMode DM;
            SDL_GetCurrentDisplayMode(0, &DM);
            x = DM.w;
            y = DM.h;
            // borderless = true;
        }
        else
        {
            x = resolutions[current_res].w;
            y = resolutions[current_res].h;
        }
    }

    // GPU_FreeTarget(RE.screen);
    GPU_FreeTarget(RE.DFIB_screen);
    GPU_FreeTarget(RE.Color_screen);
    GPU_FreeTarget(RE.light_screen);
    GPU_FreeTarget(RE.transparent_screen);
    GPU_FreeTarget(RE.hl_screen);
    GPU_FreeTarget(RE.hld_screen);

    GPU_FreeImage(RE.DFIB_FBO);
    GPU_FreeImage(RE.Color_FBO);
    GPU_FreeImage(RE.light_FBO);
    GPU_FreeImage(RE.transparent_FBO);
    GPU_FreeImage(RE.hl_FBO);
    GPU_FreeImage(RE.hld_FBO);

    // if(!borderless)
    GPU_SetWindowResolution(x, y);

    RE.DFIB_FBO = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    GPU_SetAnchor(RE.DFIB_FBO, 0, 0);
    RE.DFIB_screen = GPU_LoadTarget(RE.DFIB_FBO);

    RE.Color_FBO = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    GPU_SetAnchor(RE.Color_FBO, 0, 0);
    RE.Color_screen = GPU_LoadTarget(RE.Color_FBO);

    RE.light_FBO = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    GPU_SetAnchor(RE.light_FBO, 0, 0);
    RE.light_screen = GPU_LoadTarget(RE.light_FBO);

    RE.transparent_FBO = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    GPU_SetAnchor(RE.transparent_FBO, 0, 0);
    RE.transparent_screen = GPU_LoadTarget(RE.transparent_FBO);

    RE.hl_FBO = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    GPU_SetAnchor(RE.hl_FBO, 0, 0);
    RE.hl_screen = GPU_LoadTarget(RE.hl_FBO);

    RE.hld_FBO = GPU_CreateImage(RE.screen->w, RE.screen->h, GPU_FORMAT_RGBA);
    GPU_SetAnchor(RE.hld_FBO, 0, 0);
    RE.hld_screen = GPU_LoadTarget(RE.hld_FBO);

    GPU_AddDepthBuffer(RE.DFIB_screen);
    GPU_SetDepthFunction(RE.DFIB_screen, GPU_LEQUAL);

    GPU_AddDepthBuffer(RE.transparent_screen);
    GPU_SetDepthFunction(RE.transparent_screen, GPU_LEQUAL);

    GPU_AddDepthBuffer(RE.hl_screen);
    GPU_SetDepthFunction(RE.hl_screen, GPU_LEQUAL);

    GPU_AddDepthBuffer(RE.hld_screen);
    GPU_SetDepthFunction(RE.hld_screen, GPU_LEQUAL);

    // GPU_AddDepthBuffer(RE.Color_screen);
    // GPU_SetDepthFunction(RE.Color_screen, GPU_ALWAYS);

    // UI.generate_tiles(-3, RE.screen->w, RE.screen->h);
    UI.generate_tiles(STATE_CONSTRUCTION, RE.screen->w, RE.screen->h);
    UI.generate_tiles(STATE_WORLD_SELECTION, RE.screen->w, RE.screen->h);
    UI.generate_tiles(STATE_MAIN_MENU, RE.screen->w, RE.screen->h);
    UI.generate_tiles(STATE_OPTIONS, RE.screen->w, RE.screen->h);

    // UI.set_option(GPU_GetFullscreen(), IDMENU_OPTIONS_FULLSCREEN);
    UI.set_option((RE.shader_features&SFEATURE_AMBIANT_OCCLUSION) != 0, IDMENU_OPTIONS_AO);
    UI.set_option((RE.shader_features&SFEATURE_SHADOWS          ) != 0, IDMENU_OPTIONS_SHADOWS);
    UI.set_option((RE.shader_features&SFEATURE_BLOCK_BORDERS    ) != 0, IDMENU_OPTIONS_BLOCKS_BORDER);
    UI.set_option(vsync, IDMENU_OPTIONS_VSYNC);
    UI.set_currres(current_res);

    for(int i = 0; i < 8; i++)
        UI.set_ui_current_blocks(i, currentblocks[i]);

    if(!GPU_GetFullscreen())
        SDL_SetWindowPosition(SDL_GetWindowFromID(RE.screen->context->windowID), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    
    SDL_GL_SetSwapInterval(vsync);
}

int Game::save_settings()
{
    FILE* file = fopen(SETTINGS_FILENAME.c_str(), "wb");

    if (!file)
    {
        std::cout << "Error opening file " << SETTINGS_FILENAME << "\n";
        fclose(file);
        return SAVE_ERROR_FILE_NOT_OPEN;
    }

    bool fullscreen = GPU_GetFullscreen();

    fwrite(&RE.shader_features, sizeof(Uint32), 1, file);
    fwrite(&current_res, sizeof(int8_t), 1, file);
    fwrite(&vsync, sizeof(int8_t), 1, file);
    fwrite(&fullscreen, sizeof(bool), 1, file);

    int8_t tmp = Mix_VolumeMusic(-1);
    fwrite(&tmp, sizeof(int8_t), 1, file);

    tmp = Mix_Volume(-1, -1);
    fwrite(&tmp, sizeof(int8_t), 1, file);

    keybutton *tmpptr = (keybutton *)(&Cinpt);
    for(Uint64 i = 0; i < sizeof(Construction_cinputs)/sizeof(keybutton); i++)
        fwrite(&tmpptr[i].code, sizeof(SDL_Scancode), 1, file);

    //Construction_cinputs

    fclose(file);
    return SAVE_ERROR_NONE;
}   

int Game::load_settings()
{
    FILE* file = fopen(SETTINGS_FILENAME.c_str(), "rb");

    if (!file)
    {
        std::cout << "Error opening file " << SETTINGS_FILENAME << "\n";

        vsync = 0;
        current_res = -1;
        RE.shader_features = -1;

        init_resolution();

        fclose(file);
        return SAVE_ERROR_FILE_NOT_OPEN;
    }

    bool fullscreen;

    fread(&RE.shader_features, sizeof(Uint32), 1, file);
    fread(&current_res, sizeof(int8_t), 1, file);
    fread(&vsync, sizeof(int8_t), 1, file);
    fread(&fullscreen, sizeof(bool), 1, file);

    int8_t tmp;
    fread(&tmp, sizeof(int8_t), 1, file);
    AE.set_music_volume(tmp*(100/128.0));

    fread(&tmp, sizeof(int8_t), 1, file);
    AE.set_sound_volume(tmp*(100/128.0));

    keybutton *tmpptr = (keybutton *)(&Cinpt);
    for(Uint64 i = 0; i < sizeof(Construction_cinputs)/sizeof(keybutton); i++)
        fread(&tmpptr[i].code, sizeof(SDL_Scancode), 1, file);

    if(fullscreen && !GPU_GetFullscreen())
        GPU_SetFullscreen(true, false);
    
    init_resolution();

    fclose(file);

    return SAVE_ERROR_NONE;
}

int Game::load_world_extras(std::string filename, world_extras* extras) {
    std::string total_filename = "saves/";
    total_filename.append(filename);
    total_filename.append("/extras.bin");

    FILE* file = fopen(total_filename.c_str(), "rb");

    if(!file) {
        return SAVE_ERROR_FILE_NOT_OPEN;
    }

    fread(extras, sizeof(world_extras), 1, file);

    fclose(file);

    return SAVE_ERROR_NONE;
}

int Game::save_world_extras(std::string filename, world_extras& extras)
{
    std::string total_filename = "saves/";
    total_filename.append(filename);
    total_filename.append("/extras.bin");

    FILE* file = fopen(total_filename.c_str(), "wb");

    if(!file) {
        return SAVE_ERROR_FILE_NOT_OPEN;
    }

    fwrite(&extras, sizeof(world_extras), 1, file);

    fclose(file);

    return SAVE_ERROR_NONE;
}

void Game::world_extras_apply(world_extras& extras, world_extras_select extras_select)
{
    if (extras_select.camera_pos)
        RE.target = extras.camera_pos; // peut être il faut d'autres trucs jsp

    if (RE.window.scale != extras.scale && extras_select.scale) {
        GameEvent.add_event(GAME_EVENT_NEWSCALE, extras.scale);
    }

    if (world.world_view_position != extras.world_view_position && extras_select.world_view_position) {
        world.world_view_position = extras.world_view_position;
    }

    if (extras_select.meteo)
    {
        Current_meteo = std::find(unlocked_meteo.begin(), unlocked_meteo.end(), extras.meteoid);

        RE.set_global_illumination(meteos[extras.meteoid].global_illumination);
        RE.background_shader = meteos[extras.meteoid].background_shader;
        RE.background_shader_data = meteos[extras.meteoid].add_data;
    }

    // if(extras.lights)
    // {
    //     memcpy(world.lights.buff, extras.lights, sizeof(light_block)*MAX_LIGHT_NUMBER);
    // }
}

void Game::world_extras_fill(world_extras& extras)
{
    extras.camera_pos = RE.target;
    extras.scale = RE.window.scale;
    extras.world_view_position = world.world_view_position;
    extras.meteoid = *Current_meteo;
    
    // memcpy(extras.lights, world.lights.buff, sizeof(light_block)*MAX_LIGHT_NUMBER);
}

int Game::load_world(std::string filename, 
               bool new_size, 
               bool recenter_camera, 
               world_extras* extras, 
               world_extras_select extras_select)
{
    // Uint64 start = Get_time_ms();
    // Uint64 end;

    // physics_engine.clear_events();

    undo_manager.clear();

    std::string total_filename = "saves/";

    total_filename.append(filename);

    total_filename.append("/world.isosave");

    int status = world.load_from_file(total_filename.c_str());

    if(status == 0) 
    {
        if(new_size)
        {
            // RE.projection_grid.free_pos();    
            RE.projection_grid.init_pos(world.max_block_coord.x, world.max_block_coord.y, world.max_block_coord.z);
        }

        if (extras != nullptr) {
            load_world_extras(filename, extras);
            if (extras_select) {
                world_extras_apply(*extras, extras_select);
            }
        }
        if (extras_select) {
            world_extras we = {0};
            load_world_extras(filename, &we);
            world_extras_apply(we, extras_select);
        }


        // std::cout << "world load successfully :)\n";

        // RE.world = world;
        // world.compress_all_chunks();

        RE.max_height_render = world.max_block_coord.z;

        RE.set_global_illumination_direction();

        if(recenter_camera)
        {
            world.find_highest_nonemptychunk();
            RE.window.scale = DEFAULT_SCALE;
            RE.refresh_sprite_size();
            RE.center_camera();
            // refresh_world_render_visible_fast();
            refresh_world_render_fast();
        }
        else
        {
            refresh_world_render_fast();
        }

        world.find_highest_nonemptychunk();
        RE.max_height_render = (world.highest_nonemptychunk+1)*CHUNK_SIZE;

        Current_world_name = filename;
    }

    return status;
}

void Game::create_new_world(coord3D size, std::string &name)
{
    World new_world;

    new_world.init(size.x, size.y, size.z);

    int id;

    for(int x = 0; x <= new_world.max_chunk_coord.x; x++)
    for(int y = 0; y <= new_world.max_chunk_coord.y; y++)
    // for(int z = 0; z <= new_world.max_chunk_coord.z; z++)
    {
        if((x+y)%2)
            id = 31;
        else 
            id = 32;

        memset(new_world.chunks[x][y][0].blocks, id, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(block));
    }

    new_world.compress_all_chunks();

    std::string total_filename = "saves/";
    total_filename.append(name);
    total_filename.append("/world.isosave");

    int status = new_world.save_to_file(total_filename);

    if(status == 0)
    {
        world_extras we;
        world_extras_fill(we);
        save_world_extras(name, we);
        std::cout << "world saved at " << total_filename << "\n";
    }
    else
    {
        std::cout << status << " : failded to save world :(\n";
    }
}

void Game::refresh_world_render_visible()
{
    GameEvent.drop_all_nfs_event();

    RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);
    RE.projection_grid.clear();
    RE.projection_grid.save_curr_interval();
    RE.refresh_pg_onscreen();
}

void Game::refresh_world_render_visible_fast()
{
    GameEvent.drop_all_nfs_event();

    RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);
    RE.projection_grid.clear();
    RE.projection_grid.save_curr_interval();
    GameEvent.add_nfs_event(NFS_OP_PG_ONSCREEN);
}

void Game::refresh_world_render()
{
    GameEvent.drop_all_nfs_event();

    RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);
    RE.projection_grid.clear();
    RE.projection_grid.save_curr_interval();
    RE.refresh_pg_onscreen();
    RE.refresh_pg_block_visible();
    // RE.projection_grid.refresh_all_identical_line();
    // RE.render_world(true);
    // RE.render_transparent_world(true);
    GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
    GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);
}

void Game::refresh_world_render_fast()
{
    GameEvent.drop_all_nfs_event();
    RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);
    RE.projection_grid.save_curr_interval();

    GameEvent.add_nfs_event(NFS_OP_PG_ONSCREEN);
    GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
    GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);   
}

void Game::undo() {
    chunk_save cs = undo_manager.undo();

    if (cs.size == 0 && cs.csize == 0) return;

    AE.Play_woosh();

    // chunk_save* new_cs = world.load_chunk_save(cs);
    // undo_manager.add_redo(*new_cs);

    world.load_chunk_save(cs);
    undo_manager.add_redo(cs);

    if ((cs.size + cs.csize) > 30) {
        refresh_world_render_fast();
    }
    else {
        chunk_coordonate corner1 = cs.get_smallest_chunk_coord_sum();
        chunk_coordonate corner2 = cs.get_highest_chunk_coord_sum();

        block_coordonate start_bc(0, 0, 0, corner1);
        block_coordonate end_bc(CHUNK_SIZE-1, CHUNK_SIZE-1, CHUNK_SIZE-1, corner2);

        world_coordonate start_wc = start_bc.to_coord3D();
        world_coordonate end_wc = end_bc.to_coord3D();

        start_wc.x -= 2;
        start_wc.y -=  2;

        end_wc.x += 2;
        end_wc.y += 2;

        RE.refresh_cuboid(start_wc, end_wc);
    }

    world.check_light_trash_bin();

    // delete new_cs;
}

void Game::redo() {
    chunk_save cs = undo_manager.redo();

    if (cs.size == 0 && cs.csize == 0) return;

    AE.Play_woosh(true);

    // chunk_save* new_cs = world.load_chunk_save(cs);
    // undo_manager.add_undo(*new_cs, false);

    world.load_chunk_save(cs);
    undo_manager.add_undo(cs, false);

    if ((cs.size + cs.csize) > 30) {
        refresh_world_render_fast();
    }
    else {
        chunk_coordonate corner1 = cs.get_smallest_chunk_coord_sum();
        chunk_coordonate corner2 = cs.get_highest_chunk_coord_sum();

        block_coordonate start_bc(0, 0, 0, corner1);
        block_coordonate end_bc(CHUNK_SIZE-1, CHUNK_SIZE-1, CHUNK_SIZE-1, corner2);

        world_coordonate start_wc = start_bc.to_coord3D();
        world_coordonate end_wc = end_bc.to_coord3D();

        start_wc.x -= 2;
        start_wc.y -=  2;

        end_wc.x += 2;
        end_wc.y += 2;
        
        RE.refresh_cuboid(start_wc, end_wc);
    }

    world.check_light_trash_bin();

    // delete new_cs;
}

void Game::switch_fullscreen()
{
    if(GPU_SetFullscreen(!GPU_GetFullscreen(), false))
    {
        UI.set_option(1, IDMENU_OPTIONS_FULLSCREEN);
    }
    else
    {
        UI.set_option(0, IDMENU_OPTIONS_FULLSCREEN);
    }
}

void Game::switch_AO()
{
    RE.shader_features ^= SFEATURE_AMBIANT_OCCLUSION;
    UI.set_option((RE.shader_features&SFEATURE_AMBIANT_OCCLUSION) != 0, IDMENU_OPTIONS_AO);
}

void Game::switch_shadows()
{
    RE.shader_features ^= SFEATURE_SHADOWS;
    UI.set_option((RE.shader_features&SFEATURE_SHADOWS) != 0, IDMENU_OPTIONS_SHADOWS);
}

void Game::switch_borders()
{
    RE.shader_features ^= SFEATURE_BLOCK_BORDERS;
    UI.set_option((RE.shader_features&SFEATURE_BLOCK_BORDERS) != 0, IDMENU_OPTIONS_BLOCKS_BORDER);
}

void Game::refresh_meteo()
{
    RE.set_global_illumination(meteos[*Current_meteo].global_illumination);
    RE.background_shader = meteos[*Current_meteo].background_shader;
    RE.background_shader_data = meteos[*Current_meteo].add_data;
}

bool Game::set_meteo(int val)
{
    Current_meteo = std::find(unlocked_meteo.begin(), unlocked_meteo.end(), val);
    if(Current_meteo == unlocked_meteo.end())
        return false;
    else
        return true;
}

void Game::switch_to_construction()
{
    menu_selectionables[0] = -1;
    menu_selectionables[1] = -1;
    menu_selected[1] = (int *)&Current_meteo;

    UI.set_ui_hl_mode(RE.highlight_mode);
    state = STATE_CONSTRUCTION;
}

int Game::mainloop()
{
    while(state)
    {
        init_framerate();

        timems = Get_time_ms() - timems_start;
        
        input();
        GameEvent.handle();
        AE.handle_playlist();

        RE.render_frame(state != STATE_MAIN_MENU && state != STATE_OPTIONS && state != STATE_QUIT);

        // if(0)
        if(Show_HUD || state != STATE_CONSTRUCTION)
            UI.render_frame(state, RE.screen, 
                            cb_id, new_cb_id,
                            bs_max_line, 
                            new_current_block, 
                            new_hl_type,
                            current_res,
                            scrollmenu_id,
                            curr_scrollmenu_id,
                            menu_selectionables,
                            menu_selected,
                            unlocked_blocks, 
                            New_world_name, Current_world_name , 
                            Menu_hl_name, Cinpt, &RE, &AE);


        GPU_Flip(RE.screen);
        handle_framerate();
    }

    GameEvent.game_is_running = false;
    GameEvent.handle();

    SDL_WaitThread(RE.SecondaryThread, NULL);

    GameEvent.drop_all_nfs_event();
    SDL_CondSignal(GameEvent.new_nfs_event);
    SDL_WaitThread(GameEvent.NFS_Thread, NULL);

    return state;
}