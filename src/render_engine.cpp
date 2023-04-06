#include <game.hpp>

// bouger ça autre part
int    i32round(double x){return floor(x+0.5);}
short  i16round(double x){return floor(x+0.5);}
Uint16 ui16round(double x){return floor(x+0.5);}
/////////////////////////////

Uint64 Get_time_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

bool test;
Uint32 *primitives_buffer;
Uint32 *primitives_buffer_world;
Uint32 *primitives_buffer_transparent;

Render_Engine::Render_Engine(struct World& _world) : world{_world}
{ 
    abort_rendrefresh = false;

    primitives_buffer = new Uint32[6*4000000];
    primitives_buffer_world = new Uint32[6*4000000];
    primitives_buffer_transparent = new Uint32[6*4000000];

    low_gpu_mode_enable = false;
    force_nonpanorama_mode = false;

    sprite_counter = 0;
    world_sprite_counter = 0;
    transparent_sprite_counter = 0;

    DFIB_FBO = NULL;
    DFIB_screen = NULL;

    transparent_FBO = NULL;
    transparent_screen = NULL;

    hl_FBO = NULL;
    hl_screen = NULL;

    light_FBO = NULL;
    light_screen = NULL;

    Color_FBO = NULL;
    Color_screen = NULL;

    world = _world;
    grid_enable = false;
    shader_enable = true;
    SecondaryThread = NULL;

    highlight_wcoord2 = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};
    highlight_wcoord  = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};

    refresh_shaders();
}

Render_Engine::~Render_Engine()
{
    delete primitives_buffer;
    delete primitives_buffer_world;
    delete primitives_buffer_transparent;

    GPU_FreeImage(DFIB_FBO);
    GPU_FreeImage(Color_FBO);
    GPU_FreeImage(light_FBO);
    GPU_FreeImage(transparent_FBO);
    GPU_FreeImage(hl_FBO);
    GPU_FreeImage(hld_FBO);

    GPU_FreeTarget(DFIB_screen);
    GPU_FreeTarget(Color_screen);
    GPU_FreeTarget(light_screen);
    GPU_FreeTarget(transparent_screen);
    GPU_FreeTarget(hl_screen);
    GPU_FreeTarget(hld_screen);
    
    GPU_FreeTarget(screen);

    SDL_DestroyMutex(campg_mut);
}

void Render_Engine::refresh_shaders()
{
    DFIB_shader.reset();
    world_render_shader.reset();
    post_process_shader.reset();
    light_shader.reset();

    std::string geom = "shader/blocks.geom";
    std::string geoml = "shader/light.geom";
    DFIB_shader.load("shader/blocks.vert", "shader/blocks/DFIB.frag", &geom);
    light_shader.load("shader/light.vert", "shader/light.frag", &geoml);
    transparent_shader.load("shader/blocks.vert", "shader/blocks/transparent.frag", &geom);

    world_render_shader.load("shader/opaque_world.vert", "shader/opaque_world.frag", NULL);
    post_process_shader.load("shader/post process.vert", "shader/post process.frag", NULL);

    shader_features = 0;
    shader_features ^= SFEATURE_GLOBAL_ILLUMINATION;
    shader_features ^= SFEATURE_AMBIANT_OCCLUSION;
    shader_features ^= SFEATURE_BLOCK_BORDERS;
    shader_features ^= SFEATURE_SHADOWS;
    shader_features ^= SFEATURE_BLOOM;

    float light_direction[3] = {0.75, 0.5, 1};
    GPU_SetUniformfv(3, 3, 1, light_direction);
    float global_illumination[4] = {1, 1, 1, 0.60};
    GPU_SetUniformfv(4, 4, 1, global_illumination);
}

void Render_Engine::refresh_sprite_size()
{
    // std::cout << Textures[BLOCK_BLUE]->src.w;
    block_onscreen_size    = 8*BLOCK_TEXTURE_SIZE*window.scale;
    block_onscreen_half    = 8*BLOCK_TEXTURE_SIZE*window.scale/2.0;
    block_onscreen_quarter = 8*BLOCK_TEXTURE_SIZE*window.scale/4.0;
}

void Render_Engine::render_grid()
{
    // SDL_SetRenderDrawColor(renderer, 255, 255, 0, 127);

    SDL_Color linecolor = {255, 255, 0, 255};

    pixel_coord A = {i32round(target.x - window.size.x*0.50),
                     i32round(target.y + window.size.x*0.25)};

    pixel_coord B = {i32round(A.x + window.size.x*2),
                     i32round(A.y - window.size.x)};

    while(B.x < window.size.x)
    {
        B.x += window.size.x*2;
        B.y -= window.size.x;
    }

    while(A.y >= 0)
    {
        B.y = i32round(B.y-block_onscreen_half);
        A.y = i32round(A.y-block_onscreen_half);
    }

    while(A.x >= 0)
    {
        B.x = i32round(B.x-block_onscreen_size);
        A.x = i32round(A.x-block_onscreen_size);
    }

    while(B.y < window.size.y)
    {
        // SDL_RenderDrawLine(renderer, A.x, A.y, B.x, B.y);
        GPU_Line(screen, A.x, A.y, B.x, B.y, linecolor);

        B.y = i32round(B.y+block_onscreen_half);
        A.y = i32round(A.y+block_onscreen_half);
    }

    pixel_coord C = {i32round(target.x - window.size.x*0.50),
                     i32round(target.y - window.size.x*0.25)};
    
    pixel_coord D = {i32round(C.x + window.size.x*2),
                     i32round(C.y + window.size.x)};

    while(D.x <= window.size.x)
    {
        D.x += window.size.x*2;
        D.y += window.size.x;
    }

    while(D.y >= 0)
    {
        C.y = i32round(C.y-block_onscreen_half);
        D.y = i32round(D.y-block_onscreen_half);
    }

    while(C.x >= 0)
    {
        C.x = i32round(C.x-block_onscreen_size);
        D.x = i32round(D.x-block_onscreen_size);
    }


    while(C.y <= window.size.y)
    {
        GPU_Line(screen, C.x, C.y, D.x, D.y, linecolor);

        D.y = i32round(D.y+block_onscreen_half);
        C.y = i32round(C.y+block_onscreen_half);
    }
}

