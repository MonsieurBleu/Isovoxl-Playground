#include <game.hpp>

World::World(bool _is_main)
{
    chunks = NULL;
    is_main = _is_main;

    if(is_main)
        init(256, 256, 75, true);
}

typedef chunk CHUNK;

void World::init(uint16_t _x, uint16_t _y, uint16_t _z, bool debug)
{
    // std::cout << "World::init : " << _x << " " << _y << " " << _z << "\n";
    max_chunk_coord = {_x-1, _y-1, _z-1};
    min_chunk_coord = {0, 0, 0};

    max_block_coord = {_x*CHUNK_SIZE, _y*CHUNK_SIZE, _z*CHUNK_SIZE};

    if(!debug && is_main) return;

    chunks = new CHUNK**[_x];

    memset(lights.buff, 0, sizeof(light_block)*MAX_LIGHT_NUMBER);

    for(int x = 0; x < _x ; x++)
    {
        chunks[x] = new CHUNK*[_y];

        for(int y = 0; y < _y ; y++)
        {
            chunks[x][y] = new CHUNK[_z];

            for(int z = 0; z < _z ;z++)
            {
                chunks[x][y][z] = {0};
            }
        }
    }
}

World::~World()
{
    free_chunks(true);
}

void World::free_chunks(bool debug)
{
    if(!debug && is_main) return;

    if(!chunks)
        return;

    for(int x = 0; x <= max_chunk_coord.x; x++)
    {
        for(int y = 0; y <= max_chunk_coord.y; y++)
        {
            delete [] chunks[x][y];
        }

        delete [] chunks[x];
    }

    delete [] chunks;

    chunks = NULL;
}

void World::reset_chunks()
{
    for(int x = 0; x <= max_chunk_coord.x; x++)
    {
        for(int y = 0; y <= max_chunk_coord.y ; y++)
        {
            memset(chunks[x][y], 0, sizeof(chunk)*(max_chunk_coord.z+1));
        }
    }
}

Uint16 World::get_block_id(chunk_coordonate coord, int x, int y, int z)
{
    block *b = get_block(coord, x, y ,z);

    if(!b)
        return BLOCK_EMPTY;
    
    return b->id;
}

Uint16 World::get_block_id_wcoord(int x, int y, int z)
{
    block *b = get_block_wcoord(x, y, z);

    if(!b)
        return BLOCK_EMPTY;
    
    return b->id;
}

block* World::get_block(chunk_coordonate coord, int x, int y, int z)
{
    translate_world_view_position(coord, x, y, z);

    while(x < 0){x += CHUNK_SIZE; coord.x --;}

    while(x >= CHUNK_SIZE){x -= CHUNK_SIZE; coord.x ++;}

    while(y < 0){y += CHUNK_SIZE; coord.y --;}

    while(y >= CHUNK_SIZE){y -= CHUNK_SIZE; coord.y ++;}

    while(z < 0){z += CHUNK_SIZE; coord.z --;}

    while(z >= CHUNK_SIZE){z -= CHUNK_SIZE; coord.z ++;}

    if(coord.x < min_chunk_coord.x || coord.x > max_chunk_coord.x ||
       coord.y < min_chunk_coord.y || coord.y > max_chunk_coord.y ||
       coord.z < min_chunk_coord.z || coord.z > max_chunk_coord.z)
        return NULL;

    if(x < 0 || x >= CHUNK_SIZE || 
       y < 0 || y >= CHUNK_SIZE || 
       z < 0 || z >= CHUNK_SIZE)
        return NULL;
    
    return &chunks[coord.x][coord.y][coord.z].blocks[x][y][z];
}

block* World::get_block_wcoord(int x, int y, int z)
{
    translate_world_view_wposition(x, y, z);

    if(x < 0 || y < 0 || z < 0) return NULL;

    chunk_coordonate coord = {x / CHUNK_SIZE,
                              y / CHUNK_SIZE,
                              z / CHUNK_SIZE};
    
    x %= CHUNK_SIZE;
    y %= CHUNK_SIZE;
    z %= CHUNK_SIZE;

    if(coord.x < min_chunk_coord.x || coord.x > max_chunk_coord.x ||
       coord.y < min_chunk_coord.y || coord.y > max_chunk_coord.y ||
       coord.z < min_chunk_coord.z || coord.z > max_chunk_coord.z)
        return NULL;

    return &chunks[coord.x][coord.y][coord.z].blocks[x][y][z];
}

