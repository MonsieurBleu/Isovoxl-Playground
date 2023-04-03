#include <game.hpp>
#include <filesystem>
namespace fs = std::filesystem;

int UI_SIZEDIFF = 1;

UI_Engine::UI_Engine()
{
    option_controls_scroll = 0;
    block_atlasx32 = nullptr;
    UI_image = NULL;
    UI = NULL;
}

void scroll_menu_val::init(int _size, int _winsize)
{  
    _winsize --;

    size = _size;
    winsize = _winsize;

    beg = 0;
    end = winsize;
}

bool scroll_menu_val::scroll(int y)
{
    bool limit_reached = false;

    beg += y;
    end += y;

    if(end >= size)
    {
        limit_reached = true;
        beg = size-winsize;
        end = size;
    }
    else
    if(beg < 0)
    {
        limit_reached = true;
        beg = 0;
        end = winsize;
    }

    return limit_reached;
}

void scrool_val::add(float x)
{
    val += x;

    if(val > max) val = max;
    if(val < min) val = min;
}

void button_security::start_timer()
{
    start = timems;
    activated = true;
}

void button_security::handle()
{
    if(activated && timems - start > mstimer)
        activated = false;
}

bool button_security::end_timer()
{
    activated = false;
    start = 0;

    return true;
}

bool button_security::check_double_click_security()
{
    return (timems - start) > doubleclicksec;
}

int button_security::get_timer_seg(int nbseg)
{
    return round(nbseg * float(timems - start)/float(mstimer));
}

bool button_security::is_activated() {return activated;}

void UI_Engine::prepare_window_shader()
{
    window_shader.activate();
    GPU_SetUniformf(1, timems/1000.0);
    window_shader.deactivate();
}

void UI_Engine::draw_window(GPU_Target *screen, float cx, float cy, float w, float h, Uint8 highlight_type)
{
    // GPU_Rect dst = {cx-w/2.f, cy-h/2.f, w, h};
    // GPU_RectangleRoundFilled2(UI, dst, 15, {255, 255, 255, 128});

    window_shader.activate();

    int win_const[4] = {screen->w, screen->h, (int)round(w), (int)round(h)};

    GPU_SetUniformiv(5, 4, 1, win_const);

    GPU_SetColor(Shader_Error_img.ptr, {0, 0, 0, highlight_type});

    GPU_Rect dst = {cx-w/2.f, cy-h/2.f, w, h};
    GPU_BlitRect(Shader_Error_img.ptr, NULL, UI, &dst);

    window_shader.deactivate();
}

bool dtxtl_nowinerr = false;

float UI_Engine::draw_txt_line(GPU_Target *screen, 
                              float cx,
                              float cy, 
                              const std::string strs[],
                              const int nbstrs,
                              const int id_selected,
                              int *id_highlighted,
                              const int nb_perline,
                              const float forced_fontsize,
                              int align)
{
    float sizew =  0.0;

    for(int i = 0; i < nbstrs; i++)
    {
        sizew += strs[i].size();
    }

    float fontsize = forced_fontsize;

    if(fontsize == 0)
    {
        fontsize = (1.0/sizew) * (UI->w/UI->h);
    }
    else if(fontsize < 0)
    {
        fontsize *= -(1.0/sizew) * (UI->w/UI->h);
    }

    if(nb_perline != -1)
    {
        int id_selected_rec = id_selected;
        int *id_highlighted_rec = id_highlighted;

        if(id_selected >= 0) dtxtl_nowinerr = true;

        for(int i = 0; i < nbstrs; i++)
        {
            if(i%nb_perline == 0)
            {
                draw_txt_line(screen, cx, cy, strs+i, nb_perline, id_selected_rec, id_highlighted_rec, -1, forced_fontsize, align);
                cy += fontsize*4;
                id_selected_rec -= nb_perline;

                if(id_highlighted_rec && *id_highlighted_rec != -1)
                {
                    *id_highlighted += i;
                    id_highlighted_rec = NULL;
                }
            }
        }

        dtxtl_nowinerr = false;

        return fontsize;
    }

    UI_text txt(font_monogram, "a", screen->w, screen->h,
    cx, cy, 
    fontsize, 
    &font_red_shader, IDMENU_UNTITLED, align);

    float space_size = fontsize*UI->h;
    // space_size = 0.0;

    // txt.centered_posx -= txt.getRect().w*(sizew/4.0 + (nbstrs/2.0)-0.0)+ 0.5*space_size*(nbstrs-1);

    if(align == TEXT_ALIGN_CENTER)
        txt.centered_posx -= 0.5*txt.getRect().w*(sizew/2.0 + 1.0) + 0.5*space_size*(nbstrs-1);
    else if(align == TEXT_ALIGN_RIGHT)
        txt.centered_posx -= 0.5*space_size*(nbstrs-1);

    for(int i = 0; i < nbstrs; i++)
    {
        txt.refresh_text(strs[i]);

        GPU_Rect r = txt.getRect();

            if(mouse.x > r.x-r.w/2 && mouse.x < r.x+r.w/2 &&
            mouse.y > r.y-r.h/2 && mouse.y < r.y+r.h/2)
            {
                forced_cursor = 0;
            }

        if(i == id_selected)
            draw_window(UI, r.x, r.y, r.w, r.h, UIWIN_HIGHLIGHT1);
        else
        {
            if(mouse.x > r.x-r.w/2 && mouse.x < r.x+r.w/2 &&
            mouse.y > r.y-r.h/2 && mouse.y < r.y+r.h/2)
            {
                draw_window(UI, r.x, r.y, r.w, r.h, UIWIN_HIGHLIGHT2);

                if(id_highlighted != NULL)
                    *id_highlighted = i;
            }
            else
                draw_window(UI, r.x, r.y, r.w, r.h);
        }

        if(id_selected < 0 && !dtxtl_nowinerr)
            txt.render(UI, TEXT_MOD_SHADER_ALL);
        else 
            txt.render(UI, TEXT_MOD_NONE);

        txt.centered_posx += r.w + space_size;
    }
    
    return fontsize;
}

float UI_Engine::draw_scroll_area(GPU_Target *screen, 
                                  float cx,
                                  float cy,
                                  float size,
                                  scrool_val &sc,
                                  int *selected,
                                  bool is_vertical,
                                  int nbseg)
{
    // *selected = -1;
    float scrollratio = sc.val/sc.max;

    GPU_Rect r = {screen->w*cx, screen->h*cy, screen->h*0.023f, size};

    if(is_vertical)
        draw_window(UI, r.x, r.y, r.h, r.w, UIWIN_HIGHLIGHT_NONE);
    else 
        draw_window(UI, r.x, r.y, r.w, r.h, UIWIN_HIGHLIGHT_NONE);

    GPU_Rect r2 = r;
    r2.h *= 3.f/abs(sc.max);

    float val1, val2;

    if(is_vertical)
    {
        val1 = (r.x + r2.h/2.f - r.h/2.f);
        val2 = (r.h - r2.h);

        r2.x = val1 + scrollratio*val2;
    }
    else
    {
        val1 = (r.y + r2.h/2.f - r.h/2.f);
        val2 = (r.h - r2.h);

        r2.y = val1 + scrollratio*val2;
    }

    if((!is_vertical && mouse.x > r.x-r.w/2 && mouse.x < r.x+r.w/2 &&
       mouse.y > r.y-r.h/2 && mouse.y < r.y+r.h/2 && Lclickdown)
       ||
       (is_vertical && mouse.x > r.x-r.h/2 && mouse.x < r.x+r.h/2 &&
       mouse.y > r.y-r.w/2 && mouse.y < r.y+r.w/2 && Lclickdown))
    {
        sc.follow_mouse = true;
    }

    if(Lclickup)
    {
        sc.follow_mouse = false;
    }

    if(sc.follow_mouse)
    {
        int maxtmp = val1+val2;
        int mintmp = val1;

        if(is_vertical)
        {
            r2.x = mouse.x;
            if(r2.x > maxtmp) r2.x = maxtmp;
            if(r2.x < mintmp) r2.x = mintmp;
            sc.val = ((r2.x-val1)/val2)*sc.max;

            if(nbseg > 1)
            {
                if(r2.x != maxtmp)
                {
                    sc.val = sc.val - fmod(sc.val, abs(sc.max)/(nbseg));
                    scrollratio = sc.val/sc.max;
                    r2.x = val1 + scrollratio*val2;
                }
            }
        }
        else 
        {
            r2.y = mouse.y;
            if(r2.y > maxtmp) r2.y = maxtmp;
            if(r2.y < mintmp) r2.y = mintmp;
            sc.val = ((r2.y-val1)/val2)*sc.max;
        }
    }

    if(is_vertical)
        draw_window(UI, r2.x, r2.y, r2.h, r2.w, UIWIN_HIGHLIGHT1);
    else 
        draw_window(UI, r2.x, r2.y, r2.w, r2.h, UIWIN_HIGHLIGHT1);

    return 0.0;
}