void Render_Engine::set_block_renderflags(char face, int i, int j)
{
    // r = render flag left
    // g = render flag right
    // b = render flag top
    // a = part culling
    screen_block *sb = projection_grid.get_pos(face, i , j);

    if(!sb)
    {
        std::cout
        << "Can't find matched coord for set_block_renderflags at "
        << (int)face << ' ' << i << ' ' << j << '\n';
        return;
    }

    block_coordonate bc = world.convert_wcoord(sb->x, sb->y, sb->height);
    chunk_coordonate coord = bc.chunk;
    int x = bc.x;
    int y = bc.y;
    int z = bc.z;

    if(sb->transparent_block.id)
    {
        block_coordonate bc = world.convert_wcoord(sb->x_transparent, sb->y_transparent, sb->height_transparent);

        sb->render_flags_transparent.r = world.get_block_id(bc.chunk, bc.x, bc.y+1, bc.z) ? 0 : 128;
        sb->render_flags_transparent.g = world.get_block_id(bc.chunk, bc.x+1, bc.y, bc.z) ? 0 : 128;
        sb->render_flags_transparent.b = world.get_block_id(bc.chunk, bc.x, bc.y, bc.z+1) ? 0 : 128;
        sb->render_flags_transparent.a = 0;
        // Uint8 diff;
        // if(sb->height)
        // {
        //     diff = (sb->height_transparent-sb->height);
        // }
        // else 
        // {
        //     int shift = sb->x_transparent < sb->y_transparent ? sb->x_transparent : sb->y_transparent;
        //     shift = shift < sb->height_transparent ? shift : sb->height_transparent;

        //     diff = shift;
        // }

        // diff = diff > 31 ? 31 : diff;
        // sb->render_flags_transparent.a &= (128+64+32);
        // sb->render_flags_transparent.a |= diff;

        // screen_block *sb_right = projection_grid.get_pos_world(sb->x_transparent, sb->y_transparent-1, sb->height_transparent);
        // if(sb_right)
        // {
        //     diff = (sb->height_transparent-sb_right->height);
        //     diff = diff > 31 ? 31 : diff;
        //     sb->render_flags_transparent.r &= 128;
        //     sb->render_flags_transparent.r |= diff;
        // }

        // screen_block *sb_left = projection_grid.get_pos_world(sb->x_transparent-1, sb->y_transparent, sb->height_transparent);
        // if(sb_left)
        // {
        //     diff = (sb->height_transparent-sb_left->height);
        //     diff = diff > 31 ? 31 : diff;
        //     sb->render_flags_transparent.g &= 128;
        //     sb->render_flags_transparent.g |= diff;
        // }

        // screen_block *sb_down = projection_grid.get_pos_world(sb->x_transparent, sb->y_transparent, sb->height_transparent-1);
        // if(sb_down)
        // {
        //     diff = (sb->height_transparent-sb_down->height);
        //     diff = diff > 31 ? 31 : diff;
        //     sb->render_flags_transparent.b &= 128;
        //     sb->render_flags_transparent.b |= diff;
        // }

        // for borders
        // sb->render_flags_transparent.r += world.get_block_id(coord, tx-1, ty, tz) ? 2 : 0;
        // sb->render_flags_transparent.r += world.get_block_id(coord, tx, ty-1, tz) ? 4 : 0;
        // sb->render_flags_transparent.r += world.get_block_id(coord, tx, ty, tz-1) ? 32 : 0;
    }

    if(!sb->opaque_block.id)
    {
        // std::cout
        // << "Can't find block for set_block_renderflags at "
        // << (int)face << ' ' << i << ' ' << j << '\n';
        return;
    }

    SDL_Color &render_flag = sb->render_flags;
    render_flag.r = 0;
    render_flag.g = 0;
    render_flag.b = 0;

    // for borders
    render_flag.r += world.get_opaque_block_id(coord, x-1, y, z) ? 2 : 0;
    render_flag.r += world.get_opaque_block_id(coord, x, y-1, z) ? 4 : 0;
    render_flag.r += world.get_opaque_block_id(coord, x, y, z-1) ? 32 : 0;

    int left  = world.get_opaque_block_id(coord, x, y+1, z); 
    int right = world.get_opaque_block_id(coord, x+1, y, z);
    int top   = world.get_opaque_block_id(coord, x, y, z+1);

    int id;

    // t : corner left & l : corner top left
    id = world.get_opaque_block_id(coord, x-1, y+1, z+1);
    render_flag.b += !(render_flag.b & 80) && id && id < BLOCK_LIGHT_LIMIT ? 2 : 0;

    // t : corner right & r : corner top right
    id = world.get_opaque_block_id(coord, x+1, y-1, z+1);
    render_flag.b += !(render_flag.b & 40) && id && id < BLOCK_LIGHT_LIMIT ? 1 : 0;

    // r : left && l : right
    id = world.get_opaque_block_id(coord, x+1, y+1, z);
    render_flag.g += id && id < BLOCK_LIGHT_LIMIT ? 64 : 0;

    // r : bottom left & l : bottom right
    id = world.get_opaque_block_id(coord, x+1, y+1, z-1);
    render_flag.g += !(render_flag.g & 72) && id && id < BLOCK_LIGHT_LIMIT ? 1 : 0;

    if(!left)
    {
        render_flag.r += 128;

        // AO LEFT
        id = world.get_opaque_block_id(coord, x-1, y+1, z);
        render_flag.r += id && id < BLOCK_LIGHT_LIMIT ? 64 : 0;

        //AO RIGHT
        // render_flag.r += world.get_opaque_block_id(coord, x+1, y+1, z) ? 32 : 0;

        //AO TOP
        id = world.get_opaque_block_id(coord, x, y+1, z+1);
        render_flag.r += id && id < BLOCK_LIGHT_LIMIT ? 16 : 0;

        //AO BOTTOM
        id = world.get_opaque_block_id(coord, x, y+1, z-1);
        render_flag.r += id && id < BLOCK_LIGHT_LIMIT ? 8 : 0;

        //AO CORNER TOP LEFT
        // render_flag.r += !(render_flag.r & 80) && world.get_opaque_block_id(coord, x-1, y+1, z+1) ? 4 : 0;

        //AO CORNER BOTTOM RIGHT
        // render_flag.r += !(render_flag.r & 40) && world.get_opaque_block_id(coord, x+1, y+1, z-1) ? 2 : 0;

        //AO CORNER BOTTOM LEFT
        id = world.get_opaque_block_id(coord, x-1, y+1, z-1);
        render_flag.r += !(render_flag.r & 72) && id && id < BLOCK_LIGHT_LIMIT ? 1 : 0;
    }

    if(!right)
    {
        render_flag.g += 128;

        // AO LEFT
        // render_flag.g += world.get_opaque_block_id(coord, x+1, y+1, z) ? 64 : 0;

        //AO RIGHT
        id = world.get_opaque_block_id(coord, x+1, y-1, z);
        render_flag.g += id && id < BLOCK_LIGHT_LIMIT ? 32 : 0;

        //AO TOP
        id = world.get_opaque_block_id(coord, x+1, y, z+1);
        render_flag.g += id && id < BLOCK_LIGHT_LIMIT ? 16 : 0;

        //AO BOTTOM
        id = world.get_opaque_block_id(coord, x+1, y, z-1);
        render_flag.g += id && id < BLOCK_LIGHT_LIMIT ? 8 : 0;

        //AO CORNER TOP RIGHT
        // render_flag.g += !(render_flag.g & 48) && world.get_opaque_block_id(coord, x+1, y-1, z+1) ? 4 : 0;

        //AO CORNER BOTTOM RIGHT
        id = world.get_opaque_block_id(coord, x+1, y-1, z-1);
        render_flag.g += !(render_flag.g & 40) && id && id < BLOCK_LIGHT_LIMIT ? 2 : 0;

        //AO CORNER BOTTOM LEFT
        // render_flag.g += !(render_flag.g & 72) && world.get_opaque_block_id(coord, x+1, y+1, z-1) ? 1 : 0;
    }

    if(!top)
    {
        render_flag.b += 128;

        // AO TOP LEFT
        id = world.get_opaque_block_id(coord, x-1, y, z+1);
        render_flag.b += id && id < BLOCK_LIGHT_LIMIT ? 64 : 0;

        // AO TOP RIGHT 
        id = world.get_opaque_block_id(coord, x, y-1, z+1);
        render_flag.b += id && id < BLOCK_LIGHT_LIMIT ? 32 : 0;

        // AO BOTTOM LEFT 
        id = world.get_opaque_block_id(coord, x, y+1, z+1);
        render_flag.b += id && id < BLOCK_LIGHT_LIMIT ? 16 : 0;

        // AO BOTTOM RIGHT
        id = world.get_opaque_block_id(coord, x+1, y, z+1);
        render_flag.b += id && id < BLOCK_LIGHT_LIMIT ? 8 : 0;

        // AO CORNER TOP
        id = world.get_opaque_block_id(coord, x-1, y-1, z+1);
        render_flag.b += !(render_flag.b & 96) && id && id < BLOCK_LIGHT_LIMIT ? 4 : 0;

        // AO CORNER LEFT 
        // render_flag.b += !(render_flag.b & 80) && world.get_opaque_block_id(coord, x-1, y+1, z+1) ? 2 : 0;

        // AO CORNER RIGHT
        // render_flag.b += !(render_flag.b & 40) && world.get_opaque_block_id(coord, x+1, y-1, z+1) ? 1 : 0;
    }
}

