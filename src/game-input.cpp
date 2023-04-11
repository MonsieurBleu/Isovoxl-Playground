#include <game.hpp>

std::string NW_map_name;

void Game::input()
{
    bool refresh_wr = false;
    int inptcode = INPUT_UNUSED;

    SDL_Event event;
    SDL_Keymod km = SDL_GetModState();
    SDL_GetMouseState((int*)&mouse.x, (int*)&mouse.y);

    while(SDL_PollEvent(&event))
    {
        if(event.key.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
        {
            UI.Lclickdown = true;
            if(menu_selectionables[15])
                continue;
        }
        if(event.key.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
            UI.Lclickup = true;

        inptcode = INPUT_UNUSED;

        switch (state)
        {
        case STATE_MAIN_MENU :
            inptcode = input_utils(event, km);
            if(!inptcode) inptcode = input_main_menu(event, km);
            break;

        case STATE_WORLD_SELECTION :
            inptcode = input_utils(event, km);
            if(!inptcode) inptcode = input_world_selection(event, km);
            break;
        
        case STATE_BLOCK_SELECTION :
            inptcode = input_utils(event, km);
            if(!inptcode) inptcode = input_block_selection(event, km);
            break;
        
        case STATE_CONSTRUCTION :
            inptcode = input_utils(event, km);
            if(!inptcode) inptcode = input_construction(event, km);
            break;

        case STATE_ADVENTURE :
            inptcode = input_utils(event, km);
            if(!inptcode) inptcode = input_adventure(event, km);
            break;
        
        case STATE_OPTIONS :
            inptcode = input_utils(event, km);
            if(!inptcode) inptcode = input_options(event, km);
            break;

        case STATE_CONTROLS :
            inptcode = input_utils(event, km);
            if(!inptcode) inptcode = input_controlmenu(event, km);
            break;
        
        case STATE_CONTROLS_SELECT :
            inptcode = input_controlselect(event, km);
            break;

        case STATE_NEW_MAP :
            // inptcode = input_utils(event, km);
            // if(!inptcode) 
            inptcode = input_new_map(event, km);
            break;

        default:
            break;
        }

        if(inptcode&INPUT_REFRESH_WORLD_RENDER)
            refresh_wr = true;

        if((inptcode&INPUT_SOUND_PLAYED) == 0 && inptcode != INPUT_UNUSED && UI.Lclickdown == true)
            AE.Play_click();
    }

    if(state == STATE_CONSTRUCTION)
    {
        const Uint8* keystate = SDL_GetKeyboardState(NULL);

        int xcamvel = 0.f;
        int ycamvel = 0.f;
        int camspeed = 10.f;

        if(keystate[Cinpt.CAM_L.code]) xcamvel += camspeed;

        if(keystate[Cinpt.CAM_R.code]) xcamvel -= camspeed;

        if(keystate[Cinpt.CAM_U.code]) ycamvel += camspeed;

        if(keystate[Cinpt.CAM_D.code]) ycamvel -= camspeed;

        if(xcamvel || ycamvel)
            GameEvent.add_event(GAME_EVENT_CAMERA_MOUVEMENT, (pixel_coord){xcamvel, ycamvel});

        if (timer_input_undo.tick()) {
            // std::cout << "undo()" << std::endl;
            undo();
        }

        if (timer_input_redo.tick()) {
            // std::cout << "redo()" << std::endl;
            redo();
        }
    }

    if(Agent_test && state == STATE_ADVENTURE)
        Agent_test->tick(NULL);

    if(refresh_wr)
    {
        if(RE.test_panorama_mode())
            refresh_world_render_fast();
        else
            refresh_world_render();
    }
}

int Game::input_utils(SDL_Event &event, SDL_Keymod km)
{
    int retval = INPUT_UNUSED;

    switch(event.key.type)
    {
        case SDL_KEYDOWN :
            if(event.key.type == SDL_KEYDOWN && event.key.repeat == 0)
                switch(event.key.keysym.sym)
                {
                    case SDLK_F4 : 

                        if(km & KMOD_LALT)
                        {
                            state = STATE_QUIT;
                            save_settings();
                            retval = INPUT_CLEAR;
                        }
                        // else 
                        // {
                        //     Show_FPS = !Show_FPS;
                        // }

                        break;

                    case SDLK_RETURN :
                        if(km & KMOD_LALT)
                        {
                            GPU_SetFullscreen(!GPU_GetFullscreen(), false);
                            retval = INPUT_CLEAR;
                        }
                        break;
                    
                    case SDLK_h :
                            std::cout << "Opaque voxels rendered : " << format(RE.world_sprite_counter) << "\n";
                            std::cout << "Transparent voxels rendered : " << format(RE.transparent_sprite_counter) << "\n";
                            std::cout << "Panorama mode : " << (RE.window.scale < PANORAMA_SCALE_THRESHOLD) << "\n";
                            std::cout << "Current block : " << currentblocks[cb_id] << "\n";
                        break;
                    
                    // case SDLK_F6 :
                    //         RE.low_gpu_mode_enable = !RE.low_gpu_mode_enable;

                    //         if(RE.low_gpu_mode_enable)
                    //             std::cout << "Low GPU mode enable\n";
                    //         else 
                    //         {
                    //             RE.projection_grid.refresh_all_identical_line();
                    //             std::cout << "Low GPU mode disable\n";
                    //         }
                    //     break;

                    case SDLK_p :
                        system("cls");
                        std::cout << "\nrefreshing shaders...";
                        RE.refresh_shaders();
                        RE.DFIB_shader.activate();
                        GPU_SetShaderImage(RE.Textures[BLOCK_AO]->ptr, RE.DFIB_shader.get_location("ao"), 2);
                        GPU_SetShaderImage(RE.Textures[SHADERTEXT_WATER]->ptr, RE.DFIB_shader.get_location("water"), 4);
                        GPU_SetShaderImage(RE.Textures[BLOCK_BORDER]->ptr, RE.DFIB_shader.get_location("border"), 5);
                        GPU_SetShaderImage(RE.Textures[BLOCK_NORMAL]->ptr, RE.DFIB_shader.get_location("normal"), 6);
                        GPU_SetShaderImage(RE.Textures[BLOCK_LIGHT]->ptr, RE.DFIB_shader.get_location("light"), 7);
                        RE.DFIB_shader.deactivate();
                        
                        init_meteos();

                        switch (state)
                        {
                        case STATE_ADVENTURE:
                        case STATE_CONSTRUCTION:
                        case STATE_BLOCK_SELECTION:
                        case STATE_WORLD_SELECTION:
                            RE.set_global_illumination(meteos[*Current_meteo].global_illumination);
                            RE.background_shader = meteos[*Current_meteo].background_shader;
                            RE.background_shader_data = meteos[*Current_meteo].add_data;
                            break;
                        
                        default:
                            RE.set_global_illumination(meteos[METEO_MAIN_MENU].global_illumination);
                            RE.background_shader = meteos[METEO_MAIN_MENU].background_shader;
                            RE.background_shader_data = meteos[METEO_MAIN_MENU].add_data;
                            break;
                        }

                        UI.generate_tiles(-2, RE.screen->w, RE.screen->h);

                        retval = INPUT_CLEAR;
                        break;

                    // case SDLK_KP_MINUS :
                    //     std::cout << "toggle physics engine\n";
                    //     physics_engine.toggle_running();
                    //     break;
                    
                    // case SDLK_KP_PERIOD :
                    //     std::cout << "Physics engine step\n";
                    //     physics_engine.tick();
                    //     break;

                    // case SDLK_F5 :
                    //     system("cls");
                    //     WG.init_shaders();
                    //     WG.generate_pnoise(0, 1024, 1024);
                    // break;
                    
                    // case SDLK_F7 :
                    //     GameEvent.drop_all_nfs_event();
                    // break;

                    default : break;
                }
        default : break;
    }

    return retval;
}

int Game::input_main_menu(SDL_Event &event, SDL_Keymod km)
{
    int retval = INPUT_UNUSED;

    switch(event.key.type)
    {
        case SDL_KEYDOWN :
            if(event.key.type == SDL_KEYDOWN && event.key.repeat == 0)
                switch(event.key.keysym.sym)
                {       
                    case SDLK_ESCAPE :
                        state = STATE_QUIT;
                        retval = INPUT_CLEAR;
                        break;

                    case SDLK_RETURN :
                        if(~km & KMOD_LALT)
                        {
                            state = STATE_WORLD_SELECTION;
                            menu_selectionables[0] = -1;
                            retval = INPUT_CLEAR;
                        }
                        break;

                    default : break;
                }
        
        case SDL_MOUSEBUTTONDOWN :
            if(event.button.button == SDL_BUTTON_LEFT)
            {
                if(Menu_hl_name == IDMENU_QUIT)
                {
                    state = STATE_QUIT;
                    retval = INPUT_CLEAR;
                }
                
                else if(Menu_hl_name == IDMENU_PLAY)
                {
                    state = STATE_WORLD_SELECTION;
                    menu_selectionables[0] = -1;
                    RE.projection_grid.clear();
                    retval = INPUT_CLEAR;
                }

                else if(Menu_hl_name == IDMENU_OPTIONS)
                {
                    state = STATE_OPTIONS;
                    current_restmp = current_res;
                    retval = INPUT_CLEAR;
                }
            }
            break;

        default : break;
    }

    return retval;
}

int menu_selected_buff[16] = {-1};

int Game::input_new_map(SDL_Event &event, SDL_Keymod km)
{
    int retval = INPUT_UNUSED;

    bool wg_changes = false;

    switch(event.key.type)
    {
        case SDL_KEYDOWN :
            if(event.key.type == SDL_KEYDOWN && event.key.repeat == 0)
                switch(event.key.keysym.sym)
                {       
                    case SDLK_ESCAPE :
                        state = STATE_WORLD_SELECTION;
                        world.world_view_position = wvp_tmp;
                        menu_selectionables[0] = -1;
                        retval = INPUT_CLEAR;
                        SDL_StopTextInput();
                        break;
                    
                    // case SDLK_KP_ENTER :
                    //     switch_to_construction();
                    //     retval = INPUT_CLEAR;
                    //     break;

                    default : break;
                }
            if(event.key.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_BACKSPACE)
            {
                if(!NW_map_name.empty())
                {
                    NW_map_name.pop_back();
                    retval = INPUT_CLEAR;

                    if(std::filesystem::exists("saves/"+NW_map_name))
                        menu_selected[4] = WORLD_NAME_EXIST;
                    else
                        menu_selected[4] = WORLD_NAME_OK;
                }
            }
            break;

        case SDL_MOUSEMOTION :
            if(event.motion.state == SDL_BUTTON_RMASK)
            {
                GameEvent.add_event(GAME_EVENT_CAMERA_MOUVEMENT, (pixel_coord){event.motion.xrel, event.motion.yrel});
                retval = INPUT_CLEAR;
            }
            break;
        
        case SDL_MOUSEWHEEL :
            GameEvent.add_event(GAME_EVENT_ADDSCALE, event.wheel.y);
            retval = INPUT_CLEAR;
            break;

        case SDL_MOUSEBUTTONDOWN :
            if(event.button.button == SDL_BUTTON_LEFT)
            {
                if(menu_selectionables[5] == 0)
                {
                    switch_to_construction();
                    Current_world_name = NW_map_name;
                    std::filesystem::create_directory("saves/" + NW_map_name);
                    std::filesystem::create_directory("saves/" + NW_map_name + "/undo_worlds");
                    std::filesystem::create_directory("saves/" + NW_map_name + "/redo_worlds");
                    world.save_to_file("saves/" + NW_map_name + "/world.isosave");
                    world_extras we;
                    world_extras_fill(we);
                    save_world_extras(NW_map_name, we);

                    UI.generate_tiles(STATE_WORLD_SELECTION, RE.screen->w, RE.screen->h);

                    retval = INPUT_CLEAR;
                }
                else
                {
                    for(int i = 0; i < 3; i++)
                    {
                        if(menu_selectionables[i] != -1 && menu_selectionables[i] != *menu_selected[i])
                        {
                            *menu_selected[i] = menu_selectionables[i];
                            wg_changes = true;
                        }
                    }
                    retval = INPUT_CLEAR;
                }
            }
            break;

        case SDL_TEXTINPUT :
        {
            char newc = event.text.text[0];

            if(
                (newc >= 'a' && newc <= 'z') ||
                (newc >= 'A' && newc <= 'Z') ||
                (newc >= '0' && newc <= '9') ||
                (newc == ' ' && NW_map_name.size()) ||
                newc == '-' ||
                newc == '_'
            )
            {
                if(NW_map_name.size() < WORLD_NAME_MAX_LENGHT)
                NW_map_name += newc;

                if(std::filesystem::exists("saves/"+NW_map_name))
                    menu_selected[4] = WORLD_NAME_EXIST;
                else
                    menu_selected[4] = WORLD_NAME_OK;
            }
        }
        break;

        default : break;
    }

    if(*menu_selected[0] != WG.world_size_id || 
       *menu_selected[1] != WG.biome_id ||
       *menu_selected[2] != WG.preset_id)
        wg_changes = true;

    if(state == STATE_NEW_MAP && wg_changes && *menu_selected[0] != -1 && *menu_selected[1] != -1 && *menu_selected[2] != -1)
    {
        GameEvent.drop_all_nfs_event();

        if(!SDL_TryLockMutex(GameEvent.world_and_projection_grid_mut))
        {
            Current_world_name = "/noworld";

            bool newsize = WG.world_size_id != *menu_selected[0];
            world.free_chunks();
            if(newsize)
            {
                RE.projection_grid.free_pos();
            }

            WG.world_size_id = *menu_selected[0];
            WG.biome_id      = *menu_selected[1];
            WG.preset_id     = *menu_selected[2];

            int biome_meteo_id = METEO_MAIN_MENU;
            switch (WG.biome_id)
            {
                case BIOME_MOUNTAINS :
                case BIOME_PLAINS : 
                    biome_meteo_id = METEO_ANIMATED_SKY; break;

                case BIOME_DUNES :
                case BIOME_CANYONS : 
                    biome_meteo_id = METEO_ANIMATED_DUSK; break;

                case BIOME_SNOWVALLEY :
                case BIOME_ICEPEAKS : 
                    biome_meteo_id = METEO_AZUR_AURORA; break;

                case BIOME_DARK : 
                    biome_meteo_id = METEO_NEBULA; break;

                case BIOME_FANTASY : 
                    biome_meteo_id = METEO_ANIMATED_SKY; break;
            
                case BIOME_FLAT :
                    biome_meteo_id = METEO_ANIMATED_SKY; break;

                default: break;
            }

            Current_meteo = std::find(unlocked_meteo.begin(), unlocked_meteo.end(), biome_meteo_id);

            RE.set_global_illumination(meteos[biome_meteo_id].global_illumination);
            RE.background_shader = meteos[biome_meteo_id].background_shader;
            RE.background_shader_data = meteos[biome_meteo_id].add_data;

            WG.prepare_batch_operations();
            if(newsize)
            {
                RE.projection_grid.init_pos(WG.world_size.x*CHUNK_SIZE, WG.world_size.y*CHUNK_SIZE, WG.world_size.z*CHUNK_SIZE);
            }
            else
                RE.projection_grid.clear();

            world.init(WG.world_size.x, WG.world_size.y, WG.world_size.z);

            RE.window.scale = DEFAULT_SCALE;
            RE.refresh_sprite_size();
            RE.center_camera();
            RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);
            RE.projection_grid.save_curr_interval();
            RE.set_global_illumination_direction();

            GameEvent.add_nfs_event(NFS_GENERATE_NEW_MAP);
            GameEvent.add_nfs_event(NFS_OP_PG_ONSCREEN);
            GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
            GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);

            SDL_UnlockMutex(GameEvent.world_and_projection_grid_mut);
        }
    }

    return retval;
}