void UI_Engine::draw_mouse_indications(GPU_Target *screen, const std::string &txt, int align, int winflags)
{
    UI_text expl(font_monogram, txt, screen->w, screen->h, 
                 0, 0, 0.015, nullptr, 
                 IDMENU_UNTITLED, align);
    
    if(align == TEXT_ALIGN_LEFT)
    {
        expl.centered_posx = mouse.x + screen->w*0.0175*1.5;
        expl.centered_posy = mouse.y;
    }
    else if(align == TEXT_ALIGN_RIGHT)
    {
        expl.centered_posx = mouse.x - screen->w*0.0175*1.5;
        expl.centered_posy = mouse.y;
    }
    else if(align == TEXT_ALIGN_CENTER)
    {
        expl.centered_posx = mouse.x;
        expl.centered_posy = mouse.y + screen->w*0.0175*1.5;
    }
    
    GPU_Rect winr = expl.getRect();
    draw_window(screen, winr.x, winr.y, winr.w, winr.h, winflags);
    expl.render(screen, TEXT_MOD_NONE);
}

void UI_Engine::render_frame(int game_state, 
                             GPU_Target* screen, 
                             int cb_id,
                             int &new_cb_id,
                             int bs_line_min,
                             int &new_current_block,
                             int &new_hl_type,
                             int current_res,
                             int &scrollmenu_id,
                             int curr_scrollmenu_id,
                             int menu_selectionables[16],
                             int *menu_selected[16],
                             std::list<int> &unlocked_blocks,
                             std::string &New_world_name,
                             std::string &Current_world_name, 
                             std::string &Menu_hl_name,
                             Construction_cinputs &Cinpt,
                             void *REptr,
                             void *AEptr)
{
    Render_Engine *RE = (Render_Engine *)REptr;
    Audio_Engine *AE = (Audio_Engine *)AEptr;

    UI = screen;

    forced_cursor = -1;

    // old 
    // GPU_Target *screen_tmp = screen;
    // screen = UI;
    // GPU_Clear(UI);

    Menu_hl_name = "/";

    prepare_window_shader();

    menu_selectionables[15] = 0;

    if(game_state == STATE_MAIN_MENU || game_state == STATE_WORLD_SELECTION)
    {
        GPU_SetUniformf(1, timems/1000.0);
        // main_menu_title->render(UI, TEXT_MOD_SHADER_ALL);

        // draw_window(UI, main_menu_logo->getx(), 
        //                 main_menu_logo->gety(),
        //                 main_menu_logo->getw(),
        //                 main_menu_logo->geth());

        main_menu_logo->render(UI);
    }

    if(game_state == STATE_MAIN_MENU)
    {
        main_menu_version->render(UI, TEXT_MOD_HL_PERLETTER | TEXT_MOD_HL_BIGGER);

        for(auto i = main_menu.begin(); i != main_menu.end(); i++)
        {
            if((**i).render(UI, TEXT_MOD_HL_ALL | TEXT_MOD_SHADER_HL | TEXT_MOD_HL_WAVE))
            {
                Menu_hl_name = (**i).nameid;
            }
        }

        if(AE->is_playing_music())
        {
            const IsoMusic &curmus = AE->get_current_music();
            // const std::string txt = curmus.autor + " - " + curmus.name;
            const std::string txt = "Playing " + curmus.name + " by " + curmus.autor;
            UI_text music(font_round2, txt, screen->w, screen->h, 0.5, 0.95, 0.02);

            music.render(UI, TEXT_MOD_HL_PERLETTER | TEXT_MOD_HL_BIGGER);
        }
    }

    if(game_state == STATE_WORLD_SELECTION)
    {
        New_world_name = "/noworld";

        float cpytmp;

        // int textflag = TEXT_MOD_HL_ALL | TEXT_MOD_SHADER_HL | TEXT_MOD_HL_WAVE;
        int textflag = TEXT_MOD_HL_ALL | TEXT_MOD_SHADER_HL;

        for(auto i = world_selection_txt.begin(); i != world_selection_txt.end(); i++)
        {
            cpytmp = (**i).centered_posy;

            (**i).centered_posy -= world_selection_scroll.val*50;

            if((**i).centered_posy < screen->h*0.20)
            {
                (**i).centered_posy = cpytmp;
                continue;
            }

            GPU_Rect r = (**i).getRect();

            bool is_selected = (**i).textcpy == Current_world_name;

            if(is_selected)
                draw_window(UI, r.x, r.y, r.w, r.h, UIWIN_HIGHLIGHT1);

            if(mouse.x > r.x-r.w/2 && mouse.x < r.x+r.w/2 &&
               mouse.y > r.y-r.h/2 && mouse.y < r.y+r.h/2)
            {
                New_world_name = (**i).nameid;

                if(!is_selected)
                    draw_window(UI, r.x, r.y, r.w, r.h, UIWIN_HIGHLIGHT2);
            }

            else if(!is_selected)
                draw_window(UI, r.x, r.y, r.w, r.h, UIWIN_HIGHLIGHT_NONE);

            (**i).render(UI, textflag);

            (**i).centered_posy = cpytmp;

        }

        menu_selectionables[1] = -1;
        menu_selectionables[2] = -1;

        ///////////////////////////////////////////////////////////////
        if(Current_world_name[0] != '/')
        {
            UI_text play(font_monogram, WORLD_SELECTION_PLAY, 
                        screen->w, screen->h, 0.50, 0.875, 0.0225); 

            GPU_Rect r = play.getRect();

            if(mouse.x > r.x-r.w/2 && mouse.x < r.x+r.w/2 &&
               mouse.y > r.y-r.h/2 && mouse.y < r.y+r.h/2)
            {
                menu_selectionables[2] = 0;
                draw_window(UI, r.x, r.y, r.w, r.h, UIWIN_HIGHLIGHT1 | UIWIN_HIGHLIGHT2);
            }
            else
            {
                draw_window(UI, r.x, r.y, r.w, r.h, UIWIN_HIGHLIGHT1);
            }
            play.render(UI, TEXT_MOD_NONE);
        
        }

        ///////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////
        UI_text newmap(font_monogram, WORLD_SELECTION_NEWMAP, 
                       screen->w, screen->h, 0.50, 0.80, 0.0225);

        GPU_Rect r = newmap.getRect();

        menu_selectionables[0] = -1;

        if(mouse.x > r.x-r.w/2 && mouse.x < r.x+r.w/2 &&
           mouse.y > r.y-r.h/2 && mouse.y < r.y+r.h/2)
        {
            draw_window(UI, r.x, r.y, r.w, r.h, UIWIN_HIGHLIGHT_GREEN | UIWIN_HIGHLIGHT2);
            menu_selectionables[0] = 0;
        }
        else
            draw_window(UI, r.x, r.y, r.w, r.h, UIWIN_HIGHLIGHT_GREEN);
    
        newmap.render(UI, TEXT_MOD_NONE);
        ///////////////////////////////////////////////////////////////

        
        ///////////////////////////////////////////////////////////////
        delmapsec.handle();

        if(Current_world_name[0] != '/')
        {
            UI_text del(font_monogram, "", 
                        screen->w, screen->h, 0.50, 0.95, 0.0225);

            if(delmapsec.is_activated())
            {
                std::string txt = WORLD_SELECTION_DELMAPSEC;
                std::string vis;

                int nbseg = 3;
                int seg = nbseg - delmapsec.get_timer_seg(3);
                
                for(int i = 0; i < seg; i++)
                    vis += '-';

                del.refresh_text(vis+txt+vis);
            }
            else
                del.refresh_text(WORLD_SELECTION_DELMAP);

            r = del.getRect();

            if(mouse.x > r.x-r.w/2 && mouse.x < r.x+r.w/2 &&
            mouse.y > r.y-r.h/2 && mouse.y < r.y+r.h/2)
            {
                draw_window(UI, r.x, r.y, r.w, r.h, UIWIN_HIGHLIGHT_EROR | UIWIN_HIGHLIGHT2);
                
                if(delmapsec.is_activated() && delmapsec.check_double_click_security())
                {
                    menu_selectionables[1] = 0;

                    if(Lclickdown)
                        delmapsec.end_timer();
                }
                else if(Lclickdown && !delmapsec.is_activated())
                {
                    delmapsec.start_timer();
                }
            }
            else
                draw_window(UI, r.x, r.y, r.w, r.h, UIWIN_HIGHLIGHT_EROR);

            if(delmapsec.is_activated())
                del.render(UI, TEXT_MOD_NONE);
                // del.render(UI, TEXT_MOD_HL_FORCED | TEXT_MOD_HL_WAVE);
            else
                del.render(UI, TEXT_MOD_NONE);
        }
        ///////////////////////////////////////////////////////////////

        if(world_selection_txt.size() > 4)
            draw_scroll_area(UI, 0.02f, 0.575f, UI->h*0.75f, world_selection_scroll, &menu_selectionables[1]);
    }
    
    if(game_state == STATE_NEW_MAP)
    {
        UI_text Name(font_monogram, NW_map_name, screen->w, screen->h,
        0.5, 0.05, 
        0.035, 
        nullptr, IDMENU_UNTITLED, TEXT_ALIGN_CENTER);

        float sizes_ydec = 0.f;

        if(NW_map_name.empty())
        {
            Name.refresh_text("{Enter New Map Name}");
            GPU_Rect r = Name.getRect();
            draw_window(UI, r.x, r.y, r.w, r.h, UIWIN_HIGHLIGHT_NONE);
        }
        else
        {
            GPU_Rect r = Name.getRect();

            if(menu_selected[4] != WORLD_NAME_OK)
                draw_window(UI, r.x, r.y, r.w, r.h, UIWIN_HIGHLIGHT_EROR);
            else
                draw_window(UI, r.x, r.y, r.w, r.h, UIWIN_HIGHLIGHT1);
            
            if(menu_selected[4] == WORLD_NAME_EXIST)
            {
                UI_text ex(font_monogram, NW_name_exist, screen->w, screen->h, 0.5, 0.125, 0.025);
                r = ex.getRect();

                draw_window(UI, r.x, r.y, r.w, r.h, UIWIN_HIGHLIGHT_EROR);

                ex.render(UI, TEXT_MOD_NONE);

                sizes_ydec += r.h/float(UI->h);
            }
        }

        Name.render(UI, TEXT_MOD_NONE);

        menu_selectionables[0] = -1;
        menu_selectionables[1] = -1;
        menu_selectionables[2] = -1;

        float fontsize = 0.0225;
        draw_txt_line(UI, 0.5, sizes_ydec+0.125, NW_SIZE_NAMES, 5, *menu_selected[0], &menu_selectionables[0], -1, fontsize);
        float yjump = fontsize*3;
        draw_txt_line(UI, 0.125+fontsize, 0.025 + yjump*2, NW_BIOMES_NAMES, 9, *menu_selected[1], &menu_selectionables[1], 1, fontsize);
        draw_txt_line(UI, 0.5, 0.95, NW_PRESET_NAMES, 4, *menu_selected[2], &menu_selectionables[2], -1, fontsize);

        if(*menu_selected[3])
        {
            UI_text gen(font_monogram, NW_generating, screen->w, screen->h, 0.5, 0.5, 0.025);

            gen.render(UI, TEXT_MOD_HL_FORCED | TEXT_MOD_HL_WAVE);
        }

        menu_selectionables[5] = -1;
        
        if(!NW_map_name.empty() && menu_selected[4] == WORLD_NAME_OK && *menu_selected[0] >= 0 && *menu_selected[1] >= 0 && *menu_selected[2] >= 0)
        {
            UI_text save(font_monogram, NW_save, screen->w, screen->h, 0.5, 0.875, fontsize);

            GPU_Rect r = save.getRect();

            draw_window(UI, r.x, r.y, r.w, r.h, UIWIN_HIGHLIGHT_GREEN);

            if(save.render(UI, TEXT_MOD_HL_ALL | TEXT_MOD_HL_WAVE))
            {
                menu_selectionables[5] = 0;
            }

            // draw_txt_line(UI, 0.5, 0.90, &NW_save, 1, 0, NULL, -1, fontsize);
        }
    }

    if(game_state == STATE_OPTIONS)
    {
        int opt_size = opt_font_size*screen->h;

        int cnt = 0;
        float menu_y = 0.214;
        float menu_yjumps = 0.075;

        auto i = options_toggleable.begin();
        auto j = options_buttons.begin();

        float button_x = ((**i).centered_posx - 2.5*opt_size)/(float)(screen->w);

        draw_window(UI, (**i).centered_posx+opt_size*6,
                        (**i).centered_posy+opt_size*4.25, 
                        opt_size*21, 
                        opt_size*12);

        for(; i != options_toggleable.end() && 
              j != options_buttons.end(); 
              i++, j++)
        {
            if(cnt < options_scroll.beg || cnt > options_scroll.end)
            {
                cnt ++;
                continue;
            }

            (**i).centered_posy = menu_y*screen->h;
            (**j).change_position_norm(button_x*screen->w, menu_y*screen->h);

            (**i).render(UI, TEXT_MOD_NONE);



            int hlflags = UIWIN_HIGHLIGHT_NONE;

            if((**j).is_mouse_over())
            {
                hlflags += UIWIN_HIGHLIGHT2;
                Menu_hl_name = (**j).nameid;
            }

            if((**j).atlas->src.x)
            {
                hlflags = UIWIN_HIGHLIGHT1;
            }

            draw_window(UI, (**j).getx(), (**j).gety(), (**j).getw(), (**j).geth(), hlflags);

            (**j).render(UI);

            cnt ++;
            menu_y += menu_yjumps;
        }

        int x = (*options_resolutions.begin())->centered_posx;
        int y = (*(options_resolutions.begin().operator++()))->centered_posy;
        draw_window(UI, x, y, opt_size*15, opt_size*9);

        bool fullscreen = GPU_GetFullscreen();

        SDL_DisplayMode DM;
        SDL_GetCurrentDisplayMode(0, &DM);

        bool borderless = false;

        if(!fullscreen && DM.w == screen->w && DM.h == screen->h)
            borderless = true;

        for(auto i = options_resolutions.begin(); i != options_resolutions.end(); i++)
        {
            if(
                (fullscreen && (**i).nameid == IDMENU_OPTIONS_RESOLUTION_FLLSCR)  ||
                ((**i).nameid == IDMENU_OPTIONS_RESOLUTION_CUSTOM) ||
                (borderless && (**i).nameid == IDMENU_OPTIONS_RESOLUTION_BORLSS)
            ){
                draw_window(UI, (**i).centered_posx, 
                                (**i).centered_posy, 
                                opt_size*15, 
                                opt_size*2,
                                UIWIN_HIGHLIGHT1);
            }

            if((**i).render(UI, TEXT_MOD_HL_ALL | TEXT_MOD_SHADER_HL))
            {
                Menu_hl_name = (**i).nameid;
            }
        }

        while(j != options_buttons.end())
        {
            bool hl = (**j).is_mouse_over();
            if(hl)
            {
                font_menu_shader.activate();
                Menu_hl_name = (**j).nameid;
            }

            (**j).render(UI);

            font_menu_shader.deactivate();
            j++;
        }

        // float resratio = (screen->w/screen->h);
        // int controlw = opt_size*(TXT_OPTIONS_CONTROLS.size()+1)*resratio;

        // draw_window(UI, options_controls->centered_posx+controlw/2 - opt_size*resratio, 
        //                 options_controls->centered_posy,
        //                 controlw ,
        //                 opt_size*2);

        GPU_Rect r = options_controls->getRect();

        draw_window(UI, r.x, r.y, r.w, r.h);

        if(options_controls->render(UI, TEXT_MOD_HL_ALL | TEXT_MOD_SHADER_HL))
        {   
            Menu_hl_name = options_controls->nameid;
        }

        music_scroll.val = Mix_VolumeMusic(-1)*(music_scroll.max/MIX_MAX_VOLUME);
        if(music_scroll.val <= -9.8) music_scroll.val = music_scroll.max; // weird debug
        float tmpmus = music_scroll.val;

        sound_scroll.val = Mix_Volume(-1, -1)*(sound_scroll.max/MIX_MAX_VOLUME);
        if(sound_scroll.val <= -9.8) sound_scroll.val = sound_scroll.max; // weird debug
        float tmpsou = sound_scroll.val;

        float cx = 0.5f;
        float cy = 0.6f;
        float size = UI->w*0.3f;
        float ydec = 0.15;
        float xdec = 0.0;

        draw_scroll_area(UI, cx-xdec, cy, size, music_scroll, nullptr, true, 20);
        draw_scroll_area(UI, cx+xdec, cy+ydec, size, sound_scroll, nullptr, true, 20);

        int sv = (int)round(-music_scroll.val*10);
        sv -= sv%5;
        UI_text val(font_monogram, std::to_string(sv), screen->w, screen->h, cx-xdec, cy+0.0375, 0.020); 
        r = val.getRect();
        draw_window(UI, r.x, r.y, r.w, r.h);  
        val.render(UI, TEXT_MOD_NONE);

        UI_text music(font_monogram, TXT_OPTIONS_MUSICS, screen->w-xdec, screen->h, cx-xdec, cy-0.0375, 0.020);
        r = music.getRect();
        draw_window(UI, r.x, r.y, r.w, r.h);  
        music.render(UI, TEXT_MOD_NONE);

        sv = (int)round(-sound_scroll.val*10);
        sv -= sv%5;
        UI_text val2(font_monogram, std::to_string(sv), screen->w, screen->h, cx+xdec, cy+ydec+0.0375, 0.020); 
        r = val2.getRect();
        draw_window(UI, r.x, r.y, r.w, r.h);  
        val2.render(UI, TEXT_MOD_NONE);

        UI_text sound(font_monogram, TXT_OPTIONS_SOUNDS, screen->w+xdec, screen->h, cx+xdec, cy+ydec-0.0375, 0.020); 
        r = sound.getRect();
        draw_window(UI, r.x, r.y, r.w, r.h);  
        sound.render(UI, TEXT_MOD_NONE);

        if(tmpmus != music_scroll.val)
        {
            AE->set_music_volume(music_scroll.val*-10);
        }

        if(tmpsou != sound_scroll.val && abs(tmpsou - sound_scroll.val) > 0.1)
        {
            AE->set_sound_volume(sound_scroll.val*-10);
            AE->Play_click(2);
        }

    }

    if(game_state == STATE_BLOCK_SELECTION)
    {

        // GPU_RectangleFilled2(UI, {(float)0.15*screen->w, 
        //                           (float)0.15*screen->h, 
        //                           (float)0.7*screen->w, 
        //                           (float)0.7*screen->h}, 
        //                           {35, 35, 35, 255});

        int line = 8;
        if(screen->w >= 1920) line = 16;
        int linemax = line*floor(4.0*screen->h/600.0);

        float dist = 0.35;
        float xmin = 0.50*screen->w + 96*(- line/2.0 - dist);
        float ymin = 0.20*screen->h + 96*(- 0.5 - dist);
        float w = 96*(line + dist*2.0);
        float h = 96*(linemax/line + dist*2.0);

        // window_shader.activate();

        // int win_const[4] = {screen->w, screen->h, (int)round(w), (int)round(h)};

        // GPU_SetUniformiv(5, 4, 1, win_const);
        // GPU_SetUniformf(1, timems/1000.0);
        // GPU_Rect dst = {xmin, ymin, w, h};
        // GPU_BlitRect(RE->Textures[BLOCK_AO]->ptr, NULL, UI, &dst);

        // window_shader.deactivate();

        draw_window(screen, xmin+w/2.0, ymin+h/2.0, w, h);

        UI_tile block(
        block_atlasx32, 16, 16, 0, 
        screen->w, screen->h, 
        0.10, 0.10,
        0.25, 0.20);

        auto b = unlocked_blocks.begin();
        int i = 0;

        // int line = floor(5.0*screen->w/800.0);

        new_current_block = 0;

        for(int j = 0; j/line < bs_line_min && b != unlocked_blocks.end(); b++, j++);

        while(b != unlocked_blocks.end() && (i/line) < 8 && linemax)
        {
            block.change_atlas_id((*b) - 1);

            // float x = (0.5 + 0.05*(i%line - line/2.0 + 0.5))*screen->w;
            // float y = (0.25 + 0.075*(i/line))*screen->h;

            float x = 0.50*screen->w + 96*(i%line - line/2.0 + 0.5);
            float y = 0.20*screen->h + 96*(i/line);

            block.change_position_norm(x, y);

            // block.change_size_norm(0.10*screen->h, 0.10*screen->h);

            block.change_size_norm(64, 64);

            if(block.is_mouse_over())
            {
                new_current_block = *b;

                // block.change_size_norm(0.15*screen->h, 0.15*screen->h);
                // block.render(UI);
                // block.change_size_norm(0.10*screen->h, 0.10*screen->h);

                block.change_size_norm(192, 192);
            }
            else
                block.change_size_norm(128, 128);
            
            block.render(UI);

            b++;
            i++;
            linemax --;
        }
        
    }

    if(game_state == STATE_CONSTRUCTION || game_state == STATE_BLOCK_SELECTION)
    {
        new_cb_id = -1;

        // int blocksize = 128;
        // int blocksize = 64 + cos(timems/1000.0)*64;
        float blocksize = 96.0 * screen->h/1080.0;
        int winh = blocksize*0.75;
        int ypos = screen->h-blocksize/2;

        draw_window(UI, screen->w*0.5, ypos, blocksize*8, winh);

        // GPU_SetImageFilter(block_atlasx32->ptr, GPU_FILTER_LINEAR);
        for(int i = 0; i < 8; i++)
        {
            currentblocks[i]->refresh_atlas_id();

            currentblocks[i]->change_position_norm(0.5*screen->w + blocksize*(3.5-i), ypos);

            if(cb_id == i)
            {
                // currentblocks[i]->change_size_norm(0.20*screen->h, 0.20*screen->h);
                // currentblocks[i]->render(UI);
                // currentblocks[i]->change_size_norm(0.15*screen->h, 0.15*screen->h);

                // currentblocks[i]->change_size_norm(160, 160);

                currentblocks[i]->change_size_norm(blocksize, blocksize);

                draw_window(screen, currentblocks[i]->getx(), 
                                    ypos,
                                    blocksize,
                                    winh, 
                                    UIWIN_HIGHLIGHT1);
            }
            else
            {
                currentblocks[i]->change_size_norm(blocksize, blocksize);

                if(currentblocks[i]->is_mouse_over())
                {
                    forced_cursor = 0;
                    new_cb_id = i;

                    draw_window(screen, currentblocks[i]->getx(), 
                                        ypos,
                                        blocksize,
                                        winh, 
                                        UIWIN_HIGHLIGHT2);
                }
                else
                {
                    // draw_window(screen, currentblocks[i]->getx(), 
                    //                     currentblocks[i]->gety(),
                    //                     blocksize,
                    //                     blocksize);
                }
            }

            currentblocks[i]->render(UI);

        }

        int flag = UIWIN_HIGHLIGHT_NONE;

        menu_selectionables[0] = -1;
        if(construction_bs->is_mouse_over())
        {
            forced_cursor = 0;
            flag = UIWIN_HIGHLIGHT2;
            menu_selectionables[0] = 0;
        }
        
        if(game_state == STATE_BLOCK_SELECTION)
            flag = UIWIN_HIGHLIGHT1;

        construction_bs->change_position_norm(screen->w*0.5, ypos-blocksize/2 - screen->h*0.01);

        draw_window(UI, construction_bs->getx(), construction_bs->gety(), construction_bs->getw(), construction_bs->geth(), flag);

        construction_bs->render(UI);
    }

    if(game_state == STATE_CONSTRUCTION)
    {
        if(RE->highlight_type == HIGHLIGHT_PIPETTE)
            forced_cursor = 0;

        int blocksize = 128;
        if(screen->h < 1080)
            blocksize = 64;
        // float blocksize = 128 * screen->h/1080;
        int winh = blocksize*0.75;
        int ypos = blocksize/2;

        main_game_hl_mode->change_size_norm(blocksize*5, blocksize);
        main_game_hl_mode->change_position_norm(0.5*screen->w, ypos);

        new_hl_type = -1;
        
        if(main_game_hl_mode->is_mouse_over())
        {
            forced_cursor = 0;

            coord2D test = main_game_hl_mode->is_mouse_over_parts({5, 1});
            new_hl_type = test.x;
            // RE->highlight_type = test.x;
            // std::cout << test.x << " " << test.y << "\n";
        }

        int x = main_game_hl_mode->getx();
        // int y = main_game_hl_mode->gety();
        int w = main_game_hl_mode->getw();
        // int h = main_game_hl_mode->geth();

        draw_window(screen, x, ypos, w, winh);

        if(RE->highlight_type)
        {
            draw_window(screen, x-(w*(3.0-RE->highlight_type))/5.0, ypos, w/5.0, winh, UIWIN_HIGHLIGHT1);

            if(new_hl_type != -1 && new_hl_type != RE->highlight_type-1)
                draw_window(screen, x-(w*(2.0-new_hl_type))/5.0, ypos, w/5.0, winh, UIWIN_HIGHLIGHT2);
        }

        // main_game_hl_type->change_size_norm(0.01*screen->h, 0.01*screen->h);
        main_game_hl_mode->render(UI, true);
        // main_game_hl_type->render(UI, false);


        // SDL_GetMouseState((int*)&mouse.x, (int*)&mouse.y);
        // main_game_hl_mode->change_position_norm(mouse.x, mouse.y);
        // main_game_hl_mode->render(UI, false);


        if(RE->highlight_wcoord2.x != HIGHLIGHT_NOCOORD && RE->highlight_type != HIGHLIGHT_PIPETTE)
        {
            int diffx = abs(RE->highlight_wcoord2.x - RE->highlight_wcoord.x);
            int diffy = abs(RE->highlight_wcoord2.y - RE->highlight_wcoord.y);
            int diffz = RE->highlight_wcoord2.z - RE->highlight_wcoord.z;

            std::string test;

            test += ' ';

            if(diffx > 0)
            {
                test += format(diffx+1);
                test += ' ';
            }

            if(diffy > 0)
            {
                test += format(diffy+1);
                test += ' ';
            }

            if(diffz != 0)
            {
                if(diffz > 0)
                    test += '-';

                test += format(abs(diffz)+1);
                test += ' ';
            }

            if(test != " ")
            {
                UI_text bsi
                (
                    font_monogram,
                    test,
                    UI->w, UI->h,
                    ((float)mouse.x)/UI->w, 
                    ((float)mouse.y - 50.f)/UI->h,
                    0.017,
                    &font_menu_shader
                );

                // int optsize = screen->h*0.017;
                // int w = optsize*test.size()+2;
                // int h = optsize*2.75;
                // draw_window(UI, bsi.centered_posx, bsi.centered_posy-optsize*0.25, w, h);

                GPU_Rect r = bsi.getRect();
                draw_window(UI, r.x, r.y, r.w, r.h);

                bsi.render(UI, TEXT_MOD_NONE);
            }

        }


        if(RE->highlight_mode)
        {
            // std::cout << RE->highlight_mode << "\n";
            int winflag = 0;
            std::string name;

            switch (RE->highlight_mode)
            {
            case HIGHLIGHT_MOD_PLACE:
                winflag = UIWIN_HIGHLIGHT_GREEN;
                name = "PLACING";
                break;

            case HIGHLIGHT_MOD_PLACE_ALT:
                winflag = UIWIN_HIGHLIGHT1;
                name = "PLACING BEHIND";
                break;

            case HIGHLIGHT_MOD_DELETE :
                winflag = UIWIN_HIGHLIGHT_EROR;
                name = "DELETING";
                break;

            case HIGHLIGHT_MOD_REPLACE :
                winflag = UIWIN_HIGHLIGHT_EROR;
                name = "REPLACING";
                break;

            default:
                break;
            }

            UI_text hlm(font_monogram, name, screen->w, screen->h, 0.5, 0.15, 0.0175);
            hlm.centered_posy = ypos + winh/2.0 + screen->h*0.03;
            GPU_Rect r = hlm.getRect();
            draw_window(UI, r.x, r.y, r.w, r.h, winflag);
            hlm.render(UI, TEXT_MOD_NONE);
        }
    

        int flag = UIWIN_HIGHLIGHT_NONE;

        if(construction_meteo->is_mouse_over())
        {
            forced_cursor = 0;
            menu_selectionables[15] = 1;
            flag = UIWIN_HIGHLIGHT2;

            if(Lclickdown)
            {
                AE->Play_click();
                meteo_option_activated = !meteo_option_activated;
            }
        }
        
        if(meteo_option_activated)
            flag = UIWIN_HIGHLIGHT1;

        construction_meteo->change_position_norm(screen->w*0.9725, ypos);

        draw_window(UI, construction_meteo->getx(), construction_meteo->gety(), construction_meteo->getw(), construction_meteo->geth(), flag);

        construction_meteo->render(UI);

        //////////////////////////////////////////////////////////////

        flag = UIWIN_HIGHLIGHT_NONE;

        if(construction_help->is_mouse_over())
        {
            forced_cursor = 0;
            menu_selectionables[15] = 1;
            flag = UIWIN_HIGHLIGHT2;

            if(Lclickdown)
            {
                AE->Play_click();
                help_activated = !help_activated;
            }
        }
        
        if(help_activated)
            flag = UIWIN_HIGHLIGHT1;

        construction_help->change_position_norm(screen->w*0.0275, ypos);

        draw_window(UI, construction_help->getx(), construction_help->gety(), construction_help->getw(), construction_help->geth(), flag);

        construction_help->render(UI);

        //////////////////////////////////////////////////////////////

        flag = UIWIN_HIGHLIGHT_NONE;

        if(construction_mask_mode->is_mouse_over())
        {
            forced_cursor = 0;
            menu_selectionables[15] = 1;

            flag = UIWIN_HIGHLIGHT2;

            if(Lclickdown)
            {
                AE->Play_click();
                RE->world.mask_mod = !RE->world.mask_mod;
            }
        }
        
        if(RE->world.mask_mod)
            flag = UIWIN_HIGHLIGHT1;

        construction_mask_mode->change_position_norm(screen->w*(0.0275 + 0.05), ypos);

        draw_window(UI, construction_mask_mode->getx(), construction_mask_mode->gety(), construction_mask_mode->getw(), construction_mask_mode->geth(), flag);

        construction_mask_mode->render(UI);

        if(construction_mask_mode->is_mouse_over())
        {
            forced_cursor = 0;
            draw_mouse_indications(UI, Mask_Mode_Indication, TEXT_ALIGN_LEFT);
        }

        if(construction_help->is_mouse_over())
        {
            forced_cursor = 0;
            draw_mouse_indications(UI, Help_Indication, TEXT_ALIGN_LEFT);
        }

        if(construction_meteo->is_mouse_over())
        {
            forced_cursor = 0;
            draw_mouse_indications(UI, Meteo_Indication, TEXT_ALIGN_RIGHT);
        }

        if(RE->world.lights.is_full && timems-RE->world.lights.time_since_last_full < 4000)
        {
            // UI_text expl(font_monogram, ALERT_Lights, screen->w, screen->h, 0, 0, 0.0125, nullptr, IDMENU_UNTITLED, TEXT_ALIGN_CENTER);
            // expl.centered_posx = mouse.x;
            // expl.centered_posy = mouse.y + screen->h*0.025;
            // GPU_Rect winr = expl.getRect();
            // draw_window(UI, winr.x, winr.y, winr.w, winr.h, UIWIN_HIGHLIGHT_EROR);
            // expl.render(UI, TEXT_MOD_NONE);

            draw_mouse_indications(UI, ALERT_Lights, TEXT_ALIGN_CENTER, UIWIN_HIGHLIGHT_EROR);
        }
    }

    if(game_state == STATE_CONSTRUCTION && meteo_option_activated)
    {
        menu_selectionables[1] = -1;
        // I hate std::list
        std::list<int>::iterator *currmeteo = (std::list<int>::iterator*)menu_selected[1];
        draw_txt_line(UI, 0.975, 0.15, MeteoNames, 7, **currmeteo - 1, &menu_selectionables[1], 1, 0.02, TEXT_ALIGN_RIGHT);

        // if(menu_selectionables[1] == -1)
        //     forced_cursor = 0;
    }

    if(game_state == STATE_CONSTRUCTION && help_activated)
    {
        // draw_txt_line(UI, 0.025, 0.15, Tutorial, 7, -1, &menu_selectionables[1], 1, 0.02, TEXT_ALIGN_LEFT);
        GPU_Rect r;
        int flag = UIWIN_HIGHLIGHT_ALT | UIWIN_HIGHLIGHT_GREEN;
        // float ydec = screen->h*0.04625;

        UI_text tuto(font_monogram, "x", screen->w, screen->h, 0.025, 0.15, 0.0175, nullptr, IDMENU_UNTITLED, TEXT_ALIGN_LEFT);
        int i = 0;

        int ydec = tuto.getRect().h*1.15;

        for(; i < 5; i++)
        {
            tuto.refresh_text(Tutorial[i]);

            r = tuto.getRect();
            draw_window(UI, r.x, r.y, r.w, r.h, flag);

            tuto.render(UI, TEXT_MOD_NONE);
            tuto.centered_posy += ydec;
        }           

        std::string name;

        name = SDL_GetKeyName(SDL_GetKeyFromScancode(Cinpt.CAM_ROTL.code));
        name += "/";
        name += SDL_GetKeyName(SDL_GetKeyFromScancode(Cinpt.CAM_ROTR.code));
        name += Tutorial[i++];
        tuto.refresh_text(name);
        r = tuto.getRect();
        draw_window(UI, r.x, r.y, r.w, r.h, flag);
        tuto.render(UI, TEXT_MOD_NONE);
        tuto.centered_posy += ydec;

        name = SDL_GetKeyName(SDL_GetKeyFromScancode(Cinpt.MODE_NEXT.code));
        name += "/";
        name += SDL_GetKeyName(SDL_GetKeyFromScancode(Cinpt.MODE_PREV.code));
        name += Tutorial[i++];
        tuto.refresh_text(name);
        r = tuto.getRect();
        draw_window(UI, r.x, r.y, r.w, r.h, flag);
        tuto.render(UI, TEXT_MOD_NONE);
        tuto.centered_posy += ydec;

        name = SDL_GetKeyName(SDL_GetKeyFromScancode(Cinpt.OPEN_BLSEl.code));
        name += Tutorial[i++];
        tuto.refresh_text(name);
        r = tuto.getRect();
        draw_window(UI, r.x, r.y, r.w, r.h, flag);
        tuto.render(UI, TEXT_MOD_NONE);
        tuto.centered_posy += ydec;

        name = SDL_GetKeyName(SDL_GetKeyFromScancode(Cinpt.TGLE_HUD.code));
        name += Tutorial[i++];
        tuto.refresh_text(name);
        r = tuto.getRect();
        draw_window(UI, r.x, r.y, r.w, r.h, flag);
        tuto.render(UI, TEXT_MOD_NONE);
        tuto.centered_posy += ydec;       

        name = SDL_GetKeyName(SDL_GetKeyFromScancode(Cinpt.TGLE_GRID.code));
        name += Tutorial[i++];
        tuto.refresh_text(name);
        r = tuto.getRect();
        draw_window(UI, r.x, r.y, r.w, r.h, flag);
        tuto.render(UI, TEXT_MOD_NONE);
        tuto.centered_posy += ydec;  

        name = SDL_GetKeyName(SDL_GetKeyFromScancode(Cinpt.QUICKSAVE.code));
        name += Tutorial[i++];
        tuto.refresh_text(name);
        r = tuto.getRect();
        draw_window(UI, r.x, r.y, r.w, r.h, flag);
        tuto.render(UI, TEXT_MOD_NONE);
        tuto.centered_posy += ydec;       

        name = SDL_GetKeyName(SDL_GetKeyFromScancode(Cinpt.QUICKLOAD.code));
        name += Tutorial[i++];
        tuto.refresh_text(name);
        r = tuto.getRect();
        draw_window(UI, r.x, r.y, r.w, r.h, flag);
        tuto.render(UI, TEXT_MOD_NONE);
        tuto.centered_posy += ydec;

        name = Tutorial[i++];
        tuto.refresh_text(name);
        r = tuto.getRect();
        draw_window(UI, r.x, r.y, r.w, r.h, flag);
        tuto.render(UI, TEXT_MOD_NONE);
        tuto.centered_posy += ydec;

        name = Tutorial[i++];
        tuto.refresh_text(name);
        r = tuto.getRect();
        draw_window(UI, r.x, r.y, r.w, r.h, flag);
        tuto.render(UI, TEXT_MOD_NONE);
        tuto.centered_posy += ydec;
    }

    if(game_state == STATE_CONTROLS || game_state == STATE_CONTROLS_SELECT)
    {
        float opt_font_sizetmp = opt_font_size;

        opt_font_size *= 0.85;

        int nb_controls = sizeof(Construction_cinputs)/sizeof(keybutton);

        float ocsmin = -29;
        float ocsmax = 0;

        if(option_controls_scroll < ocsmin)
            option_controls_scroll = ocsmin;

        if(option_controls_scroll > ocsmax)
            option_controls_scroll = ocsmax;


        keybutton *controls = (keybutton*)&Cinpt;

        float opt_size = opt_font_size*screen->h;
        float yjumps = 3.0 * opt_size;
        float basey = 0.1 + 50.0*option_controls_scroll/screen->h;
        float basex = 0.5 - opt_font_size*3;

        UI_text uicont
        (
            font_monogram,
            "NULL",
            screen->w,
            screen->h,
            basex, basey,
            opt_font_size,
            nullptr,
            IDMENU_UNTITLED,
            TEXT_ALIGN_LEFT
        );

        float uibuttonfontscale = 0.65;

        UI_text uiname
        (
            font_monogram,
            "NULL",
            screen->w,
            screen->h,
            basex-(opt_font_size*7.0*screen->h)/screen->w, basey,
            opt_font_size*uibuttonfontscale,
            nullptr,
            IDMENU_UNTITLED,
            TEXT_ALIGN_CENTER
        );

        int flags;
        int fontflag;

        float winh = 0.5*opt_size*nb_controls + 0.85*yjumps*nb_controls;
        float winx = 0.5*screen->w;
        // float winy = basey*screen->h + (1.0/2.0)*(opt_size*nb_controls + yjumps*nb_controls*screen->h);
        float winy = basey*screen->h + 0.5*winh + opt_size;
        float winw = opt_size*45;
        // float winh = opt_size*nb_controls + yjumps*nb_controls*screen->h;

        draw_window(UI, winx, winy, winw, winh, UIWIN_HIGHLIGHT_NONE);

        scrollmenu_id = -1;

        for(int i = 0; i < nb_controls; i++)
        {
            fontflag = TEXT_MOD_NONE;
            flags = UIWIN_HIGHLIGHT1;

            uicont.refresh_text(controls[i].name);
            uicont.centered_posy += yjumps;
            
            std::string name = SDL_GetKeyName(SDL_GetKeyFromScancode(controls[i].code));
            uiname.refresh_text(name);
            uiname.centered_posy += yjumps;

            winx = uiname.centered_posx;
            winy = uiname.centered_posy;
            winw = uibuttonfontscale*opt_size*(name.size()+2.5);
            winh = uibuttonfontscale*opt_size*3.0;

            if(curr_scrollmenu_id == -1 &&
               mouse.x > (winx-winw/2) && mouse.x < (winx+winw/2) && 
               mouse.y > (winy-winh/2) && mouse.y < (winy+winh/2))
               {
                flags = UIWIN_HIGHLIGHT2;
                scrollmenu_id = i;
               }
            
            if(curr_scrollmenu_id == i)
            {
                flags = UIWIN_HIGHLIGHT2;
                fontflag = TEXT_MOD_HL_WAVE | TEXT_MOD_HL_FORCED;
            }

            draw_window(UI, winx, winy, winw, winh, flags);

            uiname.render(UI, fontflag);
            uicont.render(UI, TEXT_MOD_NONE);
        }

        opt_font_size = opt_font_sizetmp;
    }

    // font_menu_shader.deactivate();

    // GPU_Blit(UI_image, NULL, screen_tmp, 0, 0);
    // GPU_BlitRect(UI_image, NULL, screen_tmp, NULL);


    if(forced_cursor != -1 && forced_cursor != curent_cursor)
    {
        curent_cursor = forced_cursor;
        SDL_SetCursor(cursors[curent_cursor]);
    }
    
    if(forced_cursor == -1 && current_custom_cursor != curent_cursor)
    {
        curent_cursor = current_custom_cursor;
        SDL_SetCursor(cursors[curent_cursor]);
    }

    Lclickdown = false;
    Lclickup = false;
}

