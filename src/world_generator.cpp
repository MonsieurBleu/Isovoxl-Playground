#include <game.hpp>

void World_Generator::load_heightmap_from_file(const std::string &filename)
{
    HMt.init_from_file(filename.c_str());
}

void World_Generator::load_heightmap_from_id()
{
    if(HMt.ptr)
    {
        GPU_FreeImage(HMt.ptr);
        HMt.ptr = NULL;
    }

    std::string filename;

    switch (preset_id)
    {
        case 0 : filename = "Alien Landscape"; break;

        case 1 : filename = "Great Lakes"; break;

        case 2 : filename = "Rocky Land And Rivers"; break;

        case 3 : filename = "Mountain Range"; break;

        case 4 : filename = "Rugged Terrain"; break; // old
    
    default: return; break;
    }
    
    load_heightmap_from_file("ressources/textures/heightmaps/"+filename+".png");
}

void World_Generator::load_heightmap_from_shder(int seed)
{
    HMt.ptr = GPU_CreateImage(1024, 1024, GPU_FORMAT_RGBA);
    GPU_SetAnchor(HMt.ptr, 0, 0);
    GPU_Target *HMtarget = GPU_LoadTarget(HMt.ptr);

    Procedural_Erosion_Generator.activate();

    GPU_RectangleFilled2(HMtarget, {0, 0, 1.f*HMt.ptr->w, 1.f*HMt.ptr->h}, {0, 255, 255, 255});

    Procedural_Erosion_Generator.deactivate();

    GPU_Flip(HMtarget);

    // HM = GPU_CopySurfaceFromImage(Pnoiset.ptr);

    // GPU_FreeImage(HMt.ptr);
}


void World_Generator::init_shaders()
{
    Spass1.load("shader/UI/UI_font.vert", "shader/world_generator/pass1.frag", NULL);
    Procedural_Erosion_Generator.load("shader/UI/UI_font.vert", "shader/world_generator/pass2.frag", NULL);
}

void World_Generator::generate_pnoise(Uint64 seed, int w, int h)
{
    // if(Pnoiset.ptr)
    // {
    //     GPU_FreeImage(Pnoiset.ptr);
    //     Pnoiset.ptr = NULL;
    // }

    Pnoiset.ptr = GPU_CreateImage(w, h, GPU_FORMAT_RGBA);
    GPU_SetAnchor(Pnoiset.ptr, 0, 0);
    GPU_Target *HMtarget = GPU_LoadTarget(Pnoiset.ptr);

    Spass1.activate();

    GPU_RectangleFilled2(HMtarget, {0, 0, 1.f*Pnoiset.ptr->w, 1.f*Pnoiset.ptr->h}, {0, 255, 255, 255});

    Spass1.deactivate();

    GPU_Flip(HMtarget);

    Pnoise = GPU_CopySurfaceFromImage(Pnoiset.ptr);

    // GPU_SaveImage(Pnoiset.ptr, "ressources/heightmap.png", GPU_FILE_AUTO);

    GPU_FreeImage(Pnoiset.ptr);
    // GPU_FreeTarget(HMtarget);
}

const Uint8 water = 241;
float height_ratio;

