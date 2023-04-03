#include <game.hpp>

std::string format(unsigned long long i) {
  char buffer[128]; // can be adapted more tightly with std::numeric_limits

  char* p = buffer + 128;
  *(--p) = '\0';

  unsigned char count = 0;
  while (i != 0) {
    *(--p) = '0' + (i % 10);
    i /= 10;

    if (++count == 3) { count = 0; *(--p) = ' '; }
  }

  return p;
}

Multithreaded_Event_Handler::Multithreaded_Event_Handler(Render_Engine &_RE, UndoManager& _UM, Audio_Engine& _AE) : RE(_RE), UM(_UM), AE(_AE)
{
    PE = NULL;
    init_cond = SDL_CreateMutex();
    render_transpw_mut = SDL_CreateMutex();
    new_frame_to_render = SDL_CreateCond();
    secondary_frame_op_finish = SDL_CreateCond();

    nfs_mut = SDL_CreateMutex();
    new_nfs_event = SDL_CreateCond();

    world_and_projection_grid_mut = SDL_CreateMutex();

    game_is_running = true;
    RE.SecondaryThread = SDL_CreateThread(SecondaryThread_operations, "iso2", this);
    NFS_Thread = SDL_CreateThread(NFS_operations, "iso3", this);

    event_queue2 = new game_event[MAX_EVENT2];
    reader = -1;
}

Multithreaded_Event_Handler::~Multithreaded_Event_Handler()
{
    SDL_DestroyMutex(init_cond);
    SDL_DestroyMutex(nfs_mut);
    SDL_DestroyMutex(render_transpw_mut);

    SDL_DestroyCond(new_frame_to_render);
    SDL_DestroyCond(secondary_frame_op_finish);
    SDL_DestroyCond(new_nfs_event);

    delete[] event_queue2;
}

void Multithreaded_Event_Handler::add_event(game_event *new_event)
{
    // event_queue.push(new_event);
    if(reader < MAX_EVENT2)
    {
        reader++;
        event_queue2[reader] = *new_event;
    }
    else
    {
        std::cout << "MAX EVENT QUEUE SIZE REACHED!\n";
    }
}

