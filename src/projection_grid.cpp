#include <game.hpp>

Projection_grid::Projection_grid()
{
    // if(pos[0] || pos[1] || pos[2])
    // {
    //     std::cerr << "\nRENDER FATAL ERROR : Can't initialize non empty projection grid.\n";
    //     return;
    // }
    
    size[0][0] = 1 + CHUNK_SIZE*256;
    size[0][1] = 1 + CHUNK_SIZE*75;

    size[1][0] = 1 + CHUNK_SIZE*256;
    size[1][1] = 1 + CHUNK_SIZE*75;

    size[2][0] = 1 + CHUNK_SIZE*256;
    size[2][1] = 1 + CHUNK_SIZE*256;

    for(int face = 0; face < 3; face ++)
    {
        pos[face] = new screen_block*[size[face][0]];

        for(int i = 0; i < size[face][0]; i++)
        {
            pos[face][i] = new screen_block[size[face][1]];

            for(int j = 0; j < size[face][1]; j++)
            {
                pos[face][i][j].height = -1;
                pos[face][i][j].height_transparent = -1;
                pos[face][i][j].x = -1;
                pos[face][i][j].y = -1;
                pos[face][i][j].x_transparent = -1;
                pos[face][i][j].y_transparent = -1;
                pos[face][i][j].is_on_screen = true;
                pos[face][i][j].render_flags = {0, 0, 0, 0};
                pos[face][i][j].render_flags_transparent = {0, 0, 0, 0};
                pos[face][i][j].opaque_block.id = BLOCK_EMPTY;
                pos[face][i][j].transparent_block.id = BLOCK_EMPTY;
                pos[face][i][j].identical_line_counter = 0;
                pos[face][i][j].identical_line_counter_transparent = 0;
            }
        }
    }

    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 2; j++)
        {
            visible_frags[i][j].beg = 0;
            visible_frags[i][j].end = size[i][j];
        }
}

Projection_grid::~Projection_grid()
{
    // free_pos();

    size[0][0] = 1 + CHUNK_SIZE*256;
    size[0][1] = CHUNK_SIZE*75;

    size[1][0] = 1 + CHUNK_SIZE*256;
    size[1][1] = CHUNK_SIZE*75;

    size[2][0] = 1 + CHUNK_SIZE*256;
    size[2][1] = 1 + CHUNK_SIZE*256;

    for(int face = 0; face < 3; face ++)
    {
        if(pos[face])
        {
            for(int i = 0; i < size[face][0]; i++)
                if(pos[face][i])
                    delete [] pos[face][i];

            delete [] pos[face];
        }
    }

    pos[0] = NULL;
    pos[1] = NULL;
    pos[2] = NULL;
}

void Projection_grid::free_pos()
{
    clear();

    return;

    for(int face = 0; face < 3; face ++)
    {
        if(pos[face])
        {
            for(int i = 0; i < size[face][0]; i++)
                if(pos[face][i])
                    delete [] pos[face][i];

            delete [] pos[face];
        }
    }

    pos[0] = NULL;
    pos[1] = NULL;
    pos[2] = NULL;
}