void World_Generator::generate_world_test(chunk_coordonate world_size, World &world)
{
    srand(time(NULL));

    init_shaders();
    generate_pnoise(0, world_size.x*8, world_size.y*8);

    Texture HMscaled;
    HMscaled.ptr = GPU_CreateImage(world_size.x*8, world_size.y*8, GPU_FORMAT_RGBA);

    GPU_Target *HMtarget = GPU_LoadTarget(HMscaled.ptr);

    // GPU_ClearColor(HMtarget, {255, 255, 255, 255});
    GPU_BlitRect(HMt.ptr, NULL, HMtarget, NULL);
    // GPU_Blit(HMt.ptr, NULL, HMtarget, 0, 0);
    GPU_Flip(HMtarget);

    world.init(world_size.x, world_size.y, world_size.z);

    SDL_Surface *HMstmp = GPU_CopySurfaceFromImage(HMscaled.ptr);

    std::cout << "\nHM img pixel format : " << SDL_GetPixelFormatName(HMstmp->format->format) << "\n";

    // do not work for jpg's
    if(HMstmp->format->format != SDL_PIXELFORMAT_ABGR8888)
    {
        std::cout << "Doing converstion to ABGR8888\n";
        HM = SDL_ConvertSurfaceFormat(HMstmp, SDL_PIXELFORMAT_ABGR8888, 0);

        SDL_FreeSurface(HMstmp);
    }
    else
    {
        HM = HMstmp;
    }

    height_ratio = ((world.max_chunk_coord.x+1)/128.0);

    float ratiox = HM->pitch;
    float ratioy = 4;

    // generate_height_Mountains(world, ratiox, ratioy);
    // generate_height_Plains(world, ratiox, ratioy);
    // generate_height_Canyons(world, ratiox, ratioy);
    // generate_height_Dunes(world, ratiox, ratioy);
    // generate_height_Icepeaks(world, ratiox, ratioy);
    // generate_height_SnowValley(world, ratiox, ratioy);
    // generate_height_Dark(world, ratiox, ratioy);
    generate_height_Fantasy(world, ratiox, ratioy);

    SDL_FreeSurface(HM);
    world.compress_all_chunks();
}

void World_Generator::prepare_batch_operations()
{
    if(preset_id == BIOME_FLAT) return;

    switch (world_size_id)
    {
        case 0 : world_size = {256, 256, 75}; break;

        case 1 : world_size = {128, 128, 64}; break;

        case 2 : world_size = {64, 64, 32}; break;

        case 3 : world_size = {32, 32, 32}; break;

        case 4 : world_size = {16, 16, 16}; break;
    
    default: return; break;
    }

    load_heightmap_from_id();
    init_shaders();
    // load_heightmap_from_shder();
    generate_pnoise(0, world_size.x*8, world_size.y*8);
    
    HMscaled.ptr = GPU_CreateImage(world_size.x*8, world_size.y*8, GPU_FORMAT_RGBA);

    GPU_Target *HMtarget = GPU_LoadTarget(HMscaled.ptr);

    GPU_BlitRect(HMt.ptr, NULL, HMtarget, NULL);
    GPU_Flip(HMtarget);
    
    HM = GPU_CopySurfaceFromImage(HMscaled.ptr);
}

void World_Generator::generate_world(World &world)
{
    is_generating = 1;

    height_ratio = ((world.max_chunk_coord.x+1)/128.0);

    float ratiox = HM->pitch;
    float ratioy = 4;

    world.reset_chunks();

    switch (biome_id)
    {
        case BIOME_PLAINS : generate_height_Plains(world, ratiox, ratioy); break;

        case BIOME_DUNES : generate_height_Dunes(world, ratiox, ratioy); break;

        case BIOME_SNOWVALLEY : generate_height_SnowValley(world, ratiox, ratioy); break;

        case BIOME_MOUNTAINS : generate_height_Mountains(world, ratiox, ratioy); break;

        case BIOME_CANYONS : generate_height_Canyons(world, ratiox, ratioy); break;

        case BIOME_ICEPEAKS : generate_height_Icepeaks(world, ratiox, ratioy); break;

        case BIOME_DARK : generate_height_Dark(world, ratiox, ratioy); break;

        case BIOME_FANTASY : generate_height_Fantasy(world, ratiox, ratioy); break;
    
        case BIOME_FLAT : generate_height_Flat(world); break;

    default: break;
    }

    if(biome_id != BIOME_FLAT)
        SDL_FreeSurface(HM);

    world.compress_all_chunks();
    is_generating = 0;
}


