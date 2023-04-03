#ifndef UI_ENGINE_HPP
#define UI_ENGINE_HPP

#include <iostream>
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_gpu.h>

// struct Texture;
#include <render_engine.hpp>

// #include <texture.hpp>


#include <constants.hpp>
#include <texture.hpp>
#include <Shader.hpp>
#include <list>
#include <vector>

#include <inputs.hpp>

#define TEXT_ALIGN_LEFT   0
#define TEXT_ALIGN_CENTER 1
#define TEXT_ALIGN_RIGHT  2

#define TEXT_MOD_NONE          0
#define TEXT_MOD_SHADER_ALL    1
#define TEXT_MOD_SHADER_HL     2
#define TEXT_MOD_HL_BIGGER     4
#define TEXT_MOD_HL_WAVE       8
#define TEXT_MOD_HL_PERLETTER  16
#define TEXT_MOD_HL_ALL        32
#define TEXT_MOD_GET_ID        64
#define TEXT_MOD_HL_FORCED     128

#define UIWIN_HIGHLIGHT_NONE 0
#define UIWIN_HIGHLIGHT1 128
#define UIWIN_HIGHLIGHT2 64
#define UIWIN_HIGHLIGHT_EROR 32
#define UIWIN_HIGHLIGHT_GREEN 16
#define UIWIN_HIGHLIGHT_ALT   8

std::list<int>::iterator circularPrev(std::list<int> &, std::list<int>::iterator &);
std::list<int>::iterator circularNext(std::list<int> &, std::list<int>::iterator &);

extern std::string format(unsigned long long i);

void init_ASCII_to_ATLAS();

extern int UI_SIZEDIFF;

struct UI_tile_size
{
    float xmin;
    float xmax;

    float ymin;
    float ymax;
};

class UI_tile
{
    private :
        float scalex;
        float scaley;

        int dsizex;
        int dsizey;

        GPU_Rect dest;
        
        int atlas_col;
        int atlas_line;
        int atlas_id;

    public :
    
        std::shared_ptr<Texture> atlas;
        
        UI_tile(const char* filename, 
                int acol, 
                int aline, 
                int aid,
                int screenw,
                int screenh,
                float sizex,
                float sizey,
                float x,
                float y);

        UI_tile(std::shared_ptr<Texture> t_atlas, 
                int acol, 
                int aline, 
                int aid,
                int screenw,
                int screenh,
                float sizex,
                float sizey,
                float x,
                float y);

        // int get_id();

        void change_atlas_id(int newaid);
        void refresh_atlas_id();
        void change_size_norm(float sizex, float sizey);
        void change_position_norm(float x, float y);

        int getx();
        int gety();
        int getw();
        int geth();

        void render(GPU_Target *, bool use_scale = true);

        bool is_mouse_over();
        coord2D is_mouse_over_parts(coord2D div);

        std::string file_represented;
        std::string nameid;
};

class UI_text
{
    private :
        float charwith;

        int align;

        std::shared_ptr<UI_tile> font;
        std::vector<Uint8> text;

        Shader *shader;

    public :

        float centered_posx;
        float centered_posy;

        UI_text(
            std::shared_ptr<UI_tile> textfont,
            std::string const &strtext,
            int screenw,
            int screenh,
            float centered_x,
            float centered_y,
            float charsize,
            Shader *textshdr = nullptr,
            const std::string &textnameid = IDMENU_UNTITLED,
            int textalign = TEXT_ALIGN_CENTER
            );

        bool render(GPU_Target *, Uint64);

        void refresh_text(std::string const &strtext);

        GPU_Rect getRect();

        std::string textcpy;
        std::string nameid;
};

struct scroll_menu_val  // old
{
    int size;
    int winsize;
    int beg;
    int end;

    bool scroll(int y);
    void init(int _size, int _winsize);
};

struct scrool_val
{   
    float min;
    float max;
    float val;
    bool follow_mouse = false;