void UI_Engine::set_option(int atlas, const std::string& id)
{
    for(auto i = options_buttons.begin(); i != options_buttons.end(); i++)
    {
        if((**i).nameid == id)
        {
            (**i).change_atlas_id(atlas);
        }
    }
}

void UI_Engine::set_ui_current_blocks(int cb_id, int bolck_id)
{
    currentblocks[cb_id]->change_atlas_id(bolck_id-1);
}

void UI_Engine::set_ui_hl_mode(int id)
{
    // main_game_hl_mode->change_atlas_id(id);

    // if(!id)
    //     SDL_SetCursor(cursors[id]);

    current_custom_cursor = id;
}

void UI_Engine::set_ui_hl_type(int id)
{
    main_game_hl_type->change_atlas_id(id-1);
}

void UI_Engine::set_currres(int curres)
{
    std::string newres;

    if(curres != -1)
    {
        newres += std::to_string(resolutions[curres].w);
        newres += "x";
        newres += std::to_string(resolutions[curres].h);
    }
    else
    {
        SDL_DisplayMode DM;
        SDL_GetCurrentDisplayMode(0, &DM);

        newres += std::to_string(DM.w);
        newres += "x";
        newres += std::to_string(DM.h);

    }

    options_resolutions.back()->refresh_text(newres);
}