Uint16 World::get_block_id_wcoord_nowvp(coord3D wcoord)
{
    chunk_coordonate coord = {wcoord.x / CHUNK_SIZE,
                              wcoord.y / CHUNK_SIZE,
                              wcoord.z / CHUNK_SIZE};
    
    wcoord.x %= CHUNK_SIZE;
    wcoord.y %= CHUNK_SIZE;
    wcoord.z %= CHUNK_SIZE;

    if(coord.x < min_chunk_coord.x || coord.x > max_chunk_coord.x ||
       coord.y < min_chunk_coord.y || coord.y > max_chunk_coord.y ||
       coord.z < min_chunk_coord.z || coord.z > max_chunk_coord.z)
        return BLOCK_EMPTY;

    return chunks[coord.x][coord.y][coord.z].blocks[wcoord.x][wcoord.y][wcoord.z].id;
}

Uint16 World::get_opaque_block_id(chunk_coordonate coord, int x, int y, int z)
{
    block *b = get_block(coord, x, y ,z);

    if(!b || b->id >= BLOCK_TRANSPARENT_LIMIT)
        return BLOCK_EMPTY;
    
    return b->id;
}

block_coordonate World::convert_wcoord(int x, int y, int z)
{
    block_coordonate bc;

    bc.x = x%CHUNK_SIZE;
    bc.y = y%CHUNK_SIZE;
    bc.z = z%CHUNK_SIZE;

    bc.chunk.x = x/CHUNK_SIZE;
    bc.chunk.y = y/CHUNK_SIZE;
    bc.chunk.z = z/CHUNK_SIZE;

    return bc;
}

world_coordonate World::convert_coord(block_coordonate bc)
{
    world_coordonate coord;

    coord.x = bc.x + bc.chunk.x*CHUNK_SIZE;
    coord.y = bc.y + bc.chunk.y*CHUNK_SIZE;
    coord.z = bc.z + bc.chunk.z*CHUNK_SIZE;

    if(coord.x > max_block_coord.x)
    {
        std::cout << "\n" << bc.chunk.x << " " << bc.x << " " << max_chunk_coord.x;
    }

    return coord;
}

void World::translate_world_view_position(chunk_coordonate& coord, int& x, int& y, int& z)
{
    switch (world_view_position)
    {
    case 1 :
    {
        int ytmp = y;
        int cytmp = coord.y;

        y = x;
        x = ytmp;

        coord.y = coord.x;
        coord.x = cytmp;

        coord.y = max_chunk_coord.y-coord.y;
        y = CHUNK_SIZE-1-y;
    }


        break;
    
    case 3 :
    {
        int ytmp = y;
        int cytmp = coord.y;

        y = x;
        x = ytmp;

        coord.y = coord.x;
        coord.x = cytmp;

        coord.x = max_chunk_coord.x-coord.x;
        x = CHUNK_SIZE-1-x;
        break;
    }

    case 2 :
        coord.y = max_chunk_coord.y-coord.y;
        y = CHUNK_SIZE-1-y;
    
        coord.x = max_chunk_coord.x-coord.x;
        x = CHUNK_SIZE-1-x;
        break;

    default:
        break;
    }
}

void World::translate_world_view_wposition(int& x, int& y, int& z)
{
    switch (world_view_position)
    {
    case 1 :
    {
        int ytmp = y;
        y = x;
        x = ytmp;
        y = max_block_coord.y-1-y;
    }
        break;
    
    case 3 : 
    {
        int ytmp = y;
        y = x;
        x = ytmp;
        x = max_block_coord.x-1-x;
        break;
    }

    case 2 :
        y = max_block_coord.y-1-y;
        x = max_block_coord.x-1-x;
        break;

    default:
        break;
    }
}

void World::invert_wvp(int &x, int &y)
{
    switch (world_view_position)
    {
    case 1 :
    {
        y = max_block_coord.y-1-y;

        int ytmp = y;

        y = x;
        x = ytmp;
    }
        break;
    
    case 3 : 
    {
        x = max_block_coord.x-1-x;

        int ytmp = y;

        y = x;
        x = ytmp;
        break;
    }

    case 2 : 
        y = max_block_coord.y-1-y;
        x = max_block_coord.x-1-x;
        break;
    

    default:
        break;
    } 
}