//////////// MOUNTAINS ////////////
const Uint8 Mountains_sand = 17;
const Uint8 Mountains_rock = 30;
const Uint8 Mountains_grass = 36; //39 color variation
const Uint8 Mountains_grass2 = 35; //38; color variation
const Uint8 Mountains_dirt = 19;
const Uint8 Mountains_snow = 25;

Uint16 Mountains_sealevel;
const Uint16 Mountains_sealevel_preset[5] = {57, 30, 100, 46, 0};

const float Mountains_height_shrink_val = 0.75;
float Mountains_height_shrink = 0.75;

Uint8 hfunc_Mountains(const coord3D &pos, Uint16 height, int randmod)
{
    coord3D p = pos;

    if(height > (Mountains_sealevel+32)*height_ratio)
        p.z += (float)(pos.z)/(float)(height)*30*height_ratio; //*15; 

    p.z -= randmod*2;

    if(p.z > (Mountains_sealevel+167)*height_ratio && pos.z > height-1)
        return Mountains_snow;

    if(p.z > (Mountains_sealevel+152)*height_ratio)
        return Mountains_rock;

    if(pos.z > height-50 && pos.z < height-1)
        return Mountains_dirt;

    if(p.z > (Mountains_sealevel+72)*height_ratio && pos.z > height-1)
        return Mountains_grass2;

    p.z += randmod;

    if(p.z < (Mountains_sealevel+5)*height_ratio)
        return Mountains_sand;

    return Mountains_grass;
}

void World_Generator::generate_height_Mountains(World &world, int ratiox, int ratioy)
{
    Mountains_height_shrink = Mountains_height_shrink_val*height_ratio;
    Mountains_sealevel = Mountains_sealevel_preset[preset_id];

    int pixelx = 0;
    int pixely = 0;

    // std::cout << world.max_block_coord << "\n";

    for(int x = 0; x < world.max_block_coord.x; x++)
    {
        pixely = 0;

        for(int y = 0; y < world.max_block_coord.y; y++)
        {
            if(*abord_operations) return;

            Uint16 height = *(Uint16*)((Uint8*)HM->pixels + pixelx + pixely + 1);
            height = height*Mountains_height_shrink;


            short randmod = *(Uint16*)((Uint8*)Pnoise->pixels + pixelx + pixely + 1);
            randmod = randmod/32.0;

            // 255

            int z = 0;

            if(height >= world.max_block_coord.z) height = world.max_block_coord.z-1;

            // std::cout << height << "\n";

            for(; z < height-(3+randmod%2); z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = Mountains_rock;
            }
            for(; z <= height; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = hfunc_Mountains({x, y, z}, height, randmod);
            }
            for(; z <= Mountains_sealevel*height_ratio; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = water;
            }

            pixely += ratioy;
        }

        pixelx += ratiox;
    }
}


//////////// Plains ////////////
const Uint8 Plains_sand = 17;
const Uint8 Plains_sand2 = 24;
const Uint8 Plains_rock = 30;
const Uint8 Plains_grass = 34;
// const Uint8 Plains_grass2 = 36; 
// const Uint8 Plains_grass3 = 35; 
const Uint8 Plains_grass2 = 38; 
const Uint8 Plains_grass3 = 39; 
const Uint8 Plains_dirt = 19;
const Uint8 Plains_snow = 25;

Uint16 Plains_sealevel;
const Uint16 Plains_sealevel_preset[5] = {2, 0, 2, 0, 0};

const float Plains_height_shrink_val = 0.15; 
float Plains_height_shrink = 0.0;