int Game::input_options(SDL_Event &event, SDL_Keymod km)
{
    int retval = INPUT_UNUSED;

    switch(event.key.type)
    {
        case SDL_KEYDOWN :
            if(event.key.type == SDL_KEYDOWN && event.key.repeat == 0)
                switch(event.key.keysym.sym)
                {

                    case SDLK_ESCAPE :
                    {
                        state = STATE_MAIN_MENU;

                        SDL_DisplayMode DM;
                        SDL_GetCurrentDisplayMode(0, &DM);

                        if(
                        (current_res > 0 && (RE.screen->w != resolutions[current_res].w || RE.screen->h != resolutions[current_res].h)) ||
                        (current_res == -1 && (RE.screen->w != DM.w || RE.screen->h != DM.h))
                        )
                        {
                            current_res = current_restmp;
                            UI.set_currres(current_res);
                        }

                        save_settings();
                    }
                        retval = INPUT_CLEAR;
                        break;

                    // case SDLK_RETURN :
                    //     if(~km & KMOD_LALT)
                    //     {
                            // SDL_ShowCursor(SDL_DISABLE);
                        //     UI.set_ui_hl_mode(RE.highlight_mode);
                        //     state = STATE_CONSTRUCTION;
                        //     retval = INPUT_CLEAR;
                        // }
                        // break;

                    case SDLK_KP_ENTER :
                        state = STATE_CONTROLS;
                        retval = INPUT_CLEAR;
                    break;

                    default : break;
                }
        
        case SDL_MOUSEBUTTONDOWN :
            if(event.button.button == SDL_BUTTON_LEFT)
            {
                if(Menu_hl_name == IDMENU_OPTIONS_RESOLUTION_FLLSCR)
                {
                    GPU_SetFullscreen(!GPU_GetFullscreen(), false);
                    // init_resolution();
                    retval = INPUT_CLEAR;
                }

                else if(Menu_hl_name == IDMENU_OPTIONS_RESOLUTION_CUSTOM_PREV)
                {
                    current_res --;
                    if(current_res < 0) current_res = 0;
                    if(current_res > RESOLUTIONS_NB) current_res = RESOLUTIONS_NB;
                    UI.set_currres(current_res);
                    retval = INPUT_CLEAR;
                }
                else if(Menu_hl_name == IDMENU_OPTIONS_RESOLUTION_CUSTOM_NEXT)
                {
                    current_res ++;
                    if(current_res < 0) current_res = 0;
                    if(current_res > RESOLUTIONS_NB) current_res = RESOLUTIONS_NB;
                    UI.set_currres(current_res);
                    retval = INPUT_CLEAR;
                }
                
                else if(Menu_hl_name == IDMENU_OPTIONS_RESOLUTION_BORLSS)
                {
                    // GPU_SetFullscreen(false, true);

                    current_res = -1;
                    init_resolution();

                    // SDL_SetWindowPosition(SDL_GetWindowFromID(RE.screen->context->windowID), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

                    retval = INPUT_CLEAR;
                }

                else if(Menu_hl_name == IDMENU_OPTIONS_RESOLUTION_CUSTOM)
                {
                    // GPU_SetFullscreen(false, false);
                    init_resolution();
                    // SDL_SetWindowPosition(SDL_GetWindowFromID(RE.screen->context->windowID), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

                    retval = INPUT_CLEAR;
                }

                else if(Menu_hl_name == IDMENU_OPTIONS_AO)
                {
                    switch_AO();
                    retval = INPUT_CLEAR;
                }
                else if(Menu_hl_name == IDMENU_OPTIONS_SHADOWS)
                {
                    switch_shadows();
                    retval = INPUT_CLEAR;
                }
                else if(Menu_hl_name == IDMENU_OPTIONS_BLOCKS_BORDER)
                {
                    switch_borders();
                    retval = INPUT_CLEAR;
                }
                else if(Menu_hl_name == IDMENU_OPTIONS_VSYNC)
                {
                    if(vsync)
                    {
                        vsync = 0;
                    }
                    else
                    {
                        vsync = 1;
                    }

                    UI.set_option(vsync, IDMENU_OPTIONS_VSYNC);
                    retval = INPUT_CLEAR;
                }
                else if(Menu_hl_name == IDMENU_OPTIONS_CONTROLS)
                {
                    state = STATE_CONTROLS;
                    retval = INPUT_CLEAR;
                }
            }

            break;

        case SDL_MOUSEWHEEL :

            if(Menu_hl_name == IDMENU_OPTIONS_RESOLUTION_CUSTOM)
            {
                current_res -= event.wheel.y;
                if(current_res < 0) current_res = 0;
                if(current_res > RESOLUTIONS_NB) current_res = RESOLUTIONS_NB;
                UI.set_currres(current_res);
                retval = INPUT_CLEAR;
            }

            // UI.options_scroll.scroll(-event.wheel.y);

            break;

        default : break;
    }

    return retval;
}