void Multithreaded_Event_Handler::add_event(const int _id)
{
    // game_event* new_event = new game_event;
    game_event new_event;

    new_event.id = _id;

    add_event(&new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const float _scale)
{
    // game_event* new_event = new game_event;
    game_event new_event;

    new_event.id = _id;
    new_event.data.scale = _scale;

    add_event(&new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const pixel_coord _target)
{
    // game_event* new_event = new game_event;
    game_event new_event;

    new_event.id = _id;
    new_event.data.target = _target;

    add_event(&new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const coord3D coord)
{
    // game_event* new_event = new game_event;
    game_event new_event;

    new_event.id = _id;
    new_event.data.coord1.chunk = coord;

    add_event(&new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const block_coordonate coord)
{
    // game_event* new_event = new game_event;
    game_event new_event;

    new_event.id = _id;
    new_event.data.coord1 = coord;

    add_event(&new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const block_coordonate coord, Uint16 _blockid)
{
    // game_event* new_event = new game_event;
    game_event new_event;

    new_event.id = _id;
    new_event.data.coord1 = coord;
    new_event.data.blockid = _blockid;

    add_event(&new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const world_coordonate coord, Uint16 _blockid)
{
    // game_event* new_event = new game_event;
    game_event new_event;

    new_event.id = _id;
    new_event.data.wcoord1 = coord;
    new_event.data.blockid = _blockid;

    add_event(&new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, const block_coordonate _coord1, const block_coordonate _coord2)
{
    // game_event* new_event = new game_event;
    game_event new_event;

    new_event.id = _id;
    new_event.data.coord1 = _coord1;
    new_event.data.coord1 = _coord2;

    add_event(&new_event);
}

void Multithreaded_Event_Handler::add_event(const int _id, sprite_voxel* voxel)
{
    // game_event* new_event = new game_event;

    game_event new_event;

    new_event.id = _id;
    new_event.data.svoxel = voxel;

    add_event(&new_event);
}

void Multithreaded_Event_Handler::handle()
{
    SDL_LockMutex(init_cond);

    SecondaryThread_opcode = 0;

    bool refresh_identical_line = false;

    // while(!event_queue.empty())
    while(reader >= 0)
    {
        // game_event event = *event_queue.front();
        // delete event_queue.front();

        game_event event = event_queue2[reader];

        switch(event.id)
        {
        case GAME_EVENT_NEWSCALE :

            RE.window.scale = event.data.scale;

            SecondaryThread_opcode |= STHREAD_OP_PG_BLOCK_VISIBLE;

            RE.refresh_sprite_size();
            RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);

            break;

        case GAME_EVENT_ADDSCALE :
            
            if(event.data.scale > 0)
            {
                if(RE.window.scale < MAX_FLOAT_SCALE)
                {
                    RE.window.scale *= 2;
                    RE.target.x = RE.target.x*2 - (RE.screen->w*0.5);
                    RE.target.y = RE.target.y*2 - (RE.screen->h*0.5);
                }
            }
            else if(RE.window.scale > MIN_FLOAT_SCALE)
            {
                RE.window.scale /= 2;
                RE.target.x = (RE.target.x)/2 + (RE.screen->w*0.25);
                RE.target.y = (RE.target.y)/2 + (RE.screen->h*0.25);
            }

            SecondaryThread_opcode |= STHREAD_OP_PG_BLOCK_VISIBLE;

            RE.refresh_sprite_size();

            // if(RE.window.scale >= PANORAMA_SCALE_THRESHOLD)
            // {
                RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);
            // }
            // else 
            if(RE.window.scale > 0.25*PANORAMA_SCALE_THRESHOLD)
            {
                refresh_identical_line = true;
            }   

            // RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);
            break;
        
        case GAME_EVENT_CAMERA_MOUVEMENT :

            RE.target.x += event.data.target.x;
            RE.target.y += event.data.target.y;
            SecondaryThread_opcode |= STHREAD_OP_PG_BLOCK_VISIBLE;

            RE.refresh_sprite_size();
            RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);
            break;

        case GAME_EVENT_PLAYER_BLOCK_MOD :
        {
            PE->world_mutex.lock();

            // GERER LE MOMENT OU TU DELETE
            int maxz = 0;

            if(event.data.blockid != BLOCK_EMPTY)
            {

                maxz = RE.highlight_wcoord2.z > RE.highlight_wcoord.z ? RE.highlight_wcoord2.z : RE.highlight_wcoord.z;

                if(maxz >= RE.max_height_render)
                    RE.max_height_render = maxz+1;
                
                if(maxz/CHUNK_SIZE >= RE.world.highest_nonemptychunk)
                    RE.world.highest_nonemptychunk = maxz/CHUNK_SIZE;

            }

            if(RE.highlight_wcoord2.x < 0)
            {
                block_coordonate bc  = RE.world.convert_wcoord(event.data.wcoord1.x, event.data.wcoord1.y, event.data.wcoord1.z);
                block_coordonate bc2 = RE.world.convert_wcoord(event.data.wcoord1.x, event.data.wcoord1.y, event.data.wcoord1.z); // used for undo, is translated
                
                RE.world.translate_world_view_position(bc2.chunk, bc2.x, bc2.y, bc2.z);

                chunk c = RE.world.chunks[bc2.chunk.x][bc2.chunk.y][bc2.chunk.z];

                chunk_save modified_chunk;

                if (c.compress_value == CHUNK_NON_UNIFORM) {
                    modified_chunk.resize(1, 0);
                    chunk_data* cd = modified_chunk.get_chunk(0);
                    memcpy(cd->blocks, c.blocks, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
                    cd->coord = bc2.chunk;
                    cd->compress_value = c.compress_value;
                }
                else {
                    modified_chunk.resize(0, 1);
                    compressed_chunk_data* cd = modified_chunk.get_compressed(0);
                    cd->coord = bc2.chunk;
                    cd->compress_value = c.compress_value;
                }

                UM.add_undo(modified_chunk);

                if(RE.world.modify_block_wvp(event.data.wcoord1, event.data.blockid))
                {
                    maxz = event.data.wcoord1.z;
                    RE.refresh_block_visible(bc.chunk, bc.x, bc.y, bc.z);
                    RE.refresh_block_render_flags(bc.chunk, bc.x, bc.y, bc.z);

                    if(event.data.blockid != BLOCK_EMPTY) AE.Play_voxel_modif(EFFECT_POSE_BLOCK);
                    else AE.Play_voxel_modif(EFFECT_DELETE_BLOCK);

                    // RE.refresh_line_shadows(RE.highlight_wcoord, RE.highlight_wcoord);
                }

            }
            else if(RE.highlight_wcoord.x >= 0)
            {
                int xbeg = RE.highlight_wcoord.x;
                int xend = RE.highlight_wcoord2.x;

                int ybeg = RE.highlight_wcoord.y;
                int yend = RE.highlight_wcoord2.y;

                int zbeg = RE.highlight_wcoord.z;
                int zend = RE.highlight_wcoord2.z;

                if(RE.highlight_wcoord.x > RE.highlight_wcoord2.x)
                {
                    xbeg = RE.highlight_wcoord2.x;
                    xend = RE.highlight_wcoord.x;
                }

                if(RE.highlight_wcoord.y > RE.highlight_wcoord2.y)
                {
                    ybeg = RE.highlight_wcoord2.y;
                    yend = RE.highlight_wcoord.y;
                }

                if(RE.highlight_wcoord.z > RE.highlight_wcoord2.z)
                {
                    zbeg = RE.highlight_wcoord2.z;
                    zend = RE.highlight_wcoord.z;
                }

                maxz = zend;

                // Uint64 start = Get_time_ms();
                
                Uint64 estimate_itcounter = (Uint64)(yend-ybeg+1)*(Uint64)(xend-xbeg+1)*(Uint64)(zend-zbeg+1);

                coord3D beg_nwvp({xbeg, ybeg, zbeg});
                coord3D end_nwvp({xend, yend, zend});
                RE.world.translate_world_view_wposition(beg_nwvp.x, beg_nwvp.y, beg_nwvp.z);
                RE.world.translate_world_view_wposition(end_nwvp.x, end_nwvp.y, end_nwvp.z);

                if(beg_nwvp.x > end_nwvp.x)
                {
                    int tmp = beg_nwvp.x;
                    beg_nwvp.x = end_nwvp.x;
                    end_nwvp.x = tmp;
                }

                if(beg_nwvp.y > end_nwvp.y)
                {
                    int tmp = beg_nwvp.y;
                    beg_nwvp.y = end_nwvp.y;
                    end_nwvp.y = tmp;
                }

                block_coordonate bc_beg = beg_nwvp.to_block_coordonate();
                block_coordonate bc_end = end_nwvp.to_block_coordonate();
                int modified_chunks_n = (abs(bc_beg.chunk.x - bc_end.chunk.x) + 1)*(abs(bc_beg.chunk.y - bc_end.chunk.y) + 1)*(abs(bc_beg.chunk.z - bc_end.chunk.z) + 1);

                int compressed_n = RE.world.get_compressed_chunks_in_area(bc_beg.chunk, bc_end.chunk);

                chunk_save modified_chunks(modified_chunks_n - compressed_n, compressed_n);


                int uncompressed_index = 0;
                int compressed_index = 0;
                for (int i = bc_beg.chunk.x; i <= bc_end.chunk.x; i++) {
                    for (int j = bc_beg.chunk.y; j <= bc_end.chunk.y; j++) {
                        for (int k = bc_beg.chunk.z; k <= bc_end.chunk.z; k++) {
                            block_coordonate bc(0, 0, 0, {i, j, k});
                            // RE.world.translate_world_view_position(bc.chunk, bc.x, bc.y, bc.z);

                            if (RE.world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].compress_value == CHUNK_NON_UNIFORM) {
                                chunk_data* cd = modified_chunks.get_chunk(uncompressed_index++);
                                cd->coord = bc.chunk;
                                
                                memcpy(cd->blocks, RE.world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
                                cd->compress_value = RE.world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].compress_value;
                            }
                            else {
                                compressed_chunk_data* cd = modified_chunks.get_compressed(compressed_index++);
                                cd->coord = bc.chunk;
                                cd->compress_value = RE.world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].compress_value;
                            }
                            
                        }
                    }
                }

                UM.add_undo(modified_chunks); // might cause memory problems (˵◡_◡˵) 
                                              // it caused memory problems ლ(ಠ益ಠლ) 
                                              // probably wont anymore (✿◕‿◕)
                                              // it caused memory problems again ┻━┻ ︵ヽ(`Д´)ﾉ︵﻿ ┻━┻
                                              // Ok it was a heap corruption, but not his fault  (⩾﹏⩽)

                // if(estimate_itcounter > 0)
                // if(estimate_itcounter < 15000)
                if(estimate_itcounter < 150000)
                {
                    for(int y = ybeg; y <= yend; y ++)
                    {
                        for(int x = xbeg; x <= xend; x ++)
                        for(int z = zbeg; z <= zend; z ++)
                        {
                            RE.world.modify_block_wvp({x, y, z}, event.data.blockid);
                        }

                    }

                    chunk_coordonate corner1 = {beg_nwvp.x-1, beg_nwvp.y-1, beg_nwvp.z-1};
                    chunk_coordonate corner2 = {end_nwvp.x+1, end_nwvp.y+1, end_nwvp.z+1};

                    set_in_interval(corner1.x, 1, RE.world.max_block_coord.x-1);
                    set_in_interval(corner1.y, 1, RE.world.max_block_coord.y-1);
                    set_in_interval(corner1.z, 1, RE.world.max_block_coord.z-1);
                    set_in_interval(corner2.x, 1, RE.world.max_block_coord.x-1);
                    set_in_interval(corner2.y, 1, RE.world.max_block_coord.y-1);
                    set_in_interval(corner2.z, 1, RE.world.max_block_coord.z-1);

                    RE.refresh_cuboid(corner1, corner2);

                }
                else
                {
                    if(RE.highlight_type != HIGHLIGHT_VOLUME)
                    {
                        for(int y = ybeg; y <= yend; y ++)
                        for(int x = xbeg; x <= xend; x ++)
                        for(int z = zbeg; z <= zend; z ++)
                        {
                            RE.world.modify_block_wvp({x, y, z}, event.data.blockid);
                        }
                    }
                    else
                    {

                        int chunk_xbeg = (xbeg + CHUNK_SIZE - (xbeg%CHUNK_SIZE))/CHUNK_SIZE;
                        chunk_xbeg = xbeg%CHUNK_SIZE ? chunk_xbeg : xbeg/CHUNK_SIZE;
                        int chunk_xend = ((xend) - ((xend)%CHUNK_SIZE))/CHUNK_SIZE;

                        int chunk_ybeg = (ybeg + CHUNK_SIZE - (ybeg%CHUNK_SIZE))/CHUNK_SIZE;
                        chunk_ybeg = ybeg%CHUNK_SIZE ? chunk_ybeg : ybeg/CHUNK_SIZE;
                        int chunk_yend = ((yend) - ((yend)%CHUNK_SIZE))/CHUNK_SIZE;

                        int chunk_zbeg = (zbeg + CHUNK_SIZE - (zbeg%CHUNK_SIZE))/CHUNK_SIZE;
                        chunk_zbeg = zbeg%CHUNK_SIZE ? chunk_zbeg : zbeg/CHUNK_SIZE;
                        int chunk_zend = ((zend) - ((zend)%CHUNK_SIZE))/CHUNK_SIZE;

                        int chunk_xbeg_wvp = chunk_xbeg;
                        int chunk_xend_wvp = chunk_xend;
                        int chunk_ybeg_wvp = chunk_ybeg;
                        int chunk_yend_wvp = chunk_yend;
                        int chunk_zbeg_wvp = chunk_zbeg;
                        int chunk_zend_wvp = chunk_zend;

                        if(RE.world.world_view_position%2)
                        {
                            chunk_xbeg_wvp = chunk_ybeg;
                            chunk_xend_wvp = chunk_yend;

                            chunk_ybeg_wvp = chunk_xbeg;
                            chunk_yend_wvp = chunk_xend;
                        }

                        if(RE.world.world_view_position == 1 || RE.world.world_view_position == 2)
                        {
                            chunk_ybeg_wvp = RE.world.max_chunk_coord.y-chunk_ybeg_wvp;
                            chunk_yend_wvp = RE.world.max_chunk_coord.y-chunk_yend_wvp;

                            int tmp = chunk_ybeg_wvp+1;
                            chunk_ybeg_wvp = chunk_yend_wvp+1;
                            chunk_yend_wvp = tmp;
                        }

                        if(RE.world.world_view_position == 3 || RE.world.world_view_position == 2)
                        {
                            chunk_xbeg_wvp = RE.world.max_chunk_coord.x-chunk_xbeg_wvp;
                            chunk_xend_wvp = RE.world.max_chunk_coord.x-chunk_xend_wvp;

                            int tmp = chunk_xbeg_wvp+1;
                            chunk_xbeg_wvp = chunk_xend_wvp+1;
                            chunk_xend_wvp = tmp;
                        }

                        if(RE.world.mask_mod)
                        {
                            for(int cx = chunk_xbeg_wvp; cx < chunk_xend_wvp; cx++)
                            for(int cy = chunk_ybeg_wvp; cy < chunk_yend_wvp; cy++)
                            for(int cz = chunk_zbeg_wvp; cz < chunk_zend_wvp; cz++)
                            {
                                // itcounter += CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

                                chunk *ch = &RE.world.chunks[cx][cy][cz];
                                if(ch->compress_value == CHUNK_EMPTY)
                                {
                                    memset(RE.world.chunks[cx][cy][cz].blocks, event.data.blockid, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(block));
                                    RE.world.chunks[cx][cy][cz].compress_value = event.data.blockid;
                                    
                                    // chunk_coordonate c = {cx, cy, cz};
                                    // PE->add_event(PHYSICS_EVENT_WATER_CHECK_CHUNK, &c);
                                }
                                else
                                {
                                    for(int x = 0; x < CHUNK_SIZE; x++)
                                    for(int y = 0; y < CHUNK_SIZE; y++)
                                    for(int z = 0; z < CHUNK_SIZE; z++)
                                    {
                                        block_coordonate bc;
                                        bc.chunk = {cx, cy, cz};
                                        bc.x = x;
                                        bc.y = y;
                                        bc.z = z;
                                        RE.world.modify_block_bc(bc, event.data.blockid);
                                    }
                                }

                            }
                        }
                        else
                        {
                            for(int cx = chunk_xbeg_wvp; cx < chunk_xend_wvp; cx++)
                            for(int cy = chunk_ybeg_wvp; cy < chunk_yend_wvp; cy++)
                            for(int cz = chunk_zbeg_wvp; cz < chunk_zend_wvp; cz++)
                            {
                                // itcounter += CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
                                memset(RE.world.chunks[cx][cy][cz].blocks, event.data.blockid, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(block));
                                RE.world.chunks[cx][cy][cz].compress_value = event.data.blockid;
                                
                                // chunk_coordonate c = {cx, cy, cz};
                                // PE->add_event(PHYSICS_EVENT_WATER_CHECK_CHUNK, &c);
                            }
                        }
                    
                        for(int z = zbeg; z <= zend; z ++)
                            {

                                for(int y = ybeg; y <= yend; y ++)
                                {

                                    for(int x = xbeg; x <= xend; x ++)
                                    {
                                        if(z >= chunk_zbeg*CHUNK_SIZE && z < chunk_zend*CHUNK_SIZE)
                                            if(y >= chunk_ybeg*CHUNK_SIZE && y < chunk_yend*CHUNK_SIZE)
                                                if(x == chunk_xbeg*CHUNK_SIZE)
                                                    x = chunk_xend*CHUNK_SIZE;

                                        RE.world.modify_block_wvp({x, y, z}, event.data.blockid);
                                        // itcounter ++;
                                    }
                                    
                                }

                            }
                    }


                    drop_all_nfs_event();
                    // RE.projection_grid.clear();
                    RE.projection_grid.refresh_visible_frags(RE.target, RE.screen->w, RE.screen->h, RE.block_onscreen_size);
                    RE.projection_grid.save_curr_interval();
                    // RE.refresh_pg_onscreen();
                    // RE.refresh_pg_block_visible();
                    add_nfs_event(NFS_OP_PG_ONSCREEN);
                    add_nfs_event(NFS_OP_ALL_BLOCK_VISIBLE);
                    add_nfs_event(NFS_OP_ALL_RENDER_FLAG); 
                }

                RE.highlight_wcoord2 = {HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD, HIGHLIGHT_NOCOORD};

                // Uint64 end = Get_time_ms();
                // std::locale::global(std::locale(""));
                // std::cout << "\nPosing " << format(estimate_itcounter) << " blocks in " << end-start << " ms.";
                // std::cout << "\nActually posed " << format(itcounter) << " blocks \n";
                // SecondaryThread_opcode |= STHREAD_OP_PG_BLOCK_VISIBLE;  

                if(event.data.blockid) AE.Play_voxel_modif(EFFECT_POSE_MULTIPLE_BLOCKS);
                else AE.Play_voxel_modif(EFFECT_DELETE_MULTIPLE_BLOCKS);
            }

            RE.projection_grid.refresh_all_identical_line(); // 30 ms optimisiable assez simplement 
            // RE.world.find_highest_nonemptychunk(); // 10 ms useless


        }
            PE->world_mutex.unlock();
            break;

        case GAME_EVENT_SINGLE_BLOCK_MOD_ALT :
            {
                // PE->world_mutex.lock();
                block_coordonate bc = event.data.coord1;

                block *b = &RE.world.
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
                    b->id = event.data.blockid;

                    RE.refresh_block_visible(bc.chunk, bc.x, bc.y, bc.z);

                    RE.world.compress_chunk(bc.chunk.x, bc.chunk.y, bc.chunk.z);

                    coord3D wc = RE.world.convert_coord(bc);

                    RE.refresh_line_shadows(wc.x, wc.x, wc.y, wc.z);

                    coord3D pgc = RE.projection_grid.convert_wcoord(wc.x, wc.y, wc.z);
                    RE.set_block_renderflags(pgc.x, pgc.y, pgc.z);

                    refresh_identical_line = true;
                }
                // PE->world_mutex.unlock();
            }
            break;

        case GAME_EVENT_PLACE_SPRITE_VOXEL : // old
            {
                // std::cout << "\nplacing sprite voxel";
                // std::cout << "at coord " 
                // << event.data.svoxel->wcoord.x << " "
                // << event.data.svoxel->wcoord.y << " "
                // << event.data.svoxel->wcoord.z << " ";

                block_coordonate bc = RE.world.convert_wcoord(event.data.svoxel->wcoord.x, 
                                                              event.data.svoxel->wcoord.y, 
                                                              event.data.svoxel->wcoord.z);
                
                block *b = &RE.world.
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
                    event.data.svoxel->is_occluded = false;
                    b->id = event.data.svoxel->id;

                    RE.world.compress_chunk(bc.chunk.x, bc.chunk.y, bc.chunk.z);
                    // RE.refresh_block_visible(bc.chunk, bc.x, bc.y, bc.z);

                    // coord3D wc = event.data.svoxel->wcoord;

                    // RE.refresh_line_shadows(wc.x, wc.x, wc.y, wc.z);

                    // coord3D pgc = RE.projection_grid.convert_wcoord(wc.x, wc.y, wc.z);
                    // RE.set_block_renderflags(pgc.x, pgc.y, pgc.z);

                    // RE.refresh_block_render_flags(bc.chunk, bc.x, bc.y, bc.z);

                    refresh_identical_line = true;
                }

            }
            break;

        case GAME_EVENT_REMOVE_SPRITE_VOXEL :
            {
                block_coordonate bc = RE.world.convert_wcoord(event.data.svoxel->wcoord.x, 
                                                              event.data.svoxel->wcoord.y, 
                                                              event.data.svoxel->wcoord.z);
                
                block *b = &RE.world.
                            chunks
                            [bc.chunk.x]
                            [bc.chunk.y]
                            [bc.chunk.z]
                            .blocks
                            [bc.x]
                            [bc.y]
                            [bc.z];

                b->id = BLOCK_EMPTY;
                RE.world.compress_chunk(bc.chunk.x, bc.chunk.y, bc.chunk.z);
            }
            break;

        case GAME_EVENT_INIT_WORLD_RENDER_FLAGS :
            SecondaryThread_opcode |= STHREAD_OP_ALL_RENDER_FLAG;

        default:
            break;
        }

        // event_queue.pop();
        reader --;
    }


    if(refresh_identical_line)
    {
        RE.projection_grid.refresh_all_identical_line(); 
        RE.render_world();
    }

    SDL_UnlockMutex(init_cond);
    SDL_CondSignal(new_frame_to_render);

    // if(SecondaryThread_opcode || !game_is_running)
    // {
    //     // std::cout << "MEH class : Signaling new_frame_to render\n";
    //     SDL_CondSignal(new_frame_to_render);
    // }
}

void Multithreaded_Event_Handler::drop_game_event()
{
    // while(event_queue.size() != 0)
    // {
    //     event_queue.pop();
    // }
    reader = -1;
}

int SecondaryThread_operations(void *data)
{
    Multithreaded_Event_Handler* MEH = (Multithreaded_Event_Handler*)data;

    while(MEH->game_is_running)
    {
        SDL_LockMutex(MEH->init_cond); 

        if(MEH->game_is_running && MEH->RE.projection_grid.pos[0])
        {
            SDL_CondWait(MEH->new_frame_to_render, MEH->init_cond);

            if(MEH->RE.force_nonpanorama_mode || (!MEH->RE.test_panorama_mode() && MEH->RE.projection_grid.pos[0]))
            {
                // std::cout << "2\n";
                SDL_LockMutex(MEH->render_transpw_mut);
                MEH->RE.render_transparent_world(true);
                SDL_UnlockMutex(MEH->render_transpw_mut);
            }

            /////////////////////// PG BLOCK VISIBLE  ////////////////////////
            if(MEH->SecondaryThread_opcode & STHREAD_OP_PG_BLOCK_VISIBLE)
            {
                MEH->RE.refresh_pg_block_visible();
            }

            ///////////////////// BLOCK VISIBLE  //////////////////////
            if(MEH->SecondaryThread_opcode & STHREAD_OP_ALL_BLOCK_VISBLE)
                MEH->RE.refresh_all_block_visible2();

            else if(MEH->SecondaryThread_opcode & STHREAD_OP_SINGLE_BLOCK_VISBLE)
                MEH->RE.refresh_block_visible(MEH->STO_data.coord1.chunk, MEH->STO_data.coord1.x, MEH->STO_data.coord1.y, MEH->STO_data.coord1.z);

            ////////////////////// RENDER FLAGS  ///////////////////////
            if(MEH->SecondaryThread_opcode & STHREAD_OP_ALL_RENDER_FLAG)
            {
                MEH->RE.refresh_all_render_flags2();
                MEH->RE.projection_grid.refresh_all_identical_line();
            }

            else if(MEH->SecondaryThread_opcode & STHREAD_OP_SINGLE_RENDER_FLAGS)
            {
                MEH->RE.refresh_block_render_flags(MEH->STO_data.coord1.chunk, MEH->STO_data.coord1.x, MEH->STO_data.coord1.y, MEH->STO_data.coord1.z);
                MEH->RE.projection_grid.refresh_all_identical_line();
            }
        }

        SDL_UnlockMutex(MEH->init_cond);
        // SDL_CondSignal(MEH->secondary_frame_op_finish);
    }

    return 0;
}

void Multithreaded_Event_Handler::add_nfs_event(const int nfs_event_id)
{
    nfs_event_queue.push(nfs_event_id);
    SDL_CondSignal(new_nfs_event);
}

void Multithreaded_Event_Handler::drop_all_nfs_event()
{
    RE.abort_rendrefresh = true;

    while(nfs_event_queue.size() > 1)
        nfs_event_queue.pop();

    SDL_Delay(10);
    RE.abort_rendrefresh = false;
}

int NFS_operations(void *data)
{
    Multithreaded_Event_Handler* MEH = (Multithreaded_Event_Handler*)data;

    int event;
    
    while(MEH->game_is_running)
    {
        SDL_LockMutex(MEH->nfs_mut);

        if(MEH->game_is_running)
        {
            SDL_CondWait(MEH->new_nfs_event, MEH->nfs_mut);

            event = NFS_OP_NONE;

            while(!MEH->nfs_event_queue.empty())
            {
                event = MEH->nfs_event_queue.front();
                MEH->nfs_event_queue.pop();

                MEH->is_NFS_reading_to_wpg = true;
                SDL_LockMutex(MEH->world_and_projection_grid_mut);
                switch (event)
                {
                case NFS_OP_ALL_BLOCK_VISIBLE :
                    // std::cout << "Bon, moment de refresh les blocks visibles\n";

                    MEH->RE.force_nonpanorama_mode = true;

                    // std::cout << "refreshing all block visible";
                    // startbenchrono();
                    MEH->RE.refresh_all_block_visible2();
                    // endbenchrono();

                    if(!MEH->RE.abort_rendrefresh)
                    {
                        MEH->RE.projection_grid.refresh_all_identical_line();
                        if(MEH->RE.test_panorama_mode())
                            MEH->RE.render_world();
                    }
                    
                    MEH->RE.force_nonpanorama_mode = false;

                    // MEH->RE.abort_rendrefresh = false;
                    // std::cout << "Bon, j'ai fini de refresh les blocks visibles\n";
                    break;
                
                case NFS_OP_ALL_RENDER_FLAG :
                    if(MEH->RE.projection_grid.pos[0])
                    {
                        // std::cout << "Bon, moment de refresh les blocks \n";
                        MEH->RE.force_nonpanorama_mode = true;
                        // std::cout << "refreshing all render flags";
                        // startbenchrono();
                        MEH->RE.refresh_all_render_flags2();
                        // endbenchrono();
                        MEH->RE.force_nonpanorama_mode = false;

                        if(!MEH->RE.abort_rendrefresh)
                        {
                            MEH->RE.projection_grid.refresh_all_identical_line();
                            if(MEH->RE.test_panorama_mode())
                                MEH->RE.render_world();
                        }
                        // MEH->RE.abort_rendrefresh = false;
                        // std::cout << "Bon, j'ai fini de refresh les blocks \n";
                    }
                    break;

                case NFS_OP_PG_ONSCREEN  : 
                    if(MEH->RE.test_panorama_mode()) break;
                    MEH->RE.refresh_pg_onscreen();
                    if(MEH->RE.abort_rendrefresh) break;
                    MEH->RE.refresh_pg_block_visible();
                    if(MEH->RE.abort_rendrefresh) break;
                    MEH->RE.projection_grid.refresh_all_identical_line();
                    break;

                case NFS_NEW_HEIGHT_RENDER :
                    SDL_LockMutex(MEH->RE.campg_mut);
                    MEH->RE.force_nonpanorama_mode = true;
                    MEH->RE.refresh_height_render();
                    MEH->RE.projection_grid.refresh_all_identical_line();
                    MEH->RE.force_nonpanorama_mode = false;
                    SDL_UnlockMutex(MEH->RE.campg_mut);
                    break;  

                case NFS_OP_PG_MHR :
                    MEH->RE.refresh_pg_MHR();
                    break;
                
                case NFS_GENERATE_NEW_MAP :
                    MEH->WG->generate_world(MEH->RE.world);
                    MEH->RE.world.find_highest_nonemptychunk();
                    MEH->RE.max_height_render = (MEH->RE.world.highest_nonemptychunk+1)*CHUNK_SIZE;

                    SDL_LockMutex(MEH->RE.campg_mut);
                    MEH->RE.center_camera();
                    MEH->RE.projection_grid.refresh_visible_frags(MEH->RE.target, MEH->RE.screen->w, MEH->RE.screen->h, MEH->RE.block_onscreen_size);
                    MEH->RE.projection_grid.save_curr_interval();
                    SDL_UnlockMutex(MEH->RE.campg_mut);
                    break;

                default:
                    break;
                }
                SDL_UnlockMutex(MEH->world_and_projection_grid_mut);
                MEH->is_NFS_reading_to_wpg = false;
            }
        }

        SDL_UnlockMutex(MEH->nfs_mut);
    }

    return 0;
}