void Projection_grid::init_pos(const int sizex, const int sizey, const int sizez)
{
    // if(pos[0] || pos[1] || pos[2])
    // {
    //     std::cerr << "\nRENDER FATAL ERROR : Can't initialize non empty projection grid.\n";
    //     return;
    // }
    
    size[0][0] = sizey+1;
    size[0][1] = sizez;

    size[1][0] = sizex+1;
    size[1][1] = sizez;

    size[2][0] = sizex+1;
    size[2][1] = sizey+1;

    // for(int face = 0; face < 3; face ++)
    // {
    //     pos[face] = new screen_block*[size[face][0]];

    //     for(int i = 0; i < size[face][0]; i++)
    //     {
    //         pos[face][i] = new screen_block[size[face][1]];

    //         for(int j = 0; j < size[face][1]; j++)
    //         {
    //             pos[face][i][j].height = -1;
    //             pos[face][i][j].height_transparent = -1;
    //             pos[face][i][j].x = -1;
    //             pos[face][i][j].y = -1;
    //             pos[face][i][j].x_transparent = -1;
    //             pos[face][i][j].y_transparent = -1;
    //             pos[face][i][j].is_on_screen = true;
    //             pos[face][i][j].render_flags = {0, 0, 0, 0};
    //             pos[face][i][j].render_flags_transparent = {0, 0, 0, 0};
    //             pos[face][i][j].opaque_block.id = BLOCK_EMPTY;
    //             pos[face][i][j].transparent_block.id = BLOCK_EMPTY;
    //             pos[face][i][j].identical_line_counter = 0;
    //             pos[face][i][j].identical_line_counter_transparent = 0;
    //         }
    //     }
    // }

    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 2; j++)
        {
            visible_frags[i][j].beg = 0;
            visible_frags[i][j].end = size[i][j];
        }
}

screen_block* Projection_grid::get_pos(Uint8 face, Uint32 i, Uint32 j)
{
    if(face > 2 || i >= (Uint32)size[face][0] || j >= (Uint32)size[face][1])
        return NULL;
    
    return &pos[face][i][j];
}

screen_block* Projection_grid::get_pos(chunk_coordonate coord, int x, int y, int z)
{
    return get_pos_world(coord.x*CHUNK_SIZE+x, coord.y*CHUNK_SIZE+y, coord.z*CHUNK_SIZE+z);
}

screen_block* Projection_grid::get_pos_world(int x, int y, int z)
{

    int shift = x < y ? x : y;
    shift = shift < z ? shift : z;

    x -= shift;
    y -= shift;
    z -= shift;
    
    if(x < 0 || y < 0 || z < 0 || x >= size[1][0] || y >= size[0][0] || z >= size[0][1])
        return NULL;
        
    if(x == 0)
    {
        return &pos[0][y][z];
    }
    if(y == 0)
    {
        return &pos[1][x][z];
    }
    if(z == 0)
    {
        return &pos[2][x][y];
    }

    return NULL;
}

chunk_coordonate Projection_grid::convert_wcoord(int x, int y, int z)
{

    int shift = x < y ? x : y;
    shift = shift < z ? shift : z;

    x -= shift;
    y -= shift;
    z -= shift;
    
    if(x < 0 || y < 0 || z < 0 || x >= size[1][0] || y >= size[0][0] || z >= size[0][1])
        return (chunk_coordonate){-1, -1, -1};
    
    if(x == 0)
    {
        return (chunk_coordonate){0, y, z};
    }
    if(y == 0)
    {
        return (chunk_coordonate){1, x, z};
    }
    if(z == 0)
    {
        return (chunk_coordonate){2, x, y};
    }

    return {-1, -1, -1};
}

void set_in_interval(int& x, const int min, const int max)
{
    if(x < min)
        x = min;
    else if(x > max)
        x = max;
}

void Projection_grid::save_curr_interval()
{
    for(int i = 0; i < 3; i ++)
        for(int j = 0; j < 2; j++)
        {
            visible_frags_save[i][j].beg = visible_frags[i][j].beg;
            visible_frags_save[i][j].end = visible_frags[i][j].end;
        }
}