int Game::input_controlmenu(SDL_Event &event , SDL_Keymod km)
{
    int retval = INPUT_UNUSED;

    switch(event.key.type)
    {
        case SDL_KEYDOWN :

            if(event.key.keysym.sym == SDLK_ESCAPE)
            {
                state = STATE_OPTIONS;
                retval = INPUT_CLEAR;
            }

        break;

        case SDL_MOUSEBUTTONDOWN :
            if(event.button.button == SDL_BUTTON_LEFT)
            {
                if(scrollmenu_id != -1)
                {
                    state = STATE_CONTROLS_SELECT;
                    curr_scrollmenu_id = scrollmenu_id;
                    retval = INPUT_CLEAR;
                }
            }
        break;

        case SDL_MOUSEWHEEL :
        
            UI.option_controls_scroll += event.wheel.preciseY;
            
            retval = INPUT_CLEAR;

        break;

        default :
        break;
    }

    return retval;
}

int Game::input_controlselect(SDL_Event &event , SDL_Keymod km)
{
    int retval = INPUT_UNUSED;

    if(event.key.type == SDL_KEYDOWN)
    {
        if(event.key.keysym.sym != SDLK_ESCAPE)
        {
            keybutton *controls = (keybutton*)&Cinpt;
            controls[curr_scrollmenu_id].code = event.key.keysym.scancode;
        }

        retval = INPUT_CLEAR;
        state = STATE_CONTROLS;
        curr_scrollmenu_id = -1;
    }

    return retval;
}