    void add(float);
};

class button_security
{
    private :
        bool   activated = false;
        Uint64 mstimer = 5000;
        Uint64 doubleclicksec = 500;
        Uint64 start;
    
    public :

        int get_timer_seg(int);
        bool is_activated();
        bool check_double_click_security();
        void start_timer();
        void handle();
        bool end_timer();
};

class UI_Engine
{
    SDL_Cursor *cursors[5];

    // can be overwrite in the fonction render_frame by forced_corsor
    // custom cursor requested by the game,
    int current_custom_cursor = 0; 
    int forced_cursor;
    int curent_cursor = 0; // current cursor id used by SDL

    std::list<std::shared_ptr<UI_tile>> world_selection;
    std::list<std::shared_ptr<UI_text>> world_selection_txt;
    std::list<std::shared_ptr<UI_text>> main_menu;

    std::list<std::shared_ptr<UI_text>> options_toggleable;
    std::list<std::shared_ptr<UI_tile>> options_buttons;
    std::list<std::shared_ptr<UI_text>> options_resolutions;

    std::shared_ptr<UI_text> options_controls;

    std::list<std::shared_ptr<UI_text>> build_size_indicator;

    std::unique_ptr<UI_text> main_menu_version;
    std::shared_ptr<UI_tile> main_menu_logo;

    std::unique_ptr<UI_tile> currentblocks[8];

    std::unique_ptr<UI_tile> main_game_hl_mode;
    std::unique_ptr<UI_tile> main_game_hl_type;

    std::shared_ptr<UI_tile> construction_bs;
    std::shared_ptr<UI_tile> construction_meteo;
    bool meteo_option_activated = false;
    std::shared_ptr<UI_tile> construction_mask_mode;

    std::shared_ptr<UI_tile> construction_help;
    bool help_activated = false;

    Shader font_title_shader;
    Shader font_menu_shader;
    Shader font_red_shader;
    Shader window_shader;

    float opt_font_size;

    std::shared_ptr<UI_tile> font_round2;
    std::shared_ptr<UI_tile> font_monogram;

    std::shared_ptr<Texture> block_atlasx32;

    Texture Shader_Error_img;
    

    void prepare_window_shader();

    void draw_window(GPU_Target *screen, float cx, float cy, float w, float h, Uint8 highlight_type = UIWIN_HIGHLIGHT_NONE);
    
    float draw_txt_line(GPU_Target *screen, 
                       float cx,
                       float cy, 
                       const std::string strs[],
                       const int nbstrs,
                       const int id_selected,
                       int *id_highlighted,
                       const int nb_perline = -1,
                       const float forced_fontsize = 0.f,
                       int align = TEXT_ALIGN_CENTER);

    float draw_scroll_area(GPU_Target *screen, 
                           float cx,
                           float cy,
                           float size,
                           scrool_val &sc,  
                           int *selected = NULL,
                           bool is_vertical = false,
                           int nbseg = -1);
    
    void draw_mouse_indications(GPU_Target *screen, const std::string &txt, int align, int winflags = UIWIN_HIGHLIGHT_NONE);

    public :

    UI_Engine();
    
    void set_ui_current_blocks(int cb_id, int bolck_id);
    void set_ui_hl_mode(int);
    void set_ui_hl_type(int);
    void set_option(int atlas, const std::string& id);
    void set_currres(int curres);

    void render_frame(int game_state, 
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
                      void *AEptr);

    void generate_tiles(int game_state, int screenw, int screenh);

    float option_controls_scroll;
    scrool_val world_selection_scroll;
    scrool_val music_scroll;
    scrool_val sound_scroll;

    GPU_Image *UI_image; // old
    GPU_Target *UI; // old

    float UI_scale; // old

    scroll_menu_val options_scroll; // old

    bool Lclickdown = false;
    bool Lclickup = false;

    button_security delmapsec;
};

#endif