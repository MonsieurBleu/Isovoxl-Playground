// #include <constants.hpp>
// #include <sprites.hpp>
#include <game.hpp>

Sprite::Sprite()
{
    _isinit = false;
}

bool Sprite::isinit(){ return _isinit;}

void Sprite::init_sprite(Multithreaded_Event_Handler *_GameEvent, 
                         World *_world, 
                         Render_Engine *_RE,
                         std::string& name)
{
    _isinit = true;

    world = _world;
    RE = _RE;
    GameEvent = _GameEvent;

    pos = {-1, -1, -1};

    std::string total_filename = "saves/.sprites/";
    total_filename.append(name);
    total_filename.append("/world.isosave");

    frames.load_from_file(total_filename.c_str());

    frames.world_view_position = 0;

    set_frame(0, 0);

    // for(int x = 0; x < SPRITES_WIDTH; x ++)
    // for(int y = 0; y < SPRITES_WIDTH; y ++)
    // for(int z = 0; z < SPRITES_HEIGHT; z ++)
    // {
    //     voxels[x][y][z].is_occluded = true;
    // }

    if(name == "player")
    {
        idle.init(-1);

        for(int i = 0 ; i < 3; i++)
        {
            idle.addframe({{0, 0}, 1000});
            idle.addframe({{1, 0}, 500});
        }

        idle.addframe({{0, 0}, 400});
        idle.addframe({{2, 0}, 200});
        idle.addframe({{0, 0}, 400});
        idle.addframe({{1, 0}, 500});
        

        running.init(-1);
        running.addframe({{0, 1}, 350});
        running.addframe({{1, 1}, 350});

        curr_anim = &idle;
    }

}

void Sprite::set_frame(int x, int y)
{
    int basex = x*SPRITES_WIDTH;
    int basey = y*SPRITES_WIDTH;
    int basez = 8;

    current_frame.x = x;
    current_frame.y = y;

    for(int x = 0; x < SPRITES_WIDTH; x ++)
    for(int y = 0; y < SPRITES_WIDTH; y ++)
    for(int z = 0; z < SPRITES_HEIGHT; z ++)
    {

        voxels[x][y][z].id = frames.get_block_id_wcoord(basex+x, basey+y, basez+z);
        // voxels[x][y][z].is_occluded = true;
    }
}

void Sprite::remove()
{
    // if(pos.x == -1) return;

    sprite_voxel *v = &voxels[0][0][0];

    // Deleting sprite instance frome the world & setup voxels next coords
    for(int x = 0; x < SPRITES_WIDTH;  x ++)
    for(int y = 0; y < SPRITES_WIDTH;  y ++)
    for(int z = 0; z < SPRITES_HEIGHT; z ++)
    {
        v = &voxels[x][y][z];

        if(!v->is_occluded)
        {
            block_coordonate bc = world->convert_wcoord(v->wcoord.x, 
                                                        v->wcoord.y, 
                                                        v->wcoord.z);
            
            block *b = &world->
                        chunks
                        [bc.chunk.x]
                        [bc.chunk.y]
                        [bc.chunk.z]
                        .blocks
                        [bc.x]
                        [bc.y]
                        [bc.z];

            b->id = BLOCK_EMPTY;
            // world->compress_chunk(bc.chunk.x, bc.chunk.y, bc.chunk.z);
        }
    }

    compress_chunks();
}