int Game::input_world_selection(SDL_Event &event, SDL_Keymod km)
{
    int retval = INPUT_UNUSED;

    switch(event.key.type)
    {
        case SDL_KEYDOWN :
            if(event.key.type == SDL_KEYDOWN && event.key.repeat == 0)
                switch(event.key.keysym.sym)
                {
                    case SDLK_ESCAPE :
                    {
                        // SDL_ShowCursor(SDL_ENABLE);

                        UI.set_ui_hl_mode(HIGHLIGHT_MOD_NONE);
                        state = STATE_MAIN_MENU;

                        RE.set_global_illumination(meteos[METEO_MAIN_MENU].global_illumination);
                        RE.background_shader = meteos[METEO_MAIN_MENU].background_shader;
                        RE.background_shader_data = meteos[METEO_MAIN_MENU].add_data;

                        RE.projection_grid.clear();
                        // world.free_chunks();

                        Current_world_name = "/noworld";
                        retval = INPUT_CLEAR;
                    }
                        break;

                    case SDLK_RETURN :
                        if(~km & KMOD_LALT)
                        {
                            switch_to_construction();
                            retval = INPUT_CLEAR;
                        }
                        break;

                    default : break;
                }
            break;
        
        case SDL_MOUSEBUTTONDOWN :
            if(event.button.button == SDL_BUTTON_LEFT)
            {
                if(New_world_name[0] != '/')
                {
                    To_load_World_name = New_world_name;
                    retval = INPUT_CLEAR;
                }
                else if(Current_world_name[0] != '/' && menu_selectionables[2] == 0)
                {
                    switch_to_construction();
                    AE.Play_click();
                    retval = INPUT_CLEAR;
                }
                else if(menu_selectionables[0] == 0)
                {
                    state = STATE_NEW_MAP;

                    wvp_tmp = world.world_view_position;

                    world.world_view_position = 2;

                    WG.world_size_id = -1;
                    WG.biome_id = -1;
                    WG.preset_id = -1;
                    WG.is_generating = 0;

                    // menu_selected[0] = &WG.world_size_id;
                    // menu_selected[1] = &WG.biome_id;
                    // menu_selected[2] = &WG.preset_id;

                    menu_selected_buff[0] = -1;
                    menu_selected_buff[1] = -1;
                    menu_selected_buff[2] = -1;

                    menu_selected[0] = &menu_selected_buff[0];
                    menu_selected[1] = &menu_selected_buff[1];
                    menu_selected[2] = &menu_selected_buff[2];
                    menu_selected[3] = &WG.is_generating;

                    NW_map_name = NW_default_name;
                    SDL_StartTextInput();
                    retval = INPUT_CLEAR;
                }
                else if(menu_selectionables[1] == 0)
                {
                    UI.delmapsec.end_timer();
                    RE.projection_grid.clear();
                    world.free_chunks();

                    Current_meteo = std::find(unlocked_meteo.begin(), unlocked_meteo.end(), METEO_MAIN_MENU);

                    RE.set_global_illumination(meteos[METEO_MAIN_MENU].global_illumination);
                    RE.background_shader = meteos[METEO_MAIN_MENU].background_shader;
                    RE.background_shader_data = meteos[METEO_MAIN_MENU].add_data;

                    std::filesystem::remove_all("saves/"+Current_world_name);

                    UI.generate_tiles(STATE_WORLD_SELECTION, RE.screen->w, RE.screen->h);
                    Current_world_name = "/noworld";
                    retval = INPUT_CLEAR;
                }
            }            
            break;
        
        case SDL_MOUSEMOTION :
            // New_world_name
            // Current_world_name
            
            // std::cout << New_world_name << '\n';
            // std::cout << "Jusqu'ici, tout va bien...\n" << Current_world_name;
            // if(New_world_name != Current_world_name && New_world_name[0] != '/')
            // {     
            //     GameEvent.drop_all_nfs_event();
            //     if(!GameEvent.is_NFS_reading_to_wpg)
            //     {
            //         RE.highlight_mode = HIGHLIGHT_MOD_NONE;
            //         RE.highlight_type = HIGHLIGHT_BLOCKS;
            //         UI.set_ui_hl_type(RE.highlight_type);
            //         UI.set_ui_hl_mode(RE.highlight_mode);

            //         RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
            //         RE.highlight_wcoord2 = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};

            //         world_extras_select wes(false);
            //         wes.world_view_position = true;
            //         wes.meteo = true;
            //         load_world(New_world_name, true, true, true, nullptr, wes);

            //         Current_world_name = New_world_name;
            //         retval = INPUT_CLEAR;
            //     }
            // }
            // Current_world_name = "/noworld";

            break;

        case SDL_MOUSEWHEEL :        
            UI.world_selection_scroll.add(-event.wheel.preciseY);
            retval = INPUT_CLEAR;
        break;

        default : break;
    }


    if(To_load_World_name != Current_world_name && To_load_World_name[0] != '/')
    {     
    // std::cout << Current_world_name << "\t" << To_load_World_name << "\n";
        GameEvent.drop_all_nfs_event();
        // if(!GameEvent.is_NFS_reading_to_wpg)
        if(!SDL_TryLockMutex(GameEvent.world_and_projection_grid_mut))
        {
            SDL_UnlockMutex(GameEvent.world_and_projection_grid_mut);
            
            // undo_manager.clear();
            RE.projection_grid.clear();
            world.lights.trash_bin.clear();

            RE.highlight_mode = HIGHLIGHT_MOD_NONE;
            RE.highlight_type = HIGHLIGHT_BLOCKS;
            UI.set_ui_hl_type(RE.highlight_type);
            UI.set_ui_hl_mode(RE.highlight_mode);

            RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
            RE.highlight_wcoord2 = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};

            world_extras_select wes(false);
            wes.world_view_position = true;
            wes.meteo = true;
            load_world(To_load_World_name, true, true, nullptr, wes);

            // Current_world_name = To_load_World_name;
            retval = INPUT_CLEAR;

            To_load_World_name = "/noworld";
        }
    }

    return retval;
}