Uint8 hfunc_Plains(const coord3D &pos, Uint16 height, int randmod)
{
    if(pos.z < height)
        return Plains_dirt;

    coord3D p = pos;

    // int beg  = 0;
    // int size = 2;
    // if(randmod <= beg+size && randmod >= beg)
    //     return Plains_grass3;

    p.z += (randmod/32)*0.10;

    // if(p.z <= (Plains_sealevel-4)*height_ratio)
    //     return Plains_dirt;

    if(p.z <= Plains_sealevel*height_ratio)
    {
        int beg  = 50;
        int size = 50;

        if(randmod <= beg+size && randmod >= beg)
            return Plains_sand2;

        return Plains_sand;
    }


    p.z += randmod%2;

    if(p.z > (Plains_sealevel+13)*height_ratio)
        return Plains_grass3;
    
    // p.z -= randmod%8;

    // if(p.z < (Plains_sealevel+2)*height_ratio)
    //     return Plains_sand;    

    return Plains_grass2;
}

void World_Generator::generate_height_Plains(World &world, int ratiox, int ratioy)
{
    Plains_height_shrink = Plains_height_shrink_val*height_ratio;
    Plains_sealevel = Plains_sealevel_preset[preset_id];

    int pixelx = 0;
    int pixely = 0;

    for(int x = 0; x < world.max_block_coord.x; x++)
    {
        pixely = 0;

        for(int y = 0; y < world.max_block_coord.y; y++)
        {
            if(*abord_operations) return;

            Uint16 height = *(Uint16*)((Uint8*)HM->pixels + pixelx + pixely + 1);

            height = sqrt(height)*7*Plains_height_shrink;

            // int randmod = cos(x*x + y*y)*4;
            // int randmod = rand()%4;
            short randmod = *(Uint16*)((Uint8*)Pnoise->pixels + pixelx + pixely + 1);
            // randmod = randmod/32.0;

            // 255

            int z = 0;

            if(height >= world.max_block_coord.z) height = world.max_block_coord.z-1;

            for(; z < height-3+randmod%2; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = Plains_rock;
            }
            for(; z <= height; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = hfunc_Plains({x, y, z}, height, randmod);
            }
            for(; z <= Plains_sealevel*height_ratio; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = water;
            }

            pixely += ratioy;
        }

        pixelx += ratiox;
    }
}


//////////// Canyons ////////////
const Uint8 Canyons_sand = 17;
const Uint8 Canyons_sand2 = 23;
const Uint8 Canyons_sand3 = 18;
const Uint8 Canyons_sand4 = 19;
const Uint8 Canyons_sand5 = 49;
const Uint8 Canyons_rock = 30;

Uint16 Canyons_sealevel;
const Uint16 Canyons_sealevel_preset[5] = {115, 200, 70, 150, 0};

const float Canyons_height_shrink_val = 0.50; 
float Canyons_height_shrink;

Uint8 hfunc_Canyons(const coord3D &pos, Uint16 height, int randmod)
{
    coord3D p = pos;

    p.z += randmod/100;

    if(p.z > (Canyons_sealevel+25)*height_ratio)
    {
        // int level_height = 4;
        int level_height = 8*height_ratio;

        if((p.z/level_height)%4 >= 3)
            return Canyons_sand5;

        if((p.z/level_height)%4 >= 2)
            return Canyons_sand4;

        if((p.z/level_height)%4 >= 1)
            return Canyons_sand3;


        return Canyons_sand2;
    }

    if(pos.z < height-2)
        return Canyons_rock;

    return Canyons_sand;
}

void World_Generator::generate_height_Canyons(World &world, int ratiox, int ratioy)
{
    Canyons_height_shrink = Canyons_height_shrink_val*height_ratio;
    Canyons_sealevel = Canyons_sealevel_preset[preset_id];

    int pixelx = 0;
    int pixely = 0;

    for(int x = 0; x < world.max_block_coord.x; x++)
    {
        pixely = 0;

        for(int y = 0; y < world.max_block_coord.y; y++)
        {
            if(*abord_operations) return;

            Uint16 height = *(Uint16*)((Uint8*)HM->pixels + pixelx + pixely + 1);
            short randmod = *(Uint16*)((Uint8*)Pnoise->pixels + pixelx + pixely + 1);

            // if(height > 512) height = 512;
            // height *= 0.5;

            height = 512-height;
            height = pow(height/512.0, 2)*750;
            // height = pow(height/512.0, 0.5)*500;

            height = height*Canyons_height_shrink;
            // height += 3.5*(height/512.0)*randmod/250;
            // height += 2.0090*height_ratio*randmod/256.0;
            // height += 2.0*height_ratio*randmod/256.0;

            if(randmod <= 75 && randmod >= 50) height += 1;

            int z = 0;

            if(height >= world.max_block_coord.z) height = world.max_block_coord.z-1;

            for(; z <= height; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = hfunc_Canyons({x, y, z}, height, randmod);
            }
            for(; z <= Canyons_sealevel*height_ratio; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = water;
            }

            pixely += ratioy;
        }

        pixelx += ratiox;
    }
}