void Sprite::update()
{
    sprite_voxel *v = &voxels[0][0][0];

    // Building sprite in the world

    for(int x = 0; x < SPRITES_WIDTH;  x ++)
    for(int y = 0; y < SPRITES_WIDTH;  y ++)
    for(int z = 0; z < SPRITES_HEIGHT; z ++)
    {
        v = &voxels[x][y][z];

        // if(!v->id) continue;

        v->is_occluded = true;

        // int x2 = SPRITES_WIDTH-1-x;
        int x2 = x;
        int y2 = y;
        int z2 = z;

        switch (rotation)
        {
        case 1 :
            x2 = y;
            y2 = x;
            y2 = SPRITES_WIDTH-1-y2;
            break;

        case 2 :
            x2 = y;
            y2 = x;
            x2 = SPRITES_WIDTH-1-x2;
            break;

        case 3 :
            y2 = SPRITES_WIDTH-1-y;
            x2 = SPRITES_WIDTH-1-x;
        default:
            break;
        }


        v->wcoord = {pos.x - SPRITES_WIDTH/2 + x2, pos.y - SPRITES_WIDTH/2 + y2, pos.z + z2};

        block_coordonate bc = world->convert_wcoord(v->wcoord.x, 
                                                    v->wcoord.y, 
                                                    v->wcoord.z);
        
        block *b = &world->
                    chunks
                    [bc.chunk.x]
                    [bc.chunk.y]
                    [bc.chunk.z]
                    .blocks
                    [bc.x]
                    [bc.y]
                    [bc.z];

        if(!b->id)
        {
            v->is_occluded = false;
            b->id = v->id;

                if(v->id < BLOCK_TRANSPARENT_LIMIT && v->id > BLOCK_LIGHT_LIMIT)
                    world->lights.add({(Uint8)v->id, v->wcoord.to_block_coordonate()});


            // world->compress_chunk(bc.chunk.x, bc.chunk.y, bc.chunk.z);
        }
    }

    compress_chunks();

    refresh_display();
    RE->projection_grid.refresh_all_identical_line();
}

int Sprite::tp(fcoord3D fcoord)
{
    time_since_last_vel = timems;

    coord3D coord;
    subvoxel_pos = fcoord;

    coord.x = floor(fcoord.x);
    coord.y = floor(fcoord.y);
    coord.z = floor(fcoord.z);

    if(coord.x == pos.x && coord.y == pos.y && coord.z == pos.z)
        return SPRITE_TO_SMALL_MOVE;

    if(pos.x != -1)
    {
        remove();
        refresh_display();
    }

    set_in_interval(coord.x, SPRITES_WIDTH/2, world->max_block_coord.x-SPRITES_WIDTH/2);
    set_in_interval(coord.y, SPRITES_WIDTH/2, world->max_block_coord.y-SPRITES_WIDTH/2);
    set_in_interval(coord.z, 0, world->max_block_coord.z-SPRITES_HEIGHT);
    pos = coord;

    // update();

    return 1;
}