void World::translate_world_view_wpositionf(float& x, float& y)
{
    if(world_view_position == 1)
    {
        float ytmp = y;

        y = x;
        x = ytmp;

        y = max_block_coord.y-1.0-y;
    }
    else if(world_view_position == 3)
    {
        float ytmp = y;

        y = x;
        x = ytmp;

        x = max_block_coord.x-1.0-x;
    }
    else if(world_view_position == 2)
    {
        y = max_block_coord.y-1.0-y;
        x = max_block_coord.x-1.0-x;
    }
}

void World::compress_chunk(int x, int y, int z)
{
    // struct chunk* c = &chunk[x][y][z];

    // c->compress_value = CHUNK_NON_UNIFORM;

    // int id = c->block[0][0][0].id;

    // for(int i = 1; i < CHUNK_SIZE; i++)
    //     if(c->block[0][0][i].id != id)
    //         return;

    // Uint64 *c2 = (Uint64*)(&c->block);

    // int size = (CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE)/8;

    // for(int i = 0; i < size; i++)
    //     if(c2[0] != c2[i])
    //         return;
    
    // c->compress_value = id;

    struct chunk* c = &chunks[x][y][z];
    c->compress_value = CHUNK_NON_UNIFORM;
    Uint8 val = c->blocks[0][0][0].id;

    for(int i = 0; i < CHUNK_SIZE; i++)
        for(int j = 0; j < CHUNK_SIZE; j++)
            for(int k = 0; k < CHUNK_SIZE; k++)
                if(c->blocks[i][j][k].id != val)
                    return;
    
    c->compress_value = val;

    // std::cout << "\nCHUNK COMPRESSED AT COORD " << x << " " << y << " " << z << " ";
    // std::cout << "\tWITH ID " << id;
}

void World::compress_all_chunks()
{
    for(int x = 0; x <= max_chunk_coord.x; x++)
    for(int y = 0; y <= max_chunk_coord.y; y++)
    for(int z = 0; z <= max_chunk_coord.z; z++)
    {
        compress_chunk(x, y, z);
    }
}

Uint32 World::shadow_caster_presence(world_coordonate start)
{
    // No valid coord checking
    // Positions arent output
    // Both opaque and transparent blocks are checked, but the function ends when an opaque is found
    // Jumps are positive

    if(start.x > max_block_coord.x || start.y > max_block_coord.y || start.z > max_block_coord.z)
        return 0;

    if(start.x < 0 || start.y < 0 || start.z < 0)
        return 0;

    struct chunk* c;
    block_coordonate bc = convert_wcoord(start.x, start.y, start.z);
    Uint16 oid = BLOCK_EMPTY;
    Uint16 tid = BLOCK_EMPTY;
    bool chunk_end = false;

    if(bc.chunk.y > max_chunk_coord.y)
        return 0;

    while(bc.chunk.x <= max_chunk_coord.x && bc.chunk.z <= max_chunk_coord.z)
    {
        c = &chunks[bc.chunk.x][bc.chunk.y][bc.chunk.z];

        // std::cout << "\nboucle 1" << bc.chunk.x << " " << max_chunk_coord.x << " " << c->compress_value;

        if(c->compress_value == CHUNK_NON_UNIFORM)
        {
            chunk_end = false;

            while(!chunk_end)
            {
                oid = c->blocks[bc.x][bc.y][bc.z].id;

                if(oid >= BLOCK_TRANSPARENT_LIMIT)
                {
                    tid = oid;
                }
                else if(oid != BLOCK_EMPTY)
                {
                    // std::cout << "\n\t yo1 " << c->compress_value;
                    return oid + (tid<<8);
                }

                bc.x += 1;
                bc.z += 1;

                if(bc.x == CHUNK_SIZE)
                {
                    bc.chunk.x ++;
                    bc.x = 0;
                    chunk_end = true;
                }
                if(bc.z == CHUNK_SIZE)
                {
                    bc.chunk.z ++;
                    bc.z = 0;
                    chunk_end = true;
                }
            }
        }
        else if(c->compress_value != BLOCK_EMPTY && c->compress_value < BLOCK_TRANSPARENT_LIMIT)
        {
            return c->compress_value + (tid<<8);
        }
        else
        {
            if(c->compress_value != BLOCK_EMPTY)
                tid = c->compress_value;
            
            int max = bc.x > bc.z ? bc.x : bc.z;
            max = CHUNK_SIZE-max;

            bc.x += max;
            bc.z += max;

            if(bc.x == CHUNK_SIZE)
            {
                bc.chunk.x ++;
                bc.x = 0;
            }
            if(bc.z == CHUNK_SIZE)
            {
                bc.chunk.z ++;
                bc.z = 0;
            }

        }

    }

    return tid<<8;
}