int Game::input_block_selection(SDL_Event &event, SDL_Keymod km)
{
    int retval = INPUT_UNUSED;

    switch(event.key.type)
    {
        case SDL_KEYDOWN :
            if(event.key.type == SDL_KEYDOWN && event.key.repeat == 0)
                switch(event.key.keysym.sym)
                {
                    case SDLK_w :
                    case SDLK_ESCAPE :
                        switch_to_construction();
                        retval = INPUT_CLEAR;
                        break;

                    default :
                    if(event.key.keysym.scancode == Cinpt.BLCK_PREV.code)
                    {
                        cb_id = (cb_id+1)%8;
                        retval = INPUT_CLEAR;
                    }
                    
                    else if(event.key.keysym.scancode == Cinpt.BLCK_NEXT.code)
                    {
                        cb_id = cb_id-1 < 0 ? 7 : cb_id-1;
                        retval = INPUT_CLEAR;
                    }

                    break;
                }
        
        case SDL_MOUSEBUTTONDOWN :
            if(event.button.button == SDL_BUTTON_LEFT)
            {
                if(!menu_selectionables[0])
                {
                    switch_to_construction();
                    retval = INPUT_CLEAR;
                }
                else if(new_cb_id != -1)
                {
                    cb_id = new_cb_id;
                    retval = INPUT_CLEAR | INPUT_SOUND_PLAYED;
                    AE.Play_click(2);
                }
                else if(new_current_block)
                {
                    currentblocks[cb_id] = new_current_block;
                    UI.set_ui_current_blocks(cb_id, new_current_block);
                    retval = INPUT_CLEAR | INPUT_SOUND_PLAYED;
                    AE.Play_click(2);
                }
            }
            break;

        case SDL_MOUSEWHEEL :
            if(km & KMOD_RSHIFT)
            {
                UI.UI_scale += event.wheel.y*0.25;
                if(UI.UI_scale < 0.0)
                    UI.UI_scale = 0.0;
                
                if(UI.UI_scale > 5.0)
                    UI.UI_scale = 5.0;

                retval = INPUT_CLEAR;
            }
            else
            {
                bs_max_line -= event.wheel.y;
                int line = floor(5.0*RE.screen->w/800.0);

                if(line < 1)
                    line = 1;
                    
                set_in_interval(bs_max_line, 0, unlocked_blocks.size()/line);
                retval = INPUT_CLEAR;
            }


            break;

        default : break;
    }
    
    return retval;
}