int Sprite::move(fcoord3D vel, bool sprite_rotation, 
                                bool clipping, 
                                bool wvp_movement, 
                                bool apply_movement,
                                bool apply_timed_velocity)
{
    bool obstacle_found = false;
    bool tpd = false;

    if(apply_timed_velocity)
    {
        Uint64 time = (timems-time_since_last_vel);
        vel.x *= time;
        vel.y *= time;
        vel.z *= time;
        time_since_last_vel = timems;
    }

    if(pos.x == -1)
    {
        return 0;
    }

    if(vel.x == 0.f && vel.y == 0.f && vel.z == 0.f)
    {
        curr_anim = &idle;
        // return;
    }
    else
    {
        curr_anim = &running;
    }

    if(wvp_movement)
        convert_wvp_vel(vel);

    if(sprite_rotation)
    {
        if(vel.x == 0.f && vel.y < 0.f) 
            rotation = 3;
        
        if(vel.x == 0.f && vel.y > 0.f)
            rotation = 0; 

        if(vel.x < 0 && vel.y == 0.f)
            rotation = 2;
        
        if(vel.x > 0 && vel.y == 0.f)
            rotation = 1;

        // DIAGONAL ORIENTATION 

        if(vel.x < 0.f && vel.y < 0.f)
            rotation = 2;
        
        if(vel.x < 0.f && vel.y > 0.f)
            rotation = 0; 

        if(vel.x > 0.f && vel.y < 0.f)
            rotation = 3;

        if(vel.x > 0.f && vel.y > 0.f)
            rotation = 1;
    }

    fcoord3D newpos;

    newpos.x = subvoxel_pos.x + vel.x;
    newpos.y = subvoxel_pos.y + vel.y;
    newpos.z = subvoxel_pos.z + vel.z;

    if(clipping)
    {
        int xbeg, xend;
        int ybeg, yend;
        int zbeg, zend;

        zbeg = pos.z;
        zend = pos.z+SPRITES_HEIGHT;

        if(vel.x > 0)
        {
            xbeg = pos.x + SPRITES_WIDTH/2;
            xend = xbeg + floor(newpos.x) - floor(subvoxel_pos.x) - 1;

            ybeg = pos.y-SPRITES_WIDTH/2 + 1 + 1 - 2;
            yend = pos.y+SPRITES_WIDTH/2 - 1;

            coord3D obstacle = check_zone(xbeg, xend, ybeg, yend, zbeg, zend);

            if(obstacle.x != -1)
            {
                obstacle_found = true;
                newpos.x = obstacle.x-SPRITES_WIDTH/2;
            }
        }
        else if(vel.x < 0)
        {
            xend = pos.x - SPRITES_WIDTH/2 - 1;
            xbeg = xend - abs(floor(subvoxel_pos.x)-floor(newpos.x)) + 1;

            ybeg = pos.y-SPRITES_WIDTH/2 + 1;
            yend = pos.y+SPRITES_WIDTH/2 - 1;

            coord3D obstacle = check_zone(xbeg, xend, ybeg, yend, zbeg, zend, true);

            if(obstacle.x != -1)
            {
                obstacle_found = true;
                newpos.x = obstacle.x + SPRITES_WIDTH/2 + 1;
            }
        }

        if(vel.y > 0)
        {
            xbeg = pos.x-SPRITES_WIDTH/2 + 1;
            xend = pos.x+SPRITES_WIDTH/2 - 1;

            ybeg = pos.y + SPRITES_WIDTH/2;
            yend = ybeg + floor(newpos.y) - floor(subvoxel_pos.y) - 1;

            coord3D obstacle = check_zone(xbeg, xend, ybeg, yend, zbeg, zend);

            if(obstacle.y != -1)
            {
                obstacle_found = true;
                newpos.y = obstacle.y-SPRITES_WIDTH/2;
            }
        }
        else if(vel.y < 0)
        {
            xbeg = pos.x-SPRITES_WIDTH/2 + 1 - 1;
            xend = pos.x+SPRITES_WIDTH/2 - 1;

            yend = pos.y - SPRITES_WIDTH/2 - 1;
            ybeg = yend - abs(floor(subvoxel_pos.y)-floor(newpos.y)) + 1;


            coord3D obstacle = check_zone(xbeg, xend, ybeg, yend, zbeg, zend, false, true);

            if(obstacle.y != -1)
            {
                obstacle_found = true;
                newpos.y = obstacle.y + SPRITES_WIDTH/2 + 1;
            }
                // newpos.y -= vel.y;

        }
    }

    if(apply_movement)
    {
        tpd = tp(newpos) > 0 ? true : false;
    }

    else
    {
        tpd = floor(newpos.x) != floor(pos.x) || floor(newpos.y) != floor(pos.y) || floor(newpos.z) != floor(pos.z);
    }


    if(!tpd && obstacle_found)
        return SPRITE_OBSTACLE_FOND;
    
    if(!tpd && !obstacle_found)
        return SPRITE_TO_SMALL_MOVE;
    
    return 1;
}

void Sprite::compress_chunks()
{
    block_coordonate wcoo = world->convert_wcoord(pos.x, pos.y, pos.z);

    int xbeg = wcoo.chunk.x >= 2 ? wcoo.chunk.x-2 : 0; 
    int ybeg = wcoo.chunk.y >= 2 ? wcoo.chunk.y-2 : 0; 
    int zbeg = wcoo.chunk.z;

    int xend = wcoo.chunk.x > world->max_chunk_coord.x-2 ? world->max_chunk_coord.x : wcoo.chunk.x+2;
    int yend = wcoo.chunk.y > world->max_chunk_coord.y-2 ? world->max_chunk_coord.y : wcoo.chunk.y+2;
    int zend = wcoo.chunk.z > world->max_chunk_coord.z-4 ? world->max_chunk_coord.z : wcoo.chunk.z+4;

    for(int x = xbeg; x <= xend; x++)
    for(int y = ybeg; y <= yend; y++)
    for(int z = zbeg; z <= zend; z++)
    {
        world->compress_chunk(x, y, z);
    }
}