bool World::modify_block_wvp(world_coordonate wcoord, int id)
{
    translate_world_view_wposition(wcoord.x, wcoord.y, wcoord.z);

    return modify_block(wcoord, id);
}

bool World::modify_block_wvp_bc(block_coordonate coord, int id)
{
    translate_world_view_position(coord.chunk, coord.x, coord.y, coord.z);

    return modify_block_bc(coord, id);
}

bool World::modify_block(world_coordonate wcoord, int id)
{
    block_coordonate coord = convert_wcoord(wcoord.x, wcoord.y, wcoord.z);

    return modify_block_bc(coord, id);
}

bool World::modify_block_bc(block_coordonate coord, int id)
{

    if(coord.x < min_chunk_coord.x || coord.x > max_chunk_coord.x ||
       coord.y < min_chunk_coord.y || coord.y > max_chunk_coord.y ||
       coord.z < min_chunk_coord.z || coord.z > max_chunk_coord.z)
        return false;
    
    Uint8 &cid = chunks[coord.chunk.x][coord.chunk.y][coord.chunk.z].blocks[coord.x][coord.y][coord.z].id;

    if(mask_mod && cid && id) return false;
    
    cid = id;

    if (id == BLOCK_EMPTY) {
        physics_engine->add_event(PHYSICS_EVENT_WATER_CHECK_CHUNK, &coord.chunk);
    }

    // if (id == BLOCK_WATER) {
    //     physics_engine->add_event(PHYSICS_EVENT_WATER_CHECK_BLOCK, &wcoord);
    // }

    compress_chunk(coord.chunk.x, coord.chunk.y, coord.chunk.z);

    if(id < BLOCK_TRANSPARENT_LIMIT && id > BLOCK_LIGHT_LIMIT)
        lights.add({(Uint8)id, coord});

    return true;
}

int World::find_highest_nonemptychunk()
{
    for(int z = max_chunk_coord.z; z > 0; z--)
    for(int x = 0; x <= max_chunk_coord.x; x++)
    for(int y = 0; y <= max_chunk_coord.y; y++)
    {
        if(chunks[x][y][z].compress_value != CHUNK_EMPTY)
        {
            highest_nonemptychunk = z;
            return z;
        }
    }
    highest_nonemptychunk = 1;
    return 1;
}