void UI_Engine::generate_tiles(int game_state, int screenw, int screenh)
{
    /*
        filename, atlas_col, atlas_line, atlas default id
        screenw, screenh,
        with onscreen, height onscreen
        centered pos x onscreen, centered pos y onscreen

        ==> tile w/h onscreen are normalized only from the height of the screen
        ==> Positions are normalized from the width and the height of the screen
    */
    
    // old
    // screenh *= UI_SIZEDIFF;
    // screenw *= UI_SIZEDIFF;
    if(game_state == -3)
    {
        UI_SIZEDIFF = 1;

        screenh *= UI_SIZEDIFF;
        screenw *= UI_SIZEDIFF;

        if(UI_image) GPU_FreeImage(UI_image);
        if(UI) GPU_FreeTarget(UI);

        UI_image = GPU_CreateImage(screenw, screenh, GPU_FORMAT_RGBA);
        UI = GPU_LoadTarget(UI_image);
        GPU_SetAnchor(UI_image, 0, 0);

        GPU_SetImageFilter(UI_image, GPU_FILTER_NEAREST);
    }

    if(game_state == -2)
    {
        font_title_shader.load("shader/UI/UI_font.vert", "shader/UI/UI_font_title.frag", NULL);
        font_menu_shader.load("shader/UI/UI_font.vert", "shader/UI/UI_font_menu.frag", NULL);
        font_red_shader.load("shader/UI/UI_font.vert", "shader/UI/UI_font_red.frag", NULL);
        window_shader.load("shader/UI/UI_font.vert", "shader/UI/window.frag", NULL);
    }

    if(game_state == -1)
    {
        char cur_names[5][256] = 
        {
            "ressources/textures/ui/cursor.png",
            "ressources/textures/ui/cursor_del.png",
            "ressources/textures/ui/cursor_rep.png",
            "ressources/textures/ui/cursor_add.png",
            "ressources/textures/ui/cursor_alt.png"
        };

        for(int i = 0; i < 5; i++)
        {
            SDL_Surface *surface;

            surface = GPU_LoadSurface(cur_names[i]);
            cursors[i] =  SDL_CreateColorCursor(surface, 32, 32);

            SDL_FreeSurface(surface);
        }

        font_round2 = std::make_shared<UI_tile>(
        "ressources/textures/ui/font/round2.png", 26, 3, 0, 
        screenw, screenh, 
        0.20, 0.20,
        0.0, 0.0);

        font_monogram = std::make_shared<UI_tile>(
        "ressources/textures/ui/font/monogram.png", 26, 3, 0, 
        screenw, screenh, 
        0.20, 0.20,
        0.0, 0.0);

        UI_scale = 1.0;

        Shader_Error_img.init_from_file("ressources/textures/ui/Shader_Error.png");

        GPU_SetImageFilter(Shader_Error_img.ptr, GPU_FILTER_LINEAR);

        init_ASCII_to_ATLAS();

        SDL_SetCursor(cursors[0]);
        curent_cursor = 0;
    }
    
    if(game_state == STATE_MAIN_MENU)
    {
        while(!main_menu.empty()) main_menu.pop_back();

        float main_menu_yjumps = 0.115;
        float main_menu_size = 0.0375;
        float main_menu_y = 0.635 - 0.5*(4*main_menu_size + 3*main_menu_yjumps);

        main_menu.push_back(std::make_shared<UI_text>(
        font_round2,
        TXT_MAIN_MENU_PLAY,
        screenw, screenh,
        0.5, main_menu_y,
        main_menu_size,
        &font_menu_shader,
        IDMENU_PLAY
        ));

        main_menu_y += main_menu_yjumps;
        main_menu.push_back(std::make_shared<UI_text>(
        font_round2,
        TXT_MAIN_MENU_OPTIONS,
        screenw, screenh,
        0.5, main_menu_y,
        main_menu_size,
        &font_menu_shader,
        IDMENU_OPTIONS
        ));

        main_menu_y += main_menu_yjumps;
        main_menu.push_back(std::make_shared<UI_text>(
        font_round2,
        TXT_MAIN_MENU_CREDITS,
        screenw, screenh,
        0.5, main_menu_y,
        main_menu_size,
        &font_menu_shader,
        IDMENU_CREDITS
        ));

        main_menu_y += main_menu_yjumps;
        main_menu.push_back(std::make_shared<UI_text>(
        font_round2,
        TXT_MAIN_MENU_QUIT,
        screenw, screenh,
        0.5, main_menu_y,
        main_menu_size,
        &font_menu_shader,
        IDMENU_QUIT   
        ));

        main_menu_version = std::make_unique<UI_text>(
        font_round2,
        VERSION,
        screenw, screenh,
        0.5, 0.9,
        0.02);

        float size = 0.210;

        if(timems_start%128 == 0)
        {
            SDL_Surface*image = GPU_LoadSurface("ressources/textures/ui/logo/Nothing here/icon_trans.png");
            SDL_SetWindowIcon(SDL_GL_GetCurrentWindow(), image);
            SDL_FreeSurface(image);
            main_menu_logo = std::make_shared<UI_tile>(
                "ressources/textures/ui/logo/Nothing here/logo_trans.png", 1, 1, 0, screenw, screenh, 
                size*2321.f/865.f, size, 
                0.5, 0.125);
        }
        else if(timems_start%1024 == 1)
        {
            main_menu_logo = std::make_shared<UI_tile>(
                "ressources/textures/ui/logo/Nothing here/logo_mc.png", 1, 1, 0, screenw, screenh, 
                size*2321.f/865.f, size, 
                0.5, 0.125);
        }
        else
        {
            main_menu_logo = std::make_shared<UI_tile>(
                "ressources/textures/ui/logo/logo.png", 1, 1, 0, screenw, screenh, 
                size*2321.f/865.f, size, 
                0.5, 0.125);
        }
        
        // G:\Mon Drive\Projets DEV\iso-voxel-game\ressources\textures\ui\logo\Nothing here


    }
    
    if(game_state == STATE_OPTIONS)
    {
        while(!options_buttons.empty()) options_buttons.pop_back();
        while(!options_resolutions.empty()) options_resolutions.pop_back();
        while(!options_toggleable.empty()) options_toggleable.pop_back();

        options_scroll.init(3, 4);

        float menu_y = 0.3;
        float menu_yjumps = 0.125;
        // opt_font_size = 0.035;
        opt_font_size = 0.0275;
        float opt_x = 0.630;
        // float opt_size = opt_font_size*screenh;

        const std::string opt_names[] = 
        {
            // TXT_OPTIONS_VOLUME,
            // TXT_OPTIONS_FULLSCREEN,
            // TXT_OPTIONS_RESOLUTION,
            TXT_OPTIONS_VSYNC,
            TXT_OPTIONS_SHADOWS,
            TXT_OPTIONS_BLOCKS_BORDER,
            TXT_OPTIONS_AO
        };

        const std::string opt_idmenu[] =
        {
            // IDMENU_OPTIONS_VOLUME,
            // IDMENU_OPTIONS_FULLSCREEN,
            // IDMENU_OPTIONS_RESOLUTION,
            IDMENU_OPTIONS_VSYNC,
            IDMENU_OPTIONS_SHADOWS,
            IDMENU_OPTIONS_BLOCKS_BORDER,
            IDMENU_OPTIONS_AO
        };

        for(int i = 0; i < 4; i++)
        {
            options_toggleable.push_back(std::make_shared<UI_text>(
            font_monogram,
            opt_names[i],
            screenw, screenh,
            opt_x, menu_y,
            opt_font_size,
            &font_menu_shader,
            opt_idmenu[i],
            TEXT_ALIGN_LEFT
            ));
            
            options_buttons.push_back(std::make_shared<UI_tile>(
            "ressources/textures/ui/buttons/onoff.png",
            2, 1, 0, 
            screenw, screenh, 
            0.06, 0.06,
            opt_x-0.10, menu_y
            ));


            GPU_SetImageFilter(options_buttons.back()->atlas->ptr, GPU_FILTER_LINEAR);

            options_buttons.back()->nameid = opt_idmenu[i];

            menu_y += menu_yjumps;
        };
    
        float resx = 0.50 - 0.15;
        float resy = 0.20;
        float resyjumps = 0.075;

        options_resolutions.push_back(std::make_shared<UI_text>(
        font_monogram,
        TXT_OPTIONS_RESOLUTION_FLLSCR,
        screenw, screenh,
        resx, resy,
        opt_font_size,
        &font_menu_shader,
        IDMENU_OPTIONS_RESOLUTION_FLLSCR,
        TEXT_ALIGN_CENTER
        ));


        resy += resyjumps;
        options_resolutions.push_back(std::make_shared<UI_text>(
        font_monogram,
        TXT_OPTIONS_RESOLUTION_BORLSS,
        screenw, screenh,
        resx, resy,
        opt_font_size,
        &font_menu_shader,
        IDMENU_OPTIONS_RESOLUTION_BORLSS,
        TEXT_ALIGN_CENTER
        ));

        resy += resyjumps;
        options_resolutions.push_back(std::make_shared<UI_text>(
        font_monogram,
        "...",
        screenw, screenh,
        resx, resy,
        opt_font_size,
        &font_menu_shader,
        IDMENU_OPTIONS_RESOLUTION_CUSTOM,
        TEXT_ALIGN_CENTER
        ));

        options_buttons.push_back(std::make_shared<UI_tile>(
        "ressources/textures/ui/buttons/left.png",
        1, 1, 0, 
        screenw, screenh, 
        0.04, 0.04,
        resx-(0.16*((float)(screenh)/(float)(screenw))), resy
        ));

        options_buttons.back()->nameid = IDMENU_OPTIONS_RESOLUTION_CUSTOM_PREV;

        options_buttons.push_back(std::make_shared<UI_tile>(
        "ressources/textures/ui/buttons/right.png",
        1, 1, 0, 
        screenw, screenh, 
        0.04, 0.04,
        resx+(0.16*((float)(screenh)/(float)(screenw))), resy
        ));

        options_buttons.back()->nameid = IDMENU_OPTIONS_RESOLUTION_CUSTOM_NEXT;
        
        options_controls = std::make_shared<UI_text>(
        font_monogram,
        TXT_OPTIONS_CONTROLS,
        screenw, screenh,
        resx, resy+(resyjumps*1.25),
        opt_font_size,
        &font_menu_shader,
        IDMENU_OPTIONS_CONTROLS,
        TEXT_ALIGN_CENTER
        );

        option_controls_scroll = 0.0;

        music_scroll.min = 0;
        music_scroll.max = -10;
        music_scroll.val = -5;
        sound_scroll.min = 0;
        sound_scroll.max = -10;
        music_scroll.val = -5;
    }

    if(game_state == STATE_WORLD_SELECTION)
    {
        std::list<std::string> worlds_names;

        std::string paths[3] = {"saves/", "saves/.sprites/", "saves/.test/"};

        for(int i = 0; i < 3; i++)
        {
            std::string path = paths[i];
            for (const auto & entry : fs::directory_iterator(path))
            {
                if(!entry.is_directory())
                    continue;

                std::string filename = entry.path().filename().string();

                if(filename[0] != '.')
                {
                    if(i > 0)
                        worlds_names.push_back(path.substr(6) + filename);
                    else
                        worlds_names.push_back(filename);
                }
            }
        }

        while(!world_selection.empty()) world_selection.pop_back();
        while(!world_selection_txt.empty()) world_selection_txt.pop_back();


        // worlds_names.push_back("HUT");
        // worlds_names.push_back("ISLAND");
        // worlds_names.push_back("PLAIN");
        // worlds_names.push_back("PLAYGROUND");
        // worlds_names.push_back(".SPRITES/PLAYER");

        float wstxt_y = 0.25;
        float wstxt_yjumps = 0.0225*4;
        float wstxt_x = 0.05;
        float wstxt_size = 0.0225;

        for(auto i = worlds_names.begin(); i != worlds_names.end(); i++)
        {
            world_selection_txt.push_back(std::make_shared<UI_text>(
            font_monogram,
            *i,
            screenw, screenh,
            wstxt_x, wstxt_y,
            wstxt_size,
            &font_menu_shader,
            *i,
            TEXT_ALIGN_LEFT   
            ));

            wstxt_y += wstxt_yjumps;
        }

        world_selection_scroll.val = 0;
        world_selection_scroll.max = (int)(world_selection_txt.size()*1.30*(screenh/1080.0));
        world_selection_scroll.min = 0;
    }
    
    if(game_state == STATE_CONSTRUCTION)
    {
        // current_block_shader.load("shader/UI_current_Block.vert", "shader/UI_current_Block.frag", NULL);
        
        // whome(currentblocks)

        if(!block_atlasx32)
        {
            block_atlasx32 = std::make_shared<Texture>();
            block_atlasx32->init_from_file("ressources/textures/ui/mosaic 32b.png");
            
            // GPU_SetImageFilter(block_atlasx32->ptr, GPU_FILTER_LINEAR);
            // std::cout << block_atlasx32->ptr->h << "\n";

            GPU_SetImageFilter(block_atlasx32->ptr, GPU_FILTER_LINEAR);
        }

        for(int i = 0; i < 8; i++)
        {
            currentblocks[i] = std::make_unique<UI_tile>(
            block_atlasx32, 16, 16, 0, 
            screenw, screenh, 
            0.15, 0.15,
            0.50 + (3.5-i)*0.15*((float)(screenh)/(float)(screenw)), 0.925);
            //0.50 + (3.5-i)*0.075, 0.90);
        }

        // resx-(0.22*((float)(screenh)/(float)(screenw)))

        float size_hltype = 0.11;

        main_game_hl_mode = std::make_unique<UI_tile>(
        "ressources/textures/ui/hilight_type.png", 1, 1, 0, 
        screenw, screenh, 
        size_hltype*5, size_hltype, 
        0.50, 100.0/screenh);

        // GPU_SetImageFilter(main_game_hl_mode->atlas->ptr, GPU_FILTER_LINEAR);
        GPU_SetImageFilter(main_game_hl_mode->atlas->ptr, GPU_FILTER_NEAREST);

        main_game_hl_type = std::make_unique<UI_tile>(
        "ressources/textures/ui/hilight_type.png", 5, 1, 0, 
        screenw, screenh, 
        0.20, 0.20, 
        0.50, 100.0/screenh);

        // GPU_SetImageFilter(main_game_hl_type->atlas->ptr, GPU_FILTER_LINEAR);
        GPU_SetImageFilter(main_game_hl_type->atlas->ptr, GPU_FILTER_NEAREST);
        
        // main_game_hl_type->nameid

        construction_bs = std::make_shared<UI_tile>(
            // "ressources/textures/ui/buttons/right.png",
            "ressources/textures/ui/buttons/block_selection.png", 
            1, 1, 0, screenw, screenh, 
            0.0375, 0.0375, 
            0.5, 0.870);

        construction_meteo = std::make_shared<UI_tile>(
            // "ressources/textures/ui/buttons/right.png",
            "ressources/textures/ui/buttons/meteo.png", 
            1, 1, 0, screenw, screenh, 
            0.05, 0.05, 
            0.5, 0.5);

        construction_help = std::make_shared<UI_tile>(
            // "ressources/textures/ui/buttons/right.png",
            "ressources/textures/ui/buttons/help.png", 
            1, 1, 0, screenw, screenh, 
            0.05, 0.05, 
            0.5, 0.5);
        
        construction_mask_mode = std::make_shared<UI_tile>(
            // "ressources/textures/ui/buttons/right.png",
            "ressources/textures/ui/buttons/mask mode.png", 
            1, 1, 0, screenw, screenh, 
            0.05, 0.05, 
            0.5, 0.5);
    }
}