coord3D Sprite::check_zone(int xbeg, 
                           int xend, 
                           int ybeg, 
                           int yend, 
                           int zbeg, 
                           int zend, 
                           bool reversex, 
                           bool reversey)
{
    int xsign = 1;
    int ysign = 1;

    if(reversex)
    {
        xsign = -1;
        int tmp = xbeg;
        xbeg = xend;
        xend = tmp;
    }

    if(reversey)
    {
        ysign = -1;
        int tmp = ybeg;
        ybeg = yend;
        yend = tmp;
    }

    for(int x = xbeg; xsign*x <= xsign*xend; x += xsign)
    for(int y = ybeg; ysign*y <= ysign*yend; y += ysign)
    for(int z = zbeg; z <= zend; z++)
    {
        block_coordonate bc = world->convert_wcoord(x, y, z);

        if(world->chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id != BLOCK_EMPTY)
        {
            return {x, y, z};
        }
    }

    return {-1, -1, -1};
}

void Sprite::refresh_display()
{
    world_coordonate wcoo = pos;
    
    world->invert_wvp(wcoo.x, wcoo.y);

    int xbeg = wcoo.x - SPRITES_WIDTH/2 - 1;
    int ybeg = wcoo.y - SPRITES_WIDTH/2 - 1;
    int zbeg = wcoo.z - 1;

    int xend = wcoo.x + SPRITES_WIDTH/2 + 1;
    int yend = wcoo.y + SPRITES_WIDTH/2 + 1;
    int zend = wcoo.z + SPRITES_HEIGHT + 1;

    int y = yend;
    int x, z;

    for(x = xbeg; x <= xend; x++)
    for(z = zbeg; z <= zend; z++)
    {
        int shiftx = world->max_block_coord.x-x-1;
        int shifty = world->max_block_coord.y-y-1;
        int shiftz = RE->max_height_render-z-1;

        int shiftmin = shiftx < shifty ? shiftx : shifty;
        shiftmin = shiftmin < shiftz ? shiftmin : shiftz;

        int wx, wy, wz;

        wx = x + shiftmin;
        wy = y + shiftmin;
        wz = z + shiftmin;

        RE->refresh_line_visible2(wx, wy, wz);

        coord3D pgc = RE->projection_grid.convert_wcoord(wx, wy, wz);

        RE->set_block_renderflags(pgc.x, pgc.y, pgc.z);
        RE->set_block_shadow_context2(pgc.x, pgc.y, pgc.z);
    }

    for(y = ybeg; y <= yend; y++)
    for(z = zbeg; z <= zend; z++)
    {
        int shiftx = world->max_block_coord.x-x-1;
        int shifty = world->max_block_coord.y-y-1;
        int shiftz = RE->max_height_render-z-1;

        int shiftmin = shiftx < shifty ? shiftx : shifty;
        shiftmin = shiftmin < shiftz ? shiftmin : shiftz;

        int wx, wy, wz;

        wx = x + shiftmin;
        wy = y + shiftmin;
        wz = z + shiftmin;

        RE->refresh_line_visible2(wx, wy, wz);

        coord3D pgc = RE->projection_grid.convert_wcoord(wx, wy, wz);

        RE->set_block_renderflags(pgc.x, pgc.y, pgc.z);
        RE->set_block_shadow_context2(pgc.x, pgc.y, pgc.z);
    }

    for(x = xbeg; x <= xend+1; x++)
    for(y = ybeg; y <= yend; y++)
    {
        int shiftx = world->max_block_coord.x-x-1;
        int shifty = world->max_block_coord.y-y-1;
        int shiftz = RE->max_height_render-z-1;

        int shiftmin = shiftx < shifty ? shiftx : shifty;
        shiftmin = shiftmin < shiftz ? shiftmin : shiftz;

        int wx, wy, wz;

        wx = x + shiftmin;
        wy = y + shiftmin;
        wz = z + shiftmin;

        RE->refresh_line_visible2(wx, wy, wz);

        coord3D pgc = RE->projection_grid.convert_wcoord(wx, wy, wz);

        RE->set_block_renderflags(pgc.x, pgc.y, pgc.z);
        RE->set_block_shadow_context2(pgc.x, pgc.y, pgc.z);
    }

    world_coordonate wcoo_shadow = pos;
    wcoo_shadow.x -= pos.z + SPRITES_HEIGHT + 1;

    world->invert_wvp(wcoo_shadow.x, wcoo_shadow.y);
    set_in_interval(wcoo_shadow.x, 0, world->max_block_coord.x);
    set_in_interval(wcoo_shadow.y, 0, world->max_block_coord.y);

    for(x = xend; x <= wcoo_shadow.x; x++)
    for(y = ybeg; y <= yend; y++)
    for(z = zbeg; z <= zend; z++)
    {
        int wx = x, wy = y, wz = z;
        coord3D pgc = RE->projection_grid.convert_wcoord(wx, wy, wz);
        RE->set_block_shadow_context2(pgc.x, pgc.y, pgc.z);
    }
}