void Render_Engine::send_primitives_buffer(GPU_Target *screen, GPU_Image *img, Uint32* primbuf, Uint32 &sprite_cnt)
{
    // if(screen == transparent_screen)
    // {
    //     while (test){std::cout << "wait!\n";}
    // }

    int maxval = 0xffff;
    int nb_batch = sprite_cnt/(maxval/3);
    Uint32 *primitive_ptr = primbuf;

    for(int i = 0; i < nb_batch; i++)
    {
        GPU_PrimitiveBatchV(img, screen, 
                        GPU_POINTS, 
                        maxval/3, 
                        primitive_ptr, 
                        0, 
                        NULL, 
                        GPU_BATCH_XYZ);
        
        primitive_ptr += maxval;
    }

    GPU_PrimitiveBatchV(img, screen, 
                    GPU_POINTS, 
                    sprite_cnt%(maxval/3), 
                    primitive_ptr, 
                    0, 
                    NULL, 
                    GPU_BATCH_XYZ);
}

Uint8 Render_Engine::render_block(const chunk_coordonate &pgcoord)
{
    screen_block *sb = &projection_grid.pos[pgcoord.x][pgcoord.y][pgcoord.z];
    // sprite_counter++;
    // return sb->identical_line_counter;
    block b = sb->opaque_block;

    // if(!sb->is_on_screen) return 0;

    // if(sb->is_on_screen && b.id)
    if(b.id)
    {
        sb->render_flags.a &= ~(0b11);
        sb->render_flags.a += pgcoord.x;

        int idb = world_sprite_counter*3;
        primitives_buffer_world[idb] = *((Uint32*)&sb->render_flags);
        primitives_buffer_world[idb+1] = sb->x + (sb->y<<16);
        // primitives_buffer[idb+1] = *((Uint32*)&sb->x);
        primitives_buffer_world[idb+2] = sb->height + (b.id<<16) + (pgcoord.x<<24) + (sb->identical_line_counter<<26);

        world_sprite_counter++;
        // return sb->identical_line_counter;
    }
    if(sb->identical_line_counter < 0 || sb->identical_line_counter > IDENDICAL_LINE_MAX) 
        std::cout << sb->identical_line_counter << " !!!!!\n";
    return sb->identical_line_counter;
}