//////////// Dunes ////////////
const Uint8 Dunes_sand = 17;
const Uint8 Dunes_sand2 = 24;
const Uint8 Dunes_grass = 36; 

Uint16 Dunes_sealevel;
const Uint16 Dunes_sealevel_preset[5] = {45, 25, 35, 50, 0};

const float Dunes_height_shrink_val = 0.20; 
float Dunes_height_shrink = 0.0;

Uint8 hfunc_Dunes(const coord3D &pos, Uint16 height, int randmod)
{
    int beg  = 50;
    int size = 50;

    coord3D p = pos;

    p.z += (randmod/32)%2;

    if(pos.z > height-2)
    if(p.z >= (Dunes_sealevel-1)*height_ratio && p.z < (Dunes_sealevel)*height_ratio + 1)
        return Dunes_grass;

    randmod *= 1.0 + cos(pos.z + pos.y)*0.5;

    if(randmod <= beg+size && randmod >= beg)
        return Dunes_sand2;

    return Dunes_sand;
}

void World_Generator::generate_height_Dunes(World &world, int ratiox, int ratioy)
{
    Dunes_height_shrink = Dunes_height_shrink_val*height_ratio;
    Dunes_sealevel = Dunes_sealevel_preset[preset_id];

    int pixelx = 0;
    int pixely = 0;

    for(int x = 0; x < world.max_block_coord.x; x++)
    {
        pixely = 0;

        for(int y = 0; y < world.max_block_coord.y; y++)
        {
            if(*abord_operations) return;
            Uint16 height = *(Uint16*)((Uint8*)HM->pixels + pixelx + pixely + 1);

            height = 512-height;
            height = height*Dunes_height_shrink;

            short randmod = *(Uint16*)((Uint8*)Pnoise->pixels + pixelx + pixely + 1);
            
            if(randmod <= 75 && randmod >= 70) height += 1;

            int z = 0;

            if(height >= world.max_block_coord.z) height = world.max_block_coord.z-1;

            for(; z < height-3+randmod%2; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = Dunes_sand2;
            }
            for(; z <= height; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = hfunc_Dunes({x, y, z}, height, randmod);
            }
            for(; z <= Dunes_sealevel*height_ratio; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = water;
            }

            pixely += ratioy;
        }

        pixelx += ratiox;
    }
}

//////////// Icepeaks ////////////
const Uint8 Icepeaks_rock = 30;
const Uint8 Icepeaks_dirt = 19;
const Uint8 Icepeaks_snow = 25;
const Uint8 Icepeaks_snow2 = 89;

Uint16 Icepeaks_sealevel;
const Uint16 Icepeaks_sealevel_preset[5] = {40, 25, 100, 0, 0};

const float Icepeaks_height_shrink_val = 0.50;
float Icepeaks_height_shrink = 0.75; //0.75