void Projection_grid::refresh_visible_frags(pixel_coord t, Uint16 Rx, Uint16 Ry, long double b)
{
    visible_frags[0][0].beg = floor((2.0*(t.x - Rx))/b -1);
    visible_frags[0][0].end = floor((2.0*t.x)/b + 1)+1;

    set_in_interval(visible_frags[0][0].end, 0, size[0][0]-1);

    visible_frags[0][1].beg = floor(2.0*(t.y - Ry)/b);
    visible_frags[0][1].end = floor((2.0*t.y)/b + 1 + visible_frags[0][0].end/2);

    visible_frags[1][0].beg = floor((-2.0*t.x)/b - 1);
    visible_frags[1][0].end = floor((2.0*(Rx-t.x))/b + 1)+1;

    set_in_interval(visible_frags[1][0].end, 0, size[1][0]-1);

    visible_frags[1][1].beg = floor(2.0*(t.y - Ry)/b);
    visible_frags[1][1].end = floor((2.0*t.y)/b + 1 + visible_frags[1][0].end/2)-1;

    const int x = 0;
    const int y = 1;

    visible_frags[2][x].beg = floor((-2*t.y-t.x)/b-1.5)+1;
    visible_frags[2][x].end = floor((Rx - t.x + 2*(Ry-t.y))/b + 1.5)+1;

    visible_frags[2][y].beg = floor((t.x - 2*t.y - Rx)/b - 1.5)+1;
    visible_frags[2][y].end = floor((t.x - 2*t.y + 2*Ry)/b + 1.5)+1;

    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 2; j++)
        {
            set_in_interval(visible_frags[i][j].end, 0, size[i][j]-1);
            set_in_interval(visible_frags[i][j].beg, 0, size[i][j]-1);
        }
    
}