void Render_Engine::render_world(bool use_visible_frags)
{    
    if(projection_grid.pos[0] == NULL) return;

    world_sprite_counter = 0;
    int face;

    if(!use_visible_frags)
    {
        SDL_LockMutex(campg_mut);
        face = 0;
        for(int j  = projection_grid.size[face][1]-1; 
                j >= 0; j--)

        for(int i = 0;
                i < projection_grid.size[face][0]; i++)

                i += render_block({face, i, j});

        face = 1;
        for(int j  = projection_grid.size[face][1]-1; 
                j >= 0; j--)

            for(int i = 0;
                    i < projection_grid.size[face][0]; i++)

                i += render_block({face, i, j});

        face = 2;
        for(int i = 0;
                i < projection_grid.size[face][0]; i++)

            for(int j = 0;
                    j < projection_grid.size[face][1]; j++)

                j += render_block({face, i, j});
        
        SDL_UnlockMutex(campg_mut);
        render_transparent_world(use_visible_frags);
    }
    else
    {
        SDL_LockMutex(campg_mut);

        face = 0;
        for(int j  = projection_grid.visible_frags[face][1].end; 
                j >= projection_grid.visible_frags[face][1].beg; j--)

        for(int i = projection_grid.visible_frags[face][0].beg;
                i < projection_grid.visible_frags[face][0].end; i++)

                i += render_block({face, i, j});

        face = 1;
        for(int j  = projection_grid.visible_frags[face][1].end; 
                j >= projection_grid.visible_frags[face][1].beg; j--)

            for(int i = projection_grid.visible_frags[face][0].beg;
                    i < projection_grid.visible_frags[face][0].end; i++)

                i += render_block({face, i, j});

        face = 2;
        for(int i = projection_grid.visible_frags[face][0].beg;
                i < projection_grid.visible_frags[face][0].end; i++)

            for(int j = projection_grid.visible_frags[face][1].beg;
                    j < projection_grid.visible_frags[face][1].end; j++)

                j += render_block({face, i, j});

        SDL_UnlockMutex(campg_mut);
    }
    
}

Uint8 Render_Engine::render_transparent_block(const chunk_coordonate &pgcoord)
{
    screen_block *sb = &projection_grid.pos[pgcoord.x][pgcoord.y][pgcoord.z];
    block b = sb->transparent_block;

    // if(b.id && sb->is_on_screen)
    if(b.id)
    {
        // sb->render_flags_transparent.a &= ~(0b11);
        // sb->render_flags_transparent.a += pgcoord.x;
        sb->render_flags_transparent.b &= ~(32);
        sb->render_flags_transparent.g &= ~(32);
        sb->render_flags_transparent.b += pgcoord.x%2 ? 32 : 0;
        sb->render_flags_transparent.g += pgcoord.x&2 ? 32 : 0;

        int idb = transparent_sprite_counter*3;
        primitives_buffer_transparent[idb] = *((Uint32*)&sb->render_flags_transparent);
        primitives_buffer_transparent[idb+1] = sb->x_transparent + (sb->y_transparent<<16);
        primitives_buffer_transparent[idb+2] = sb->height_transparent + (b.id<<16) + (pgcoord.x<<24) + (sb->identical_line_counter_transparent<<26);

        transparent_sprite_counter++;
    }
    if(sb->identical_line_counter_transparent < 0 || sb->identical_line_counter_transparent > IDENDICAL_LINE_MAX) 
        std::cout << sb->identical_line_counter_transparent << " transparent !!!!! " << pgcoord << "\n";

    return sb->identical_line_counter_transparent;
}

void Render_Engine::render_transparent_world(bool use_visible_frags)
{
    test = true;
    transparent_sprite_counter = 0;
    int face;

    // if(1)
    if(!use_visible_frags)
    {
        SDL_LockMutex(campg_mut);

        face = 0;
        for(int j  = projection_grid.size[face][1]; 
                j >= 0; j--)

        for(int i = 0;
                i < projection_grid.size[face][0]; i++)

                i += render_transparent_block({face, i, j});

        face = 1;
        for(int j  = projection_grid.size[face][1]; 
                j >= 0; j--)

            for(int i = 0;
                    i < projection_grid.size[face][0]; i++)

                i += render_transparent_block({face, i, j});

        face = 2;
        for(int i = 0;
                i < projection_grid.size[face][0]; i++)

            for(int j = 0;
                    j < projection_grid.size[face][1]; j++)

                j += render_transparent_block({face, i, j});
        
        SDL_UnlockMutex(campg_mut);
    }
    else
    {
        SDL_LockMutex(campg_mut);

        face = 0;
        for(int j  = projection_grid.visible_frags[face][1].end; 
                j >= projection_grid.visible_frags[face][1].beg; j--)

        for(int i = projection_grid.visible_frags[face][0].beg;
                i < projection_grid.visible_frags[face][0].end; i++)

                i += render_transparent_block({face, i, j});

        face = 1;
        for(int j  = projection_grid.visible_frags[face][1].end; 
                j >= projection_grid.visible_frags[face][1].beg; j--)

            for(int i = projection_grid.visible_frags[face][0].beg;
                    i < projection_grid.visible_frags[face][0].end; i++)

                i += render_transparent_block({face, i, j});
                
        face = 2;
        for(int i = projection_grid.visible_frags[face][0].beg;
                i < projection_grid.visible_frags[face][0].end; i++)

            for(int j = projection_grid.visible_frags[face][1].beg;
                    j < projection_grid.visible_frags[face][1].end; j++)
                
                j += render_transparent_block({face, i, j});

        SDL_UnlockMutex(campg_mut);
    }

    test = false;

    
}