Uint8 hfunc_Icepeaks(const coord3D &pos, Uint16 height, int randmod)
{
    coord3D p = pos;

    if(height > (Icepeaks_sealevel+50)*height_ratio)
        p.z += (float)(pos.z)/(float)(height)*30*height_ratio; //*15; 

    if(p.z+randmod%32 > (Icepeaks_sealevel+100)*height_ratio)
        if((pos.z+randmod%32) == height) return water+1;

    // p.z -= randmod*2;

    if(p.z > Icepeaks_sealevel*height_ratio && pos.z > height-2)
        return Icepeaks_snow;

    // if(p.z > (Icepeaks_sealevel+152)*height_ratio)
    //     return Icepeaks_rock;

    // p.z += randmod*2;

    if(pos.z >= height-50 && pos.z < height-2)
        return Icepeaks_dirt;

    if(p.z >= (Icepeaks_sealevel)*height_ratio && pos.z > height-1)
        return water+1;



    return Icepeaks_dirt;
}

void World_Generator::generate_height_Icepeaks(World &world, int ratiox, int ratioy)
{
    Icepeaks_height_shrink = Icepeaks_height_shrink_val*height_ratio;
    Icepeaks_sealevel = Icepeaks_sealevel_preset[preset_id];

    int pixelx = 0;
    int pixely = 0;

    for(int x = 0; x < world.max_block_coord.x; x++)
    {
        pixely = 0;

        for(int y = 0; y < world.max_block_coord.y; y++)
        {
            if(*abord_operations) return;

            Uint16 height = *(Uint16*)((Uint8*)HM->pixels + pixelx + pixely + 1);

            height = height*Icepeaks_height_shrink;

            height = pow(height/512.0, 1.15)*512*1.5;

            // int randmod = cos(x*x + y*y)*4;
            // int randmod = rand()%4;
            short randmod = *(Uint16*)((Uint8*)Pnoise->pixels + pixelx + pixely + 1);
            randmod = randmod/32.0;

            // 255

            int z = 0;

            if(height >= world.max_block_coord.z) height = world.max_block_coord.z-1;

            for(; z < height-(2+randmod%2); z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = Mountains_rock;
            }
            for(; z <= height; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = hfunc_Icepeaks({x, y, z}, height, randmod);
            }
            for(; z <= Icepeaks_sealevel*height_ratio; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = water;
            }
            if(height <= Icepeaks_sealevel*height_ratio)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = water+1;
            }

            pixely += ratioy;
        }

        pixelx += ratiox;
    }
}

//////////// Plains ////////////

const Uint8 SnowValley_snow = 25;
const Uint8 SnowValley_rock = Mountains_rock;
const Uint8 SnowValley_dirt = Mountains_dirt;

Uint16 SnowValley_sealevel;
const Uint16 SnowValley_sealevel_preset[5] = {2, 0, 2, 0, 0};

const float SnowValley_height_shrink_val = 0.15; 
float SnowValley_height_shrink = 0.0;

Uint8 hfunc_SnowValley(const coord3D &pos, Uint16 height, int randmod)
{
    if(pos.z == height)
        return SnowValley_snow;
    
    return SnowValley_dirt;
}

void World_Generator::generate_height_SnowValley(World &world, int ratiox, int ratioy)
{
    SnowValley_height_shrink = SnowValley_height_shrink_val*height_ratio;
    SnowValley_sealevel = SnowValley_sealevel_preset[preset_id];

    int pixelx = 0;
    int pixely = 0;

    for(int x = 0; x < world.max_block_coord.x; x++)
    {
        pixely = 0;

        for(int y = 0; y < world.max_block_coord.y; y++)
        {
            if(*abord_operations) return;

            Uint16 height = *(Uint16*)((Uint8*)HM->pixels + pixelx + pixely + 1);

            height = sqrt(height)*7*SnowValley_height_shrink;

            // int randmod = cos(x*x + y*y)*4;
            // int randmod = rand()%4;
            short randmod = *(Uint16*)((Uint8*)Pnoise->pixels + pixelx + pixely + 1);
            // randmod = randmod/32.0;

            // 255

            int z = 0;

            if(height >= world.max_block_coord.z) height = world.max_block_coord.z-1;

            for(; z < height-3+randmod%2; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = SnowValley_rock;
            }
            for(; z <= height; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = hfunc_SnowValley({x, y, z}, height, randmod);
            }
            for(; z <= SnowValley_sealevel*height_ratio; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                
                if(z == SnowValley_sealevel*height_ratio)
                    world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = water+1;
                else 
                    world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = water;
            }

            pixely += ratioy;
        }

        pixelx += ratiox;
    }
}