void World::load_chunk_save(chunk_save& cs) {

    chunk_coordonate corner1 = cs.get_smallest_chunk_coord_sum();
    chunk_coordonate corner2 = cs.get_highest_chunk_coord_sum();

    int compressed_chunks_n = get_compressed_chunks_in_area(corner1, corner2);

    int chunks_n = cs.size + cs.csize;

    int uncompressed_chunks_n = chunks_n - compressed_chunks_n;

    chunk_save new_cs(uncompressed_chunks_n, compressed_chunks_n);

    int compressed_offset = 0;
    int uncompressed_offset = 0;

    chunk* chunk_pointer; // points to the chunk currently being processed in the world


    for (int i = 0; i < cs.size; i++) {
        chunk_pointer = &chunks[cs.chunks[i].coord.x][cs.chunks[i].coord.y][cs.chunks[i].coord.z];

        if (chunk_pointer->compress_value == CHUNK_NON_UNIFORM) {
            memcpy(&new_cs.chunks[compressed_offset].blocks, chunk_pointer->blocks, sizeof(chunk));
            new_cs.chunks[compressed_offset].compress_value = CHUNK_NON_UNIFORM;
            new_cs.chunks[compressed_offset].coord = cs.chunks[i].coord;
            compressed_offset++;
        }
        else {
            new_cs.cchunks[uncompressed_offset].compress_value = chunk_pointer->compress_value;
            new_cs.cchunks[uncompressed_offset].coord = cs.chunks[i].coord;
            uncompressed_offset++;
        } 
        
        memcpy(chunk_pointer->blocks, cs.chunks[i].blocks, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
        chunk_pointer->compress_value = cs.chunks[i].compress_value;
    }

    for (int i = 0; i < cs.csize; i++) {
        chunk_pointer = &chunks[cs.cchunks[i].coord.x][cs.cchunks[i].coord.y][cs.cchunks[i].coord.z];

        if (chunk_pointer->compress_value == CHUNK_NON_UNIFORM) {
            memcpy(&new_cs.chunks[compressed_offset].blocks, chunk_pointer->blocks, sizeof(chunk));
            new_cs.chunks[compressed_offset].compress_value = CHUNK_NON_UNIFORM;
            new_cs.chunks[compressed_offset].coord = cs.cchunks[i].coord;
            compressed_offset++;
        }
        else {
            new_cs.cchunks[uncompressed_offset].compress_value = chunk_pointer->compress_value;
            new_cs.cchunks[uncompressed_offset].coord = cs.cchunks[i].coord;
            uncompressed_offset++;
        } 

        chunk_pointer->compress_value = cs.cchunks[i].compress_value;
        memset(chunk_pointer->blocks, cs.cchunks[i].compress_value, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE);
    }

    cs.apply(new_cs);


    // return new_cs; // fuck pointers (╬ Ò﹏Ó)
}

// t'as dit que tu voulait bouger cette ftc autre part donc si tu veut have fun
int World::get_compressed_chunks_in_area(const chunk_coordonate& start, const chunk_coordonate& end){
    int count = 0;
    
    for (int x = start.x; x <= end.x; x++)
    {
        for (int y = start.y; y <= end.y; y++)
        {
            for (int z = start.z; z <= end.z; z++)
            {
                if (chunks[x][y][z].compress_value != CHUNK_NON_UNIFORM)
                    count++;
            }
        }
    }

    return count;
}

int World::save_to_file(const std::string& filename) {
    FILE* file = fopen(filename.c_str(), "wb");
    if (!file) {
        std::cout << "Error opening file " << filename << "\n";
        return SAVE_ERROR_FILE_NOT_OPEN;
    }

    chunk_coordonate tmp;
    tmp.x = max_chunk_coord.x + 1;
    tmp.y = max_chunk_coord.y + 1;
    tmp.z = max_chunk_coord.z + 1;

    fwrite(&(tmp.x), sizeof(chunk_coordonate::x), 1, file);
    fwrite(&(tmp.y), sizeof(chunk_coordonate::y), 1, file);
    fwrite(&(tmp.z), sizeof(chunk_coordonate::z), 1, file);

    // std::cout << "max_chunk_coord.x = " << tmp.x << std::endl;
    // std::cout << "max_chunk_coord.y = " << tmp.y << std::endl;
    // std::cout << "max_chunk_coord.z = " << tmp.z << std::endl;

    const int number_of_chunks_allocated = 4096;
    const int chunk_size = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
    const int alloc_size = chunk_size * number_of_chunks_allocated;

    Uint8 *buffer = new Uint8[alloc_size];
    int buffer_index = 0;

    for (int x = 0; x < tmp.x; x++) {
        for (int y = 0; y < tmp.y; y++) {
            for (int z = 0; z < tmp.z; z++) {
                if (buffer_index + chunk_size > alloc_size) {
                    fwrite(buffer, 1, buffer_index, file);
                    buffer_index = 0;
                }

                switch (chunks[x][y][z].compress_value) {
                    case CHUNK_EMPTY: {// chunk is empty so we write a 0
                        buffer[buffer_index++] = '\x00';
                        // printf("wrote: %02x\n", empty);
                        break;
                    }

                    case CHUNK_NON_UNIFORM: {// chunk is not uniform so we write a 2 and we use "run length encoding" (RLE) to write the blocks ids
                        // RLE
                        buffer[buffer_index++] = '\x02';
                        Uint8 current_id = chunks[x][y][z].blocks[0][0][0].id;
                        Uint16 current_count = 0;
                        // std::cout << "current_count = " << current_count << std::endl;
                        // std::cout << "current_id = " << (int)chunk[x][y][z].block[0][0][0].id << std::endl;
                        // std::cout << "x y z: " << x << ' ' << y << ' ' << z << std::endl;
                        for (int i = 0; i < CHUNK_SIZE; i++) {
                            for (int j = 0; j < CHUNK_SIZE; j++) {
                                for (int k = 0; k < CHUNK_SIZE; k++) {
                                    if (chunks[x][y][z].blocks[i][j][k].id == current_id) {
                                        current_count++;
                                    }
                                    else {
                                        buffer[buffer_index++] = current_count & 0xff;
                                        buffer[buffer_index++] = current_count >> 8;
                                        buffer[buffer_index++] = current_id;

                                        current_id = chunks[x][y][z].blocks[i][j][k].id;
                                        current_count = 1;
                                    }
                                }
                            }
                        }
                        buffer[buffer_index++] = current_count & 0xff;
                        buffer[buffer_index++] = current_count >> 8;
                        buffer[buffer_index++] = current_id;

                        // std::cout << "current_count = " << current_count << std::endl;
                        // std::cout << "current_id = " << (int)current_id << std::endl;

                        // printf("wrote: %02x %02x\n", current_count >> 8, current_count & 0xff);
                        // printf("wrote: %02x\n", current_id);
                        break;
                    }

                    default: {// chunk is full of the same block so we write a 1 and the block id
                        buffer[buffer_index++] = '\x01';
                        buffer[buffer_index++] = chunks[x][y][z].blocks[0][0][0].id;
                        // printf("wrote: %02x\n", full);
                        // printf("wrote: %02x\n", chunk[x][y][z].block[0][0][0].id);
                        break;   
                    }
                }
            }
        }
    }
    fwrite(buffer, 1, buffer_index, file);

    fwrite(lights.buff, sizeof(light_block), MAX_LIGHT_NUMBER, file);

    delete[] buffer;
    fclose(file);
    return SAVE_ERROR_NONE;

}

int get_file_size(FILE* f) {
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);
    return size;
}