void Render_Engine::render_all_highlighted_blocks()
{
    sprite_counter = 0;
    GPU_SetUniformi(8, 8*BLOCK_TEXTURE_SIZE);

    if(highlight_type == HIGHLIGHT_BLOCKS || highlight_type == HIGHLIGHT_PIPETTE || highlight_wcoord2.x == HIGHLIGHT_NOCOORD)
    {
        render_hl_block(highlight_wcoord);
    }
    else 
    {
        int xbeg = highlight_wcoord.x;
        int xend = highlight_wcoord2.x;

        int ybeg = highlight_wcoord.y;
        int yend = highlight_wcoord2.y;

        int zbeg = highlight_wcoord.z;
        int zend = highlight_wcoord2.z;

        if(highlight_wcoord.x > highlight_wcoord2.x)
        {
            xbeg = highlight_wcoord2.x;
            xend = highlight_wcoord.x;
        }

        if(highlight_wcoord.y > highlight_wcoord2.y)
        {
            ybeg = highlight_wcoord2.y;
            yend = highlight_wcoord.y;
        }

        if(highlight_wcoord.z > highlight_wcoord2.z)
        {
            zbeg = highlight_wcoord2.z;
            zend = highlight_wcoord.z;
        }

        if(1)
        {
            int valsyz[] = {yend, zend,
                            ybeg, zend,
                            ybeg, zbeg,
                            yend, zbeg};

            int valsxy[] = {xend, yend,
                            xbeg, yend,
                            xbeg, ybeg,
                            xend, ybeg};

            int valsxz[] = {xend, zend,
                            xbeg, zend,
                            xbeg, zbeg,
                            xend, zbeg};

            int x, y, z;

            bool xnull = xend == xbeg;
            bool ynull = yend == ybeg;  
            bool znull = zend == zbeg;

            for(int i = 0; i < 8; i+=2)
            {
                // 0 2 4 6

                if( (!(i%4) || (!ynull && !znull)) && (i || !ynull || !znull) )
                {
                    y = valsyz[i];
                    z = valsyz[i+1];
                    for(x = xbeg+1; x < xend; x ++)
                    {
                        int xspos = target.x + block_onscreen_half*(x - y - 1);
                        if(xspos < -block_onscreen_half || xspos > screen->w+block_onscreen_half)
                            continue;
                        
                        int yspos = target.y + block_onscreen_quarter*(x + y - 2*z);
                        if(yspos < -block_onscreen_half || yspos > screen->h+block_onscreen_half)
                            continue;
                        
                        render_hl_block({x, y, z}, false, true, false);
                    }
                }

                if( (!(i%4) || (!xnull && !ynull)) && (i || !xnull || !ynull) )
                {
                    x = valsxy[i];
                    y = valsxy[i+1];

                    for(z = zbeg+1; z < zend; z ++)
                    {
                        int xspos = target.x + block_onscreen_half*(x - y - 1);
                        if(xspos < -block_onscreen_half || xspos > screen->w+block_onscreen_half)
                            continue;
                        
                        int yspos = target.y + block_onscreen_quarter*(x + y - 2*z);
                        if(yspos < -block_onscreen_half || yspos > screen->h+block_onscreen_half)
                            continue;
                        
                        render_hl_block({x, y, z}, false, false, true);
                    }
                }

                if( (!(i%4) || (!xnull && !znull)) && (i || !xnull || !znull) )
                {
                    x = valsxz[i];
                    z = valsxz[i+1];

                    for(y = ybeg+1; y < yend; y ++)
                    {
                        int xspos = target.x + block_onscreen_half*(x - y - 1);
                        if(xspos < -block_onscreen_half || xspos > screen->w+block_onscreen_half)
                            continue;
                        
                        int yspos = target.y + block_onscreen_quarter*(x + y - 2*z);
                        if(yspos < -block_onscreen_half || yspos > screen->h+block_onscreen_half)
                            continue;
                        
                        render_hl_block({x, y, z}, true, false, false);
                    }
                }
            }

            if(xnull && ynull && znull)
            {
                render_hl_block({xbeg, ybeg, zbeg});
            }

            if(!xnull && ynull && znull)
            {
                render_hl_block({xbeg, ybeg, zbeg}, false, true, false);
                render_hl_block({xend, ybeg, zbeg}, false, false, false);
            }
            
            else
            if(xnull && !ynull && znull)
            {
                render_hl_block({xbeg, ybeg, zbeg}, true, false, false);
                render_hl_block({xbeg, yend, zbeg}, false, false, false);
            }
            
            else
            if(xnull && ynull && !znull)
            {
                render_hl_block({xbeg, ybeg, zbeg}, false, false, true);
                render_hl_block({xbeg, ybeg, zend}, false, false, false);
            }            

            else
            if(!xnull && !ynull && znull)
            {
                render_hl_block({xbeg, ybeg, zbeg}, true, true, false);
                render_hl_block({xend, ybeg, zbeg}, true, false, false);
                render_hl_block({xbeg, yend, zbeg}, false, true, false);
                render_hl_block({xend, yend, zbeg}, false, false, false);
            }
            
            else
            if(xnull && !ynull && !znull)
            {
                render_hl_block({xbeg, ybeg, zbeg}, true, false, true);
                render_hl_block({xbeg, yend, zbeg}, false, false, true);
                render_hl_block({xbeg, ybeg, zend}, true, false, false);
                render_hl_block({xbeg, yend, zend}, false, false, false);
            }
            
            else
            if(!xnull && ynull && !znull)
            {
                render_hl_block({xbeg, ybeg, zbeg}, false, true, true);
                render_hl_block({xend, ybeg, zbeg}, false, false, true);
                render_hl_block({xbeg, ybeg, zend}, false, true, false);
                render_hl_block({xend, ybeg, zend}, false, false, false);
            }
            
            else
            if(!xnull && !ynull && !znull)
            {
                render_hl_block({xbeg, yend, zbeg}, false, true, false);
                render_hl_block({xbeg, ybeg, zend}, true, true, false);
                render_hl_block({xbeg, yend, zend}, false, true, false);

                render_hl_block({xend, ybeg, zbeg}, true, false, true);
                render_hl_block({xend, yend, zbeg}, false, false, true);
                render_hl_block({xend, ybeg, zend}, true, false, false);
                render_hl_block({xend, yend, zend}, false, false, false);
            }
        }
        
    }

    GPU_SetUniformi(10, 0); //depth mode
    send_primitives_buffer(hl_screen, Textures[BLOCK_HIGHLIGHT]->ptr, primitives_buffer, sprite_counter);
    
    GPU_SetUniformi(10, 1); //depth mode
    send_primitives_buffer(hld_screen, Textures[BLOCK_HIGHLIGHT]->ptr, primitives_buffer, sprite_counter);

    GPU_SetUniformi(10, 0); //depth mode
    GPU_SetUniformi(8, MOSAIC_TEXTURE_SIZE);
}