//////////// DARK ////////////

const Uint8 Dark_rock = 30;
const Uint8 Dark_rock2 = 31;
const Uint8 Dark_rock3 = 32;
const Uint8 Dark_dirt = 20;

Uint16 Dark_sealevel;
const Uint16 Dark_sealevel_preset[5] = {7, 8, 15, 6, 0};

const float Dark_height_shrink_val = 0.75;
float Dark_height_shrink = 0.75;

Uint8 hfunc_Dark(const coord3D &pos, Uint16 height, int randmod)
{
    coord3D p = pos;

    p.z += randmod%2;

    if(p.z <= (Dark_sealevel+1)*height_ratio && pos.z == height)
        return Dark_dirt;


    // if(p.z > (Dark_sealevel+80)*height_ratio)
    //     return Dark_rock; 

    if(p.z < height)
        return Dark_rock3;

    p.z += randmod%16;

    if(p.z > (Dark_sealevel+30)*height_ratio)
        return Dark_rock2;

    if(p.z > (Dark_sealevel+1)*height_ratio)
        return Dark_rock3;


    return Dark_rock;
}

void World_Generator::generate_height_Dark(World &world, int ratiox, int ratioy)
{
    Dark_height_shrink = Dark_height_shrink_val*height_ratio;
    Dark_sealevel = Dark_sealevel_preset[preset_id];

    int pixelx = 0;
    int pixely = 0;

    for(int x = 0; x < world.max_block_coord.x; x++)
    {
        pixely = 0;

        for(int y = 0; y < world.max_block_coord.y; y++)
        {
            if(*abord_operations) return;

            Uint16 height = *(Uint16*)((Uint8*)HM->pixels + pixelx + pixely + 1);

            height = pow(height/512.0, 4)*1024.0 + 10; 

            height = height*Dark_height_shrink;


            // int randmod = cos(x*x + y*y)*4;
            // int randmod = rand()%4;
            short randmod = *(Uint16*)((Uint8*)Pnoise->pixels + pixelx + pixely + 1);
            // randmod = randmod/32.0;

            if(randmod <= 75 && randmod >= 0 && height > 0) height -= 1;

            int z = 0;

            if(height >= world.max_block_coord.z) height = world.max_block_coord.z-1;

            for(; z <= height; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = hfunc_Dark({x, y, z}, height, randmod);
            }
            for(; z <= Dark_sealevel*height_ratio; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = water;
            }

            pixely += ratioy;
        }

        pixelx += ratiox;
    }
}

//////////// FANTASY ////////////
const Uint8 Fantasy_sand = 23;
const Uint8 Fantasy_kelp = 216; //218
const Uint8 Fantasy_kelp2 = 211;
const Uint8 Fantasy_rock = 20;
const Uint8 Fantasy_grass = 64; //color variation
const Uint8 Fantasy_grass2 = 71; //color variation
const Uint8 Fantasy_dirt = 18;
const Uint8 Fantasy_snow = 96;

Uint16 Fantasy_sealevel;
const Uint16 Fantasy_sealevel_preset[5] = {50, 20, 67, 30, 0};

const float Fantasy_height_shrink_val = 0.35;
float Fantasy_height_shrink = 0.75;