void Projection_grid::refresh_all_identical_line()
{
    if(!pos[0]) return;

    // std::cout << "PG : refreshing all identical line...\n";

    int identical_line_counter = 0;
    int identical_line_counter_transparent = 0;

    SDL_Color *rf = {0};
    SDL_Color *rf2 = {0};

    Uint8 id = 0;
    Uint8 id2 = 0;
    
    int h;
    int h2;

    SDL_Color *trf = {0};
    SDL_Color *trf2 = {0};

    Uint8 tid = 0;
    Uint8 tid2 = 0;

    screen_block *sb;

    int th;
    int th2;

    // x y
    int face = 2;

    for(int i = 0; i < size[face][0]; i++)
    {
        identical_line_counter = 0;
        identical_line_counter_transparent = 0;

        rf2 = &pos[face][i][size[face][1]-2].render_flags;
        id2 = pos[face][i][size[face][1]-2].opaque_block.id;
        h2 = pos[face][i][size[face][1]-2].height;

        trf2 = &pos[face][i][size[face][1]-2].render_flags_transparent;
        tid2 = pos[face][i][size[face][1]-2].transparent_block.id;  
        th2 = pos[face][i][size[face][1]-2].height_transparent;

        for(int j = size[face][1]-3; j >= 0; j--)
        {
            sb = &pos[face][i][j];

            rf = &sb->render_flags;
            id = sb->opaque_block.id;
            h = sb->height;

            if(identical_line_counter < IDENDICAL_LINE_MAX && 
               rf->r == rf2->r &&
               rf->g == rf2->g && 
               rf->b == rf2->b && 
               rf->a == rf2->a &&
               id == id2 &&
               h == h2)
                identical_line_counter ++;
            else
                identical_line_counter = 0;

            rf2 = rf;
            id2 = id;
            h2 = h;

            trf = &sb->render_flags_transparent;
            tid = sb->transparent_block.id;
            th = sb->height_transparent;

            if(identical_line_counter_transparent < IDENDICAL_LINE_MAX && 
               trf->r == trf2->r &&
               trf->g == trf2->g && 
               trf->b == trf2->b && 
               trf->a == trf2->a &&
               tid == tid2 &&
               th == th2)
                identical_line_counter_transparent ++;
            else
                identical_line_counter_transparent = 0;

            sb->identical_line_counter_transparent = identical_line_counter_transparent;
            sb->identical_line_counter = identical_line_counter;

            trf2 = trf;
            tid2 = tid;
            th2 = th;
        }
    }


    // z x
    face = 1;

    for(int i = 0; i < size[face][1]; i++)
    {
        identical_line_counter = 0;
        identical_line_counter_transparent = 0;

        rf2 = &pos[face][size[face][0]-1][i].render_flags;
        id2 = pos[face][size[face][0]-1][i].opaque_block.id;
        h2 = pos[face][size[face][0]-1][i].height;

        trf2 = &pos[face][size[face][0]-1][i].render_flags_transparent;
        tid2 = pos[face][size[face][0]-1][i].transparent_block.id;  
        th2 = pos[face][size[face][0]-1][i].height_transparent;

        for(int j = size[face][0]-2; j >= 0; j--)
        {
            sb = &pos[face][j][i];

            rf = &sb->render_flags;
            id = sb->opaque_block.id;
            h = sb->height;

            if(identical_line_counter < IDENDICAL_LINE_MAX && 
               rf->r == rf2->r &&
               rf->g == rf2->g && 
               rf->b == rf2->b && 
               rf->a == rf2->a &&
               id == id2 &&
               h == h2)
                identical_line_counter ++;
            else
                identical_line_counter = 0;

            rf2 = rf;
            id2 = id;
            h2 = h;

            trf = &sb->render_flags_transparent;
            tid = sb->transparent_block.id;
            th = sb->height_transparent;

            if(identical_line_counter_transparent < IDENDICAL_LINE_MAX && 
               trf->r == trf2->r &&
               trf->g == trf2->g && 
               trf->b == trf2->b && 
               trf->a == trf2->a &&
               tid == tid2 &&
               th == th2)
                identical_line_counter_transparent ++;
            else
                identical_line_counter_transparent = 0;

            sb->identical_line_counter_transparent = identical_line_counter_transparent;
            sb->identical_line_counter = identical_line_counter;

            trf2 = trf;
            tid2 = tid;
            th2 = th;
        }
    }

    // y z
    face = 0;
    for(int i = 0; i < size[face][1]; i++)
    {
        identical_line_counter = 0;
        identical_line_counter_transparent = 0;

        rf2 = &pos[face][size[face][0]-1][i].render_flags;
        id2 = pos[face][size[face][0]-1][i].opaque_block.id;
        h2 = pos[face][size[face][0]-1][i].height;

        trf2 = &pos[face][size[face][0]-1][i].render_flags_transparent;
        tid2 = pos[face][size[face][0]-1][i].transparent_block.id;  
        th2 = pos[face][size[face][0]-1][i].height_transparent;

        for(int j = size[face][0]-2; j >= 0; j--)
        {
            sb = &pos[face][j][i];

            rf = &sb->render_flags;
            id = sb->opaque_block.id;
            h = sb->height;

            if(identical_line_counter < IDENDICAL_LINE_MAX && 
               rf->r == rf2->r &&
               rf->g == rf2->g && 
               rf->b == rf2->b && 
               rf->a == rf2->a &&
               id == id2 &&
               h == h2)
                identical_line_counter ++;
            else
                identical_line_counter = 0;

            rf2 = rf;
            id2 = id;
            h2 = h;

            trf = &sb->render_flags_transparent;
            tid = sb->transparent_block.id;
            th = sb->height_transparent;

            if(identical_line_counter_transparent < IDENDICAL_LINE_MAX && 
               trf->r == trf2->r &&
               trf->g == trf2->g && 
               trf->b == trf2->b && 
               trf->a == trf2->a &&
               tid == tid2 &&
               th == th2)
                identical_line_counter_transparent ++;
            else
                identical_line_counter_transparent = 0;

            sb->identical_line_counter_transparent = identical_line_counter_transparent;
            sb->identical_line_counter = identical_line_counter;

            trf2 = trf;
            tid2 = tid;
            th2 = th;
        }
    }
}

void Projection_grid::clear()
{
    if(!pos[0]) return;

    int face, i, j;

    screen_block *sb;

    for(face = 0; face < 3; face ++)
    
        for(i = 0;
            i < size[face][0];
            i++)
        
            for(j = 0;
                j < size[face][1];
                j++)
            {
                sb = &pos[face][i][j];

                sb->opaque_block.id = BLOCK_EMPTY;
                sb->transparent_block.id = BLOCK_EMPTY;

                sb->render_flags = {0, 0, 0, 0};
                sb->render_flags_transparent = {0, 0, 0, 0};

                sb->height = 0;

                sb->identical_line_counter = 0;
                sb->identical_line_counter_transparent = 0;
            }
                
}