void Render_Engine::render_hl_block(coord3D pos, 
                                    bool hide_left, 
                                    bool hide_right,
                                    bool hide_top)
{
    screen_block *sb = projection_grid.get_pos_world(pos.x, pos.y, pos.z);

    if(!sb) return;

    int id = world.get_block_id_wcoord(pos.x+1, pos.y+1, pos.z);

    if(id && (pos.x == world.max_block_coord.x-1 || pos.y == world.max_block_coord.y-1))
        id = BLOCK_EMPTY;

    SDL_Color rf = {128, 128, 128, 0};

    if(sb->height == pos.z && sb->height)
    {
        rf = sb->render_flags;

        if(id && id < BLOCK_TRANSPARENT_LIMIT)
            rf.a |= 1;
            
        else
            rf.a &= ~1;
    }
    else //if(pos.z < max_height_render+1)
    {
        int t = BLOCK_EMPTY;
        int l = BLOCK_EMPTY;
        int r = BLOCK_EMPTY;

        if(pos.z < max_height_render)
            t = world.get_block_id_wcoord(pos.x, pos.y, pos.z+1);
        
        if(pos.z < max_height_render)
        {
            l = world.get_block_id_wcoord(pos.x, pos.y+1, pos.z);
            r = world.get_block_id_wcoord(pos.x+1, pos.y, pos.z);
        }

        if(id && id < BLOCK_TRANSPARENT_LIMIT)
            rf.a += 1;

        if(t && t < BLOCK_TRANSPARENT_LIMIT)
            rf.b = 0;

        if(l && l < BLOCK_TRANSPARENT_LIMIT)
            rf.r = 0;

        if(r && r < BLOCK_TRANSPARENT_LIMIT)
            rf.g = 0;
    }

    // if(pos.z >= max_height_render || pos.z == 0)
    if(pos.z >= max_height_render)
    {
        rf = {128, 128, 128, 0};
    }
    if(pos.z == max_height_render-1)
    {
        rf.b = 128;
    }

    if(hide_left)
        rf.r = 0;
    
    if(hide_right)
        rf.g = 0;
    
    if(hide_top)
        rf.b = 0;


    // if(sb->height_transparent >= pos.z)
    //     rf.b |= 1;

    int idb = sprite_counter*3;
    primitives_buffer[idb] = *((Uint32*)&rf);
    primitives_buffer[idb+1] = pos.x + (pos.y<<16);
    primitives_buffer[idb+2] = pos.z + (1 << 16);

    sprite_counter++;
}

void Render_Engine::render_lights()
{
    if(projection_grid.pos[0] == NULL) return;

    sprite_counter = 0;
    light_counter = 0;

    light_block *lb;
    Uint16 wid;


    for(int i = 0; i < MAX_LIGHT_NUMBER; i++)
    {
        lb = &world.lights.buff[i];

        if(lb->id)
        {
            wid = world.get_block_id_wcoord_nowvp(lb->pos);

            if(wid != lb->id)
            {
                world.lights.is_full = false;
                world.lights.time_since_last_full = 0;
                world.lights.trash_bin.push_front(*lb);
                block_coordonate coord;
                *lb = {0, coord};
                continue;
            }

            world_coordonate wc = lb->pos;
            
            world.invert_wvp(wc.x, wc.y);

            int idb = sprite_counter*3;
            primitives_buffer[idb] = 0;
            primitives_buffer[idb+1] = wc.x + (wc.y<<16);
            primitives_buffer[idb+2] = wc.z + (lb->id << 16);

            sprite_counter++;
            light_counter ++;
        }
    }

    send_primitives_buffer(light_screen, Textures[BLOCK_LIGHT]->ptr, primitives_buffer, sprite_counter);
}