Uint8 hfunc_Fantasy(const coord3D &pos, Uint16 height, int randmod)
{
    coord3D p = pos;

    // if(height > (Fantasy_sealevel+32)*height_ratio)
    //     p.z += (float)(pos.z)/(float)(height)*30*height_ratio;

    p.z -= (2.0 + randmod%5)*height_ratio;

    // if(p.z > (Fantasy_sealevel+50)*height_ratio && pos.z > height-1)
    //     return Fantasy_snow;

    // if(pos.z > height-50 && pos.z < height-1)
    //     return Fantasy_dirt;

    if(p.z > (Fantasy_sealevel+30)*height_ratio && pos.z > height-1)
        return Fantasy_grass2;

    // p.z += randmod%32;

    if(p.z < (Fantasy_sealevel)*height_ratio)
    {
        if(randmod >= 249 && randmod <= 252)
            return Fantasy_kelp;

        return Fantasy_sand;
    }

    if(randmod >= 249 && randmod <= 252)
        return Fantasy_kelp2;

    return Fantasy_grass;
}

void World_Generator::generate_height_Fantasy(World &world, int ratiox, int ratioy)
{
    Fantasy_height_shrink = Fantasy_height_shrink_val*height_ratio;
    Fantasy_sealevel = Fantasy_sealevel_preset[preset_id];

    int pixelx = 0;
    int pixely = 0;

    for(int x = 0; x < world.max_block_coord.x; x++)
    {
        pixely = 0;

        for(int y = 0; y < world.max_block_coord.y; y++)
        {
            if(*abord_operations) return;
            
            Uint16 height = *(Uint16*)((Uint8*)HM->pixels + pixelx + pixely + 1);
            // height = 512-height;

            height = height*Fantasy_height_shrink;


            // int randmod = cos(x*x + y*y)*4;
            // int randmod = rand()%4;
            short randmod = *(Uint16*)((Uint8*)Pnoise->pixels + pixelx + pixely + 1);
            randmod = randmod;

            // 255

            int z = 0;

            if(height >= world.max_block_coord.z) height = world.max_block_coord.z-1;

            for(; z < height-(3+randmod%2); z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = Fantasy_rock;
            }
            for(; z <= height; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = hfunc_Fantasy({x, y, z}, height, randmod);
            }
            for(; z <= Fantasy_sealevel*height_ratio; z++)
            {
                block_coordonate bc = world.convert_wcoord(x, y, z);
                world.chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z].blocks[bc.x][bc.y][bc.z].id = water;
            }

            pixely += ratioy;
        }

        pixelx += ratiox;
    }
}

void World_Generator::generate_height_Flat(World &world)
{
    if(preset_id == 3)
    {
        Uint8 block1 = 26;
        Uint8 block2 = 32;

        for(int x = 0; x < world.max_chunk_coord.x; x++)
        {
            for(int y = 0; y < world.max_chunk_coord.y; y++)
            {
                if((x+y)%2)
                {
                    memset(world.chunks[x][y][0].blocks, block1, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
                    memset(world.chunks[x][y][1].blocks, block2, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
                    memset(world.chunks[x][y][2].blocks, block1, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
                }   
                else
                {
                    memset(world.chunks[x][y][0].blocks, block2, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
                    memset(world.chunks[x][y][1].blocks, block1, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
                    memset(world.chunks[x][y][2].blocks, block2, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
                }
            }
        }

        return;
    }

    Uint8 block1 = 1, block2 = 2, block3 = 3;

    switch (preset_id)
    {
    case 0 :
        block1 = 64;
        block2 = 59;
        block3 = 63;
        break;

    case 1 :
        block1 = 35;
        block2 = 34;
        block3 = 33;
        break;

    case 2 :
        block1 = 96;
        block2 = 93;
        block3 = 88;
        break;

    default:
        break;
    }

    for(int x = 0; x < world.max_chunk_coord.x; x++)
    {
        for(int y = 0; y < world.max_chunk_coord.y; y++)
        {
            memset(world.chunks[x][y][0].blocks, block1, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
            memset(world.chunks[x][y][1].blocks, block2, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
            memset(world.chunks[x][y][2].blocks, block3, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
        }
    }
}