int Game::input_construction(SDL_Event &event, SDL_Keymod km)
{
    int retval = INPUT_UNUSED;

    int status;

    bool rwr = false;

    switch(event.key.type)
    {
        case SDL_KEYDOWN :
            if(event.key.repeat == 0)
            {
                if(SDLK_ESCAPE == event.key.keysym.sym)
                {
                    if(RE.highlight_wcoord2.x >= 0)
                    {
                        RE.highlight_wcoord2 = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                        RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                    }
                    else
                    {
                        status = world.save_to_file("saves/" + Current_world_name + "/world.isosave");

                        if(status == 0)
                        {
                            world_extras we;
                            world_extras_fill(we);
                            save_world_extras(Current_world_name, we);
                        }
                        else
                            std::cout << "Error loading save " << Current_world_name << "\n";

                        state = STATE_WORLD_SELECTION;
                        menu_selectionables[0] = -1;
                        UI.set_ui_hl_mode(HIGHLIGHT_MOD_NONE);
                    }
                    retval = INPUT_CLEAR;
                }

                // else if(event.key.keysym.scancode == SDL_SCANCODE_KP_1)
                // {
                //     state = STATE_ADVENTURE;
                //     retval = INPUT_CLEAR;
                // }

                else if(km&KMOD_LCTRL && event.key.keysym.scancode == SDL_SCANCODE_W)
                {
                    if (km & KMOD_LSHIFT)
                    {
                        // AE.Play_woosh(true);
                        redo();
                        timer_input_redo.start_timer();
                    }
                    else
                    {
                        // AE.Play_woosh();
                        undo();
                        timer_input_undo.start_timer();
                    }
                    retval = INPUT_CLEAR;
                }

                else if(km&KMOD_LCTRL && event.key.keysym.scancode == SDL_SCANCODE_Y)
                {
                    // AE.Play_woosh(true);
                    redo();
                    timer_input_redo.start_timer();
                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.REFRESH_D.code)
                {
                    refresh_world_render_fast();
                }

                else if(event.key.keysym.scancode == Cinpt.TGLE_GRID.code)
                {
                    RE.shader_features ^= SFEATURE_GRID;
                    save_settings();
                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.TOOL_NEXT.code)
                {
                    RE.highlight_type = (RE.highlight_type+1)%5;
                    RE.highlight_type = RE.highlight_type == 0 ? 1 : RE.highlight_type;

                    RE.height_volume_tool = -1;

                    RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};

                    UI.set_ui_hl_type(RE.highlight_type);
                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.TOOL_PREV.code)
                {
                    RE.highlight_type = RE.highlight_type-1;
                    RE.highlight_type = RE.highlight_type == 0 ? 4 : RE.highlight_type;

                    RE.height_volume_tool = -1;

                    RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};

                    UI.set_ui_hl_type(RE.highlight_type);
                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.TGLE_MODE.code)
                {
                    UI.set_ui_hl_type(HIGHLIGHT_MOD_NONE);
                    RE.highlight_type  = HIGHLIGHT_MOD_NONE;
                    RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.MODE_NEXT.code)
                {
                    RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                    RE.highlight_wcoord2 = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                    RE.highlight_mode = (RE.highlight_mode+1)%5;
                    UI.set_ui_hl_mode(RE.highlight_mode);
                    retval = INPUT_CLEAR;

                }

                else if(event.key.keysym.scancode == Cinpt.MODE_PREV.code)
                {
                    RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                    RE.highlight_wcoord2 = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                    RE.highlight_mode = RE.highlight_mode-1;
                    RE.highlight_mode = RE.highlight_mode < 0 ? 4 : RE.highlight_mode;
                    UI.set_ui_hl_mode(RE.highlight_mode);
                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.CAM_CENT.code)
                {
                    RE.center_camera();
                    GameEvent.add_event(GAME_EVENT_CAMERA_MOUVEMENT, (pixel_coord){0, 0});
                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.CAM_ROTR.code)
                {
                    RE.rotate_camera(1);

                    GameEvent.drop_all_nfs_event();

                    RE.projection_grid.clear();
                    RE.projection_grid.save_curr_interval();
                    RE.force_nonpanorama_mode = true;

                    if(RE.test_panorama_mode())
                    {
                        RE.transparent_sprite_counter = 0;
                        // GameEvent.add_nfs_event(NFS_OP_PG_ONSCREEN);
                    }
                    else
                    {
                        RE.refresh_pg_onscreen();
                        RE.refresh_pg_block_visible();
                    }

                    GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
                    GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);
                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.CAM_ROTL.code)
                {
                    RE.rotate_camera(-1);

                    GameEvent.drop_all_nfs_event();

                    RE.projection_grid.clear();
                    RE.projection_grid.save_curr_interval();

                    if(RE.test_panorama_mode())
                    {
                        RE.transparent_sprite_counter = 0;
                        // GameEvent.add_nfs_event(NFS_OP_PG_ONSCREEN);
                    }
                    else
                    {
                        RE.refresh_pg_onscreen();
                        RE.refresh_pg_block_visible();
                    }

                    GameEvent.add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
                    GameEvent.add_nfs_event(NFS_OP_ALL_RENDER_FLAG);
                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.QUICKSAVE.code)
                {
                    std::string total_filename = "saves/";
                    total_filename.append(Current_world_name);
                    total_filename.append("/world.isosave");

                    status = world.save_to_file(total_filename);

                    if(status == 0)
                    {
                        world_extras we;
                        world_extras_fill(we);
                        save_world_extras(Current_world_name, we);
                        std::cout << "world saved at " << total_filename << "\n";
                    }
                    else
                    {
                        std::cout << status << " : failded to save world :(\n";
                    }
                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.QUICKLOAD.code)
                {
                    GameEvent.drop_game_event();
                    if(!GameEvent.is_NFS_reading_to_wpg)
                        load_world(Current_world_name);
                    
                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.BLCK_PREV.code)
                {
                    cb_id = (cb_id+1)%8;
                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.BLCK_NEXT.code)
                {
                    cb_id = cb_id-1 < 0 ? 7 : cb_id-1;
                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.METEO_NEXT.code)
                {
                    Current_meteo = circularNext(unlocked_meteo, Current_meteo);
                    RE.set_global_illumination(meteos[*Current_meteo].global_illumination);
                    RE.background_shader = meteos[*Current_meteo].background_shader;
                    RE.background_shader_data = meteos[*Current_meteo].add_data;
                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.METEO_PREV.code)
                {
                    Current_meteo = circularPrev(unlocked_meteo, Current_meteo);
                    RE.set_global_illumination(meteos[*Current_meteo].global_illumination);
                    RE.background_shader = meteos[*Current_meteo].background_shader;
                    RE.background_shader_data = meteos[*Current_meteo].add_data;
                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.BLCK_NEXT.code)
                {
                    Current_meteo = circularPrev(unlocked_meteo, Current_meteo);
                    RE.set_global_illumination(meteos[*Current_meteo].global_illumination);
                    RE.background_shader = meteos[*Current_meteo].background_shader;
                    RE.background_shader_data = meteos[*Current_meteo].add_data;
                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.TGLE_PIPET.code)
                {
                    if(RE.highlight_type != HIGHLIGHT_PIPETTE)
                    {
                        Current_HL_type = RE.highlight_type;
                        RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                        RE.highlight_type = HIGHLIGHT_PIPETTE;
                        UI.set_ui_hl_type(HIGHLIGHT_PIPETTE);
                    }
                    else
                    {
                        RE.highlight_type = Current_HL_type;
                        RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                        UI.set_ui_hl_type(RE.highlight_type);
                    }

                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.TGLE_HUD.code)
                {
                    new_cb_id = -1;
                    new_hl_type = -1;
                    Show_HUD = !Show_HUD;
                    retval = INPUT_CLEAR;
                }

                else if(event.key.keysym.scancode == Cinpt.OPEN_BLSEl.code)
                {
                    state = STATE_BLOCK_SELECTION;
                    UI.set_ui_hl_mode(HIGHLIGHT_MOD_NONE);
                    retval = INPUT_CLEAR;
                }
            
            }
            break;

        case SDL_KEYUP :
            if(event.key.repeat == 0)
            {
                if (event.key.keysym.scancode == SDL_SCANCODE_W || event.key.keysym.scancode == SDL_SCANCODE_Y)
                {
                    timer_input_redo.stop_timer();
                    timer_input_undo.stop_timer();
                    retval = INPUT_CLEAR;
                }
            }
            break;

        case SDL_MOUSEMOTION :
            if(event.motion.state == SDL_BUTTON_RMASK)
            {
                GameEvent.add_event(GAME_EVENT_CAMERA_MOUVEMENT, (pixel_coord){event.motion.xrel, event.motion.yrel});
                retval = INPUT_CLEAR;
            }
            break;

        case SDL_MOUSEWHEEL :

            if((km & KMOD_LSHIFT) || (km & KMOD_RSHIFT))
            {
                // world.find_highest_nonemptychunk();
                SDL_LockMutex(RE.campg_mut);
                if(RE.max_height_render > (world.highest_nonemptychunk+1)*CHUNK_SIZE)
                {
                    if(km & KMOD_LSHIFT)
                        RE.max_height_render = (world.highest_nonemptychunk+1)*CHUNK_SIZE + event.wheel.y;
                    else 
                        RE.max_height_render = (world.highest_nonemptychunk+1)*CHUNK_SIZE + event.wheel.y*CHUNK_SIZE;
                }
                else if(km & KMOD_LSHIFT)
                    RE.max_height_render += event.wheel.y;
                else 
                    RE.max_height_render += event.wheel.y*CHUNK_SIZE;

                if(RE.max_height_render > world.max_block_coord.z)
                    RE.max_height_render = world.max_block_coord.z;

                else if(RE.max_height_render < 1)
                    RE.max_height_render = 1;

                if(event.wheel.y > 0)
                    RE.reverse_rhr = true;
                else 
                    RE.reverse_rhr = false;

                // rwr = true;
                SDL_UnlockMutex(RE.campg_mut);
                GameEvent.drop_all_nfs_event();
                GameEvent.add_nfs_event(NFS_NEW_HEIGHT_RENDER);
                // retval = INPUT_CLEAR;

                // std::cout << RE.max_height_render << "\n";
            }
            else if(km & KMOD_LCTRL)
            {
                RE.height_volume_tool += event.wheel.y;
                set_in_interval(RE.height_volume_tool, 0, world.max_block_coord.z);
                retval = INPUT_CLEAR;
            }
            else if(km & KMOD_RCTRL)
            {
                RE.height_volume_tool += 8*event.wheel.y;
                set_in_interval(RE.height_volume_tool, 0, world.max_block_coord.z);
                retval = INPUT_CLEAR;
            }
            else if(km & KMOD_RSHIFT)
            {
                UI.UI_scale += event.wheel.y*0.25;
                if(UI.UI_scale < 0.0)
                    UI.UI_scale = 0.0;
                
                if(UI.UI_scale > 5.0)
                    UI.UI_scale = 5.0;
                
                retval = INPUT_CLEAR;
            }
            else
            {
                GameEvent.add_event(GAME_EVENT_ADDSCALE, event.wheel.y);

                retval = INPUT_CLEAR;
            }

            break;
        
        case SDL_MOUSEBUTTONDOWN :
            if(event.button.button == SDL_BUTTON_LEFT)
            {
                if(menu_selectionables[1] != -1)
                {
                    set_meteo(menu_selectionables[1]+1);
                    refresh_meteo();
                    retval = INPUT_CLEAR;
                }
                else if(!menu_selectionables[0])
                {
                    state = STATE_BLOCK_SELECTION;
                    UI.set_ui_hl_mode(HIGHLIGHT_MOD_NONE);
                    retval = INPUT_CLEAR;
                }
                else if(new_hl_type != -1)
                {
                    AE.Play_click(1);
                    RE.highlight_type = new_hl_type+1;
                    retval = INPUT_CLEAR | INPUT_SOUND_PLAYED;
                }
                else if(new_cb_id != -1)
                {
                    AE.Play_click(2);
                    cb_id = new_cb_id;
                    retval = INPUT_CLEAR | INPUT_SOUND_PLAYED;
                }
                else if(RE.highlight_type == HIGHLIGHT_PIPETTE)
                {
                    Uint8 id = world.get_block_id_wcoord(RE.highlight_wcoord.x, RE.highlight_wcoord.y, RE.highlight_wcoord.z);

                    auto pipette = std::find(unlocked_blocks.begin(), unlocked_blocks.end(), id);

                    if(pipette != unlocked_blocks.end() || *pipette != *unlocked_blocks.end())
                    {
                        // std::cout << "\npipette moment " << id;

                        UI.set_ui_current_blocks(cb_id, *pipette);
                        currentblocks[cb_id] = *pipette;
                    }
                    retval = INPUT_CLEAR | INPUT_SOUND_PLAYED;
                }
                else if(RE.highlight_wcoord2.x == HIGHLIGHT_NOCOORD && RE.highlight_type >= HIGHLIGHT_FLOOR)
                {
                    RE.highlight_wcoord2 = RE.highlight_wcoord;
                    retval = INPUT_CLEAR;
                }
                else if(RE.highlight_mode >= HIGHLIGHT_MOD_REPLACE)
                {
                    // AE.Play_voxel_modif(0);

                    GameEvent.add_event(GAME_EVENT_PLAYER_BLOCK_MOD, (coord3D)RE.highlight_wcoord, currentblocks[cb_id]);
                    retval = INPUT_CLEAR | INPUT_SOUND_PLAYED;
                }
                else if(RE.highlight_mode == HIGHLIGHT_MOD_DELETE)
                {
                    GameEvent.add_event(GAME_EVENT_PLAYER_BLOCK_MOD, (coord3D)RE.highlight_wcoord, BLOCK_EMPTY);
                    retval = INPUT_CLEAR | INPUT_SOUND_PLAYED;
                }
                
            }
            else
            if(event.button.button == SDL_BUTTON_MIDDLE)
            {
                RE.highlight_wcoord = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                RE.highlight_wcoord2 = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
                retval = INPUT_CLEAR;
            }
            break;

        default:
            break;
        }

    if(rwr) retval += INPUT_REFRESH_WORLD_RENDER;

    return retval;
};