void Render_Engine::refresh_block_visible(const chunk_coordonate& coord, const int x, const int y, const int z)
{
    int wx = x + coord.x*CHUNK_SIZE;
    int wy = y + coord.y*CHUNK_SIZE;
    int wz = z + coord.z*CHUNK_SIZE;

    int shiftx = world.max_block_coord.x-wx-1;
    int shifty = world.max_block_coord.y-wy-1;
    int shiftz = max_height_render-wz-1;

    int shiftmin = shiftx < shifty ? shiftx : shifty;
    shiftmin = shiftmin < shiftz ? shiftmin : shiftz;

    wx += shiftmin;
    wy += shiftmin;
    wz += shiftmin;

    refresh_line_visible2(wx, wy, wz);
}

void Render_Engine::refresh_pg_block_visible()
{
    screen_block *sb;

    int sw = screen->w; // + block_onscreen_size*MAX_LIGHT_NUMBER;
    int sh = screen->h;

    int Px_min = floor(-2*target.x/block_onscreen_size -1) ;
    int Py_min = floor(-4*target.y/block_onscreen_size -2) - block_onscreen_quarter*MAX_LIGHT_NUMBER*0.125;
    int Px_max = ceil(2*(sw-target.x)/block_onscreen_size + 1) + block_onscreen_size*MAX_LIGHT_NUMBER*0.125;
    int Py_max = ceil(4*(sh-target.y)/block_onscreen_size + 2);

    int coosumx  = 0;
    int coosumy = 0;

    for(int y = projection_grid.visible_frags[0][0].beg;
            y < projection_grid.visible_frags[0][0].end;
            y++)
        for(int z  = projection_grid.visible_frags[0][1].end; 
                z >= projection_grid.visible_frags[0][1].beg;
                z--)
        {
            sb = &projection_grid.pos[0][y][z];

            coosumx = -y;
            coosumy = y-2*z;

            if(coosumx < Px_min || coosumx > Px_max || coosumy < Py_min || coosumy > Py_max)
                sb->is_on_screen = false;
            else
                sb->is_on_screen = true;
        }


    for(int z  = projection_grid.visible_frags[1][1].end; 
            z >= projection_grid.visible_frags[1][1].beg;
            z--)
        for(int x = projection_grid.visible_frags[1][0].beg;
                x < projection_grid.visible_frags[1][0].end;
                x++)
        {
            sb = &projection_grid.pos[1][x][z];

            coosumx = x;
            coosumy = x-2*z;

            if(coosumx < Px_min || coosumx > Px_max || coosumy < Py_min || coosumy > Py_max)
                sb->is_on_screen = false;
            else
                sb->is_on_screen = true;
        }


    for(int x = projection_grid.visible_frags[2][0].beg;
            x < projection_grid.visible_frags[2][0].end;
            x++)
        for(int y = projection_grid.visible_frags[2][1].beg;
                y < projection_grid.visible_frags[2][1].end;
                y++)
        {
            sb = &projection_grid.pos[2][x][y];

            // int max = sb->identical_line_counter > sb->identical_line_counter_transparent ?
            //           sb->identical_line_counter : sb->identical_line_counter_transparent;
            // int ytmp = y + max;

            coosumx = x-y;
            coosumy = x+y;

            if(coosumx < Px_min || coosumx > Px_max || coosumy < Py_min || coosumy > Py_max)
                sb->is_on_screen = false;
            else
                sb->is_on_screen = true;
        }
}

void Render_Engine::refresh_block_render_flags(const chunk_coordonate& coord, const int x, const int y, const int z)
{
    for(char _x = -1; _x <= 1; _x++)
        for(char _y = -1; _y <= 1; _y++)
            for(char _z = -1; _z <= 1; _z++)
            {
                chunk_coordonate c = projection_grid.convert_wcoord(coord.x*CHUNK_SIZE+x+_x, 
                                                                    coord.y*CHUNK_SIZE+y+_y, 
                                                                    coord.z*CHUNK_SIZE+z+_z);

                set_block_renderflags(c.x, c.y, c.z);
            }

    coord3D wcoord2 = {x + coord.x*CHUNK_SIZE+1, y + coord.y*CHUNK_SIZE+1, z + coord.z*CHUNK_SIZE};
    coord3D wcoord3 = {x + coord.x*CHUNK_SIZE-1, y + coord.y*CHUNK_SIZE-1, z + coord.z*CHUNK_SIZE};
    refresh_line_shadows(wcoord2, wcoord3);

    // refresh_line_shadows(x + coord.x*CHUNK_SIZE, x + coord.x*CHUNK_SIZE, y + coord.y*CHUNK_SIZE, z + coord.z*CHUNK_SIZE);
}

bool Render_Engine::test_panorama_mode()
{
    // return window.scale < PANORAMA_SCALE_THRESHOLD;
    return !low_gpu_mode_enable && window.scale < PANORAMA_SCALE_THRESHOLD;
}

