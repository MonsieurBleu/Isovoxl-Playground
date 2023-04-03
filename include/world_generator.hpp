#ifndef WORLD_GENERATOR_HPP
#define WORLD_GENERATOR_HPP


#include <SDL2/SDL.h>
#include <string>
#include <render_engine.hpp>

typedef Uint8 (*height_func)(Uint16);

class World_Generator
{
    private :    

        SDL_Surface *HM;

        
        Texture HMscaled;
        SDL_Surface *Pnoise;

        Shader Spass1;
        Shader Procedural_Erosion_Generator;

        void generate_height_Mountains(World &world, int ratiox, int ratioy);

        void generate_height_Plains(World &world, int ratiox, int ratioy);

        void generate_height_Canyons(World &world, int ratiox, int ratioy);

        void generate_height_Dunes(World &world, int ratiox, int ratioy);

        void generate_height_Icepeaks(World &world, int ratiox, int ratioy);

        void generate_height_SnowValley(World &world, int ratiox, int ratioy);

        void generate_height_Dark(World &world, int ratiox, int ratioy);

        void generate_height_Fantasy(World &world, int ratiox, int ratioy);

        void generate_height_Flat(World &world);

    public :
        Texture Pnoiset;
        Texture HMt;

        void init_shaders();

        void load_heightmap_from_id();
        void load_heightmap_from_file(const std::string &filename);
        void load_heightmap_from_shder(int seed = 0);

        void generate_pnoise(Uint64 seed, int w, int h);

        void generate_world_test(chunk_coordonate world_size, World &world);

        void prepare_batch_operations();

        void generate_world(World &world);

        coord3D world_size;

        int world_size_id;
        int biome_id;
        int preset_id;

        bool *abord_operations;
        int is_generating;
}; 

#endif