bool Sprite::refresh_animation()
{
    if(pos.x == -1)
        return false;

    coord2D newf = curr_anim->get_curr_frame();

    // coord2D newf = idle.get_curr_frame();

    // coord2D newf = running.get_curr_frame();

    if(newf.x != current_frame.x || newf.y != current_frame.y)
    {
        // std::cout << "new animation frame pog ";
        // std::cout << pos.x << " " << pos.y << " " << pos.z << " ";
        // std::cout << newf.x << " " << newf.y << "\n";
        // print_debug_info();

        set_frame(newf.x, newf.y);
        // remove();
        return true;
    }

    return false;
}

void Sprite::convert_wvp_vel(fcoord3D &vel)
{
    // int wvp = world->world_view_position;

    float tmp;

    switch (world->world_view_position)
    {
    case 1 :
        tmp = vel.x;
        vel.x = vel.y;
        vel.y = tmp;

        vel.y *= -1.f;
        break;
    
    case 2 :

        vel.y *= -1.f;
        vel.x *= -1.f;

        break;

    case 3 : 
        tmp = vel.x;
        vel.x = vel.y;
        vel.y = tmp; 

        vel.x *= -1.f;
        break;

    default:
        break;
    }
}

void Sprite::rotate_left()
{
    switch (rotation)
    {
        case 0 : rotation = 1; break;

        case 1 : rotation = 3; break;

        case 2 : rotation = 0; break;

        case 3 : rotation = 2; break;
        
        default: break;
    }

    // 0 -> 1 -> 3 -> 2
}

void Sprite::rotate_right()
{
    switch (rotation)
    {
        case 0 : rotation = 2; break;

        case 1 : rotation = 0; break;

        case 2 : rotation = 3; break;
        
        case 3 : rotation = 1; break;

        default: break;
    }

    // 0 <- 1 <- 3 <- 2
}

void Sprite::print_debug_info()
{
    system("cls");
    std::cout << "=sprite debug infos=\n";
    std::cout << "pos : " << pos.x << " " << pos.y << " " << pos.z << "\n";
    std::cout << "svpos : " << subvoxel_pos.x << " " << subvoxel_pos.y << " " << subvoxel_pos.z << "\n";
    std::cout << "current frame : " << current_frame.x << " " << current_frame.y << "\n";
    std::cout << curr_anim << " " << &idle << " " << &running << "\n";
    std::cout << "rotation : " << rotation << "\n";
    std::cout << world << " " << RE << " " << GameEvent << "\n";


    bool all_empty = true;
    for(int x = 0; x < SPRITES_WIDTH;  x ++)
    {
        for(int y = 0; y < SPRITES_WIDTH;  y ++)
        {
            for(int z = 0; z < SPRITES_HEIGHT; z ++)
            {
                // std::cout << voxels[x][y][z].id << "\t";

                if(voxels[x][y][z].id)
                    all_empty = false;
            }

            // std::cout << "\n";
        }
    }

    if(all_empty)
        std::cout << "/!\\ CURRENT FRAME IS EMPTY /!\\ \n"; 
}