void Render_Engine::render_frame(bool render_map)
{
    GPU_ClearColor(DFIB_screen, {0, 0, 0, 0});
    GPU_ClearColor(Color_screen, {0, 0, 0, 0});
    GPU_ClearColor(transparent_screen, {0, 0, 0, 0});
    GPU_ClearColor(hl_screen, {0, 0, 0, 0});
    GPU_ClearColor(screen,  {105, 156, 203, 255});

    /****************** GENERATING BACKGROUND **************************/
    background_shader->activate();

    GPU_SetUniformf(1, timems);
    int win_const[4] = {screen->w, screen->h, target.x, target.y};
    GPU_SetUniformiv(5, 4, 1, win_const);
    GPU_SetUniformui(17, background_shader_data + (world.world_view_position<<30));

    // GPU_SetShaderImage(Textures[BACKGROUND_SUNSET]->ptr, 16, 8); // donne iChannel0

    GPU_BlitRect(Textures[BLOCK_AO]->ptr, NULL, Color_screen, NULL);

    background_shader->deactivate();
    /*******************************************************************/
    
    if(render_map)
    {    
        /****************** SETTING SHADER UNIFORMS ************************/
        DFIB_shader.activate();

        GPU_SetUniformf(1, timems/7500.0);
        GPU_SetUniformi(2, shader_features);
        // GPU_SetUniformfv(3, 3, 1, gi_direction);
        GPU_SetUniformfv(4, 4, 1, global_illumination);
        GPU_SetUniformiv(5, 4, 1, win_const);
        GPU_SetUniformf(6, block_onscreen_size);
        GPU_SetUniformi(7, BLOCK_TEXTURE_SIZE);
        GPU_SetUniformi(8, MOSAIC_TEXTURE_SIZE);
        GPU_SetUniformi(9, max_height_render);

        GPU_SetShaderImage(DFIB_FBO, DFIB_shader.get_location("DFIB"), 3);
        /******************************************************************/

        /****************** RENDERING THE WORLD ***************************/

        if(!test_panorama_mode() || force_nonpanorama_mode)
            render_world(true);
        // std::cout << sprite_counter << "\t";
        send_primitives_buffer(DFIB_screen, Textures[MOSAIC]->ptr, primitives_buffer_world, world_sprite_counter);
        

        GPU_ClearColor(light_screen, {0, 0, 0, 255});
        light_shader.activate();
        GPU_SetShaderImage(Textures[MOSAIC]->ptr, light_shader.get_location("block_atlas"), 1);
        GPU_SetUniformiv(5, 4, 1, win_const);
        GPU_SetUniformf(6, block_onscreen_size);
        GPU_SetUniformi(8, 1024);
        GPU_SetUniformi(7, 1024);
        GPU_SetUniformf(1, timems/1000.0);
        // GPU_SetShaderImage(Color_FBO, light_shader.get_location("world"), 3);
        render_lights();
        light_shader.deactivate();

        
        world_render_shader.activate();
        GPU_SetUniformi(2, shader_features);
        GPU_SetShaderImage(light_FBO, world_render_shader.get_location("light"), 7);
        GPU_SetShaderImage(Textures[MOSAIC]->ptr, world_render_shader.get_location("atlas"), 1);
        GPU_SetUniformfv(3, 3, 1, gi_direction);
        GPU_SetUniformfv(4, 4, 1, global_illumination);
        GPU_SetUniformf(6, block_onscreen_size);
        GPU_Blit(DFIB_FBO, NULL, Color_screen, 0, 0);
        world_render_shader.deactivate();

        GPU_ClearColor(hld_screen, {0, 0, 0, 0});

        transparent_shader.activate();
        
        GPU_SetShaderImage(Color_FBO, transparent_shader.get_location("world"), 8);
        GPU_SetShaderImage(Textures[BLOCK_NORMAL]->ptr, transparent_shader.get_location("normal"), 6);
        GPU_SetUniformf(1, timems/7500.0);
        GPU_SetUniformfv(3, 3, 1, gi_direction);
        GPU_SetUniformfv(4, 4, 1, global_illumination);
        GPU_SetUniformiv(5, 4, 1, win_const);
        GPU_SetUniformf(6, block_onscreen_size);
        GPU_SetUniformi(9, max_height_render);
        GPU_SetUniformi(10, 0); //depth mode

        if(*state == STATE_CONSTRUCTION)
        {
            if(highlight_mode || highlight_type == HIGHLIGHT_PIPETTE)
            highlight_block2();
            GPU_SetShaderImage(Textures[BLOCK_HIGHLIGHT]->ptr, world_render_shader.get_location("atlas"), 1);
            render_all_highlighted_blocks();
        }
        
        GPU_SetShaderImage(Textures[MOSAIC]->ptr, world_render_shader.get_location("atlas"), 1);
        GPU_SetShaderImage(hl_FBO, world_render_shader.get_location("hlworld"), 9); 
        GPU_SetShaderImage(hld_FBO, world_render_shader.get_location("hlworld_depth"), 10);
        GPU_SetUniformi(7, BLOCK_TEXTURE_SIZE);
        GPU_SetUniformi(8, MOSAIC_TEXTURE_SIZE);

        SDL_LockMutex(GameEvent->render_transpw_mut);
        send_primitives_buffer(transparent_screen, Textures[MOSAIC]->ptr, primitives_buffer_transparent, transparent_sprite_counter);
        SDL_UnlockMutex(GameEvent->render_transpw_mut);


        transparent_shader.deactivate();
        GPU_Blit(transparent_FBO, NULL, Color_screen, 0, 0);
        GPU_Blit(hl_FBO, NULL, Color_screen, 0, 0);
        // GPU_Blit(hld_FBO, NULL, Color_screen, 0, 0);
        /*******************************************************************/
    }
    
    
    /****************** POST PROCESS SHADER ****************************/
    post_process_shader.activate();
    GPU_SetUniformi(2, shader_features);
    GPU_SetUniformiv(5, 4, 1, win_const);
    GPU_SetUniformf(6, block_onscreen_size);
    GPU_Blit(Color_FBO, NULL, screen, 0, 0);
    GPU_Blit(transparent_FBO, NULL, screen, 0, 0); // à enlever
    post_process_shader.deactivate();
    /*******************************************************************/

    // GPU_Clear(screen);
    // GPU_Blit(DFIB_FBO, NULL, screen, 0, 0);

    // GPU_Flip(screen);

    
    // SDL_CondWait(GameEvent->secondary_frame_op_finish, GameEvent->init_cond);
}