int Game::input_adventure(SDL_Event &event, SDL_Keymod km)
{
    int retval = INPUT_UNUSED;

    switch(event.key.type)
    {
        case SDL_KEYDOWN :
            if(event.key.type == SDL_KEYDOWN && event.key.repeat == 0)
                switch(event.key.keysym.sym)
                {

                    case SDLK_ESCAPE :
                        switch_to_construction();
                        retval = INPUT_CLEAR;
                        break;

                    case SDLK_RETURN :
                        if(~km & KMOD_LALT)
                        {
                            if(!player.isinit())
                            {
                                std::string tmpname = "player";
                                player.init_sprite(&GameEvent, &world, &RE, tmpname);
                            }
                            
                            std::cout << "spawning player sprite\n";

                            fcoord3D initial_playerpos = {world.max_block_coord.x*0.5f, 
                                                          world.max_block_coord.y*0.5f,
                                                          8.0};

                            player.tp(initial_playerpos);
                            player.rotate_right();
                            player.update();
                            // player.tp({50, 50, 2});


                            Agent_test = std::make_unique<Ant>(&world, &player, 50);
                            // Agent_test = new Ant(&world, &player);

                            retval = INPUT_CLEAR;
                        }
                        break;
                    
                    case SDLK_DELETE :

                        std::cout << "removing player sprite from world\n";
                        player.remove();
                        retval = INPUT_CLEAR;

                        break;
                    
                    case SDLK_r :
                        RE.projection_grid.save_curr_interval();
                        RE.refresh_pg_onscreen();
                        retval = INPUT_CLEAR;
                        break;

                    default : break;
                }
            break;
        
        case SDL_MOUSEMOTION :
            if(event.motion.state == SDL_BUTTON_RMASK)
            {
                GameEvent.add_event(GAME_EVENT_CAMERA_MOUVEMENT, (pixel_coord){event.motion.xrel, event.motion.yrel});
                retval = INPUT_CLEAR;
            }
            break;

        case SDL_MOUSEWHEEL :
            
            GameEvent.add_event(GAME_EVENT_ADDSCALE, event.wheel.y);
            retval = INPUT_CLEAR;

            break;

        default:
            break;
        }

    return retval;

    // const Uint8* keystate = SDL_GetKeyboardState(NULL);

    // fcoord3D player_vel = {0.0, 0.0, 0.0};
    // // fcoord3D player_vel = {0.05*cos(timems*0.005), 0.0, 0.0};

    // float speed = 0.0625;

    // if(keystate[ SDL_SCANCODE_S ])
    // {
    //     player_vel.x += 1.0*speed;
    //     player_vel.y += 1.0*speed;
    // }
    // if(keystate[ SDL_SCANCODE_W ])
    // {
    //     player_vel.x -= 1.0*speed;
    //     player_vel.y -= 1.0*speed;
    // }
    // if(keystate[ SDL_SCANCODE_D ])
    // {
    //     player_vel.x += 1.0*speed;
    //     player_vel.y -= 1.0*speed;
    // }
    // if(keystate[ SDL_SCANCODE_A ])
    // {
    //     player_vel.x -= 1.0*speed;
    //     player_vel.y += 1.0*speed;
    // }

    // bool move = player.move(player_vel, true, true, true);
    // bool refresh_anim = player.refresh_animation();

    // if(refresh_anim && move <= 0)
    // {
    //     player.remove();
    // }

    // if((move > 0) || refresh_anim)
    // {
    //     player.update();
    // }
}