int World::load_from_file(const char* filename) {
    // std::cout << "Loading world from file " << filename << "\n";
    FILE* file = fopen(filename, "rb");
    if (!file) {
        std::cout << "Error opening file " << filename << "\n";
        return SAVE_ERROR_FILE_NOT_OPEN;
    }

    int file_size = get_file_size(file);

    chunk_coordonate new_size;

    fread(&new_size, sizeof(chunk_coordonate), 1, file);

    // std::cout << "max_chunk_coord.x = " << new_size.x << std::endl;
    // std::cout << "max_chunk_coord.y = " << new_size.y << std::endl;
    // std::cout << "max_chunk_coord.z = " << new_size.z << std::endl;

    physics_engine->world_mutex.lock();

    if(!chunks || new_size.x != max_chunk_coord.x+1 || new_size.y != max_chunk_coord.y+1 || new_size.z != max_chunk_coord.z)
    {
        free_chunks();
        init(new_size.x, new_size.y, new_size.z);
    }

    unsigned int read_size = file_size - sizeof(chunk_coordonate);
    unsigned int byte_offset = 0;

    Uint8 *buffer = new Uint8[read_size];

    fread(buffer, 1, read_size, file);


    for (int x = 0; x < new_size.x; x++) {
        for (int y = 0; y < new_size.y; y++) {
            for (int z = 0; z < new_size.z; z++) {
                Uint8 compress_value = buffer[byte_offset++];
                
                switch (compress_value) {
                    case 0: // chunk is empty
                        chunks[x][y][z].compress_value = CHUNK_EMPTY;
                        memset(chunks[x][y][z].blocks, 0, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(block));
                        break;

                    case 1: // chunk is full of the same block
                        Uint8 id;
                        id = buffer[byte_offset++];
                        memset(chunks[x][y][z].blocks, id, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(block));
                        chunks[x][y][z].compress_value = id;
                        break;   

                    case 2: // chunk is not uniform
                        // RLE
                        chunks[x][y][z].compress_value = CHUNK_NON_UNIFORM;
                        Uint8 current_id = 0;
                        Uint16 current_count = 0;
                        for (int i = 0; i < CHUNK_SIZE; i++) {
                            for (int j = 0; j < CHUNK_SIZE; j++) {
                                for (int k = 0; k < CHUNK_SIZE; k++) {
                                    if (current_count == 0) {

                                        union {
                                            Uint16 value;
                                            Uint8 bytes[2];
                                        } count;
                                        count.bytes[0] = buffer[byte_offset++];
                                        count.bytes[1] = buffer[byte_offset++];

                                        current_count = count.value;
                                        current_id = buffer[byte_offset++];
                                    }
                                    chunks[x][y][z].blocks[i][j][k].id = current_id;
                                    current_count--;
                                }
                            }
                        }
                        break;

                }
            }
        }
    }

    if(byte_offset < read_size)
    {
        int light_read = read_size-byte_offset;

        set_in_interval(light_read, 0, MAX_LIGHT_NUMBER*sizeof(light_block));

        memcpy(lights.buff, buffer+byte_offset, light_read);
    }
    else
        memset(lights.buff, 0, sizeof(light_block)*MAX_LIGHT_NUMBER);
        
    physics_engine->world_mutex.unlock();

    delete[] buffer;
    fclose(file);
    return SAVE_ERROR_NONE;
}