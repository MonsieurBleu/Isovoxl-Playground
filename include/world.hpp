#ifndef WORLD_HPP
#define WORLD_HPP

class PhysicsEngine; // forward declaration

#include <undo.hpp>
#include <light_handler.hpp>
// #include <physics.hpp>


struct World
{
    World(bool _is_main = false);
    ~World();

    void SetPhysicsEngine(PhysicsEngine* pe) {physics_engine = pe;}

    void init(uint16_t, uint16_t, uint16_t, bool debug = false);
    void free_chunks(bool debug = false);

    void reset_chunks();

    chunk_coordonate max_chunk_coord;
    chunk_coordonate min_chunk_coord;

    chunk_coordonate max_block_coord;

    chunk ***chunks;

    bool mask_mod = false;

    bool is_main;

    LightHandler lights;
    void check_light_trash_bin();

    PhysicsEngine* physics_engine;

    block* get_block(chunk_coordonate, int, int, int);
    block* get_block_wcoord(int, int, int);
    Uint16 get_block_id(chunk_coordonate, int, int, int);
    Uint16 get_block_id_wcoord(int, int, int);

    // Get the block without any wvp transformation
    Uint16 get_block_id_wcoord_nowvp(coord3D);

    Uint16 get_opaque_block_id(chunk_coordonate, int, int, int);
    
    block_coordonate convert_wcoord(int, int, int);
    world_coordonate convert_coord(block_coordonate);

    void translate_world_view_position(chunk_coordonate&, int&, int&, int&);
    void translate_world_view_wposition(int&, int&, int&);
    void translate_world_view_wpositionf(float&, float&);
    void invert_wvp(int &x, int &y);
    
    int world_view_position; // I hate you, but I also love you, but I hate you

    void compress_chunk(int, int, int);
    void compress_all_chunks();

    // get the presence of blocks in the path from the given coord to the sun/moon
    // the opaque id is given from the first bit
    // the transparent id is given from the last bit
    // like everything in the world struct, it automaticly take car of world view position transformation
    Uint32 shadow_caster_presence(world_coordonate);

    // Please consider using STHREAD_OP_SINGLE_CHUNK_POS or STHREAD_OP_SINGLE_BLOCK_VISBLE instead
    // This one does NOT take care of projection grid refresh
    // Modify the block at the given world coordonate
    // Like everything in the world struct, it automaticly take car of world view position transformation
    bool modify_block_wvp(world_coordonate, int);
    bool modify_block_wvp_bc(block_coordonate, int);
    bool modify_block(world_coordonate, int);
    bool modify_block_bc(block_coordonate coord, int id);

    int highest_nonemptychunk;
    int find_highest_nonemptychunk();

    /* load and save functions
     * returns an int that encode an error code
     * 0 being no errors
     * 
     */
    int save_to_file(const std::string& filename);
    int load_from_file(const char* filename);

    void load_chunk_save(chunk_save& cs);
    int get_compressed_chunks_in_area(const chunk_coordonate& start, const chunk_coordonate& end);
};

#endif