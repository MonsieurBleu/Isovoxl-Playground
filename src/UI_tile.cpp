#include <game.hpp>

UI_tile::UI_tile(const char* filename, int acol, int aline, int aid,
                 int screenw, 
                 int screenh,
                 float sizex,
                 float sizey,
                 float x,
                 float y) 
                 : atlas_col(acol), atlas_line(aline), atlas_id(aid)
{
    atlas = std::make_shared<Texture>();

    if(filename)
        atlas->init_from_file(filename);

    atlas->set_atlas_srcrect(atlas_col, atlas_line, atlas_id);

    GPU_SetAnchor(atlas->ptr, 0.5, 0.5);

    sizex *= screenh;
    sizey *= screenh;

    x *= screenw;
    y *= screenh;

    dest.x = x;
    dest.y = y;

    dest.h = atlas->ptr->h/atlas_line;
    dest.w = atlas->ptr->w/atlas_col;

    scalex = (float)(sizex)/dest.w;
    scaley = (float)(sizey)/dest.h;
}

UI_tile::UI_tile(std::shared_ptr<Texture> t_atlas, int acol, int aline, int aid,
                 int screenw, 
                 int screenh,
                 float sizex,
                 float sizey,
                 float x,
                 float y) 
                 : atlas_col(acol), atlas_line(aline), atlas_id(aid)
{
    atlas = t_atlas;

    atlas->set_atlas_srcrect(atlas_col, atlas_line, atlas_id);

    GPU_SetAnchor(atlas->ptr, 0.5, 0.5);

    sizex *= screenh;
    sizey *= screenh;

    x *= screenw;
    y *= screenh;

    dest.x = x;
    dest.y = y;

    dest.h = atlas->ptr->h/atlas_line;
    dest.w = atlas->ptr->w/atlas_col;

    scalex = (float)(sizex)/dest.w;
    scaley = (float)(sizey)/dest.h;
}

void UI_tile::change_size_norm(float sizex, float sizey)
{
    sizex = ceil(sizex);
    sizey = ceil(sizey);

    sizex = (int)(sizex)%2 ? sizex : sizex-1;
    sizey = (int)(sizey)%2 ? sizey : sizey-1;

    scalex = (float)(sizex)/dest.w;
    scaley = (float)(sizey)/dest.h;
}

void UI_tile::change_position_norm(float x, float y)
{
    dest.x = x;
    dest.y = y;
}

int UI_tile::getx(){return dest.x;}
int UI_tile::gety(){return dest.y;}
int UI_tile::getw(){return dest.w*scalex;}
int UI_tile::geth(){return dest.h*scaley;}

bool UI_tile::is_mouse_over()
{
    int xlmin = dest.x - dest.w*scalex/2;
    int xlmax = dest.x + dest.w*scalex/2;

    int ylmin = dest.y - dest.h*scaley/2;
    int ylmax = dest.y + dest.h*scaley/2;

    pixel_coord mouseUI = {mouse.x*UI_SIZEDIFF, mouse.y*UI_SIZEDIFF};

    return mouseUI.x >= xlmin && mouseUI.x <= xlmax && mouseUI.y >= ylmin && mouseUI.y <= ylmax;
}

coord2D UI_tile::is_mouse_over_parts(coord2D div)
{
    int xlmin = dest.x - dest.w*scalex/2;
    int xlmax = dest.x + dest.w*scalex/2;

    int ylmin = dest.y - dest.h*scaley/2;
    int ylmax = dest.y + dest.h*scaley/2;

    pixel_coord mouseUI = {mouse.x*UI_SIZEDIFF, mouse.y*UI_SIZEDIFF};

    if(!(mouseUI.x >= xlmin && mouseUI.x <= xlmax && mouseUI.y >= ylmin && mouseUI.y <= ylmax))
        return {-1, -1};
    
    coord2D parts;

    parts.x = (mouseUI.x-xlmin)/((xlmax-xlmin)/div.x);
    parts.y = (mouseUI.y-ylmin)/((ylmax-ylmin)/div.y);

    if(parts.x > div.x-1) parts.x = div.x-1;
    if(parts.y > div.y-1) parts.y = div.y-1;

    return parts;
}

void UI_tile::change_atlas_id(int newaid)
{
    atlas_id = newaid;
    atlas->set_atlas_srcrect(atlas_col, atlas_line, atlas_id);
}

void UI_tile::refresh_atlas_id()
{
    atlas->set_atlas_srcrect(atlas_col, atlas_line, atlas_id);
}

void UI_tile::render(GPU_Target *screen, bool use_scale)
{
    if(atlas && atlas->ptr)
    {
        if(use_scale)
            GPU_BlitScale(atlas->ptr, &atlas->src, screen, dest.x, dest.y, scalex, scaley);

        else
            GPU_Blit(atlas->ptr, &atlas->src, screen, dest.x, dest.y);


        // GPU_Rect test;

        // test.x = dest.x;
        // test.y = dest.y;
        // test.w = scalex*dest.w;
        // test.h = scaley*dest.h;

        // GPU_BlitRect(atlas->ptr, &atlas->src, screen, &test);

    }
}