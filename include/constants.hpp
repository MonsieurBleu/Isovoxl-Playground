#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <iostream>
#include <txt_eng.hpp>

extern std::string NW_map_name;
#define WORLD_NAME_MAX_LENGHT 21

void startbenchrono();
void endbenchrono();

constexpr int MAX_UNDO_MEMORY_SIZE_DEFAULT = 1000000000; // 1GB

//////// WINDOW RESOLUTION ////////
#define DEFAULT_WINDOWS_W 1920
#define DEFAULT_WINDOWS_H 1080

#define RESOLUTIONS_NB 13

struct winres {uint16_t w; uint16_t h;};

const winres resolutions[RESOLUTIONS_NB+1] = {
                  {800, 600},
                  {1024, 768},
                  {1280, 720},
                  {1366, 768},
                  {1280, 1024},
                  {1440, 900},
                  {1536, 864},
                  {1600, 900},
                  {1920, 1080},
                  {2560, 1080},
                  {2560, 1440},
                  {3440, 1440},
                  {3840, 2160},
                  {5120, 2160}
                  };


//////// SCALE ////////
/*
Texture real size : 256px x 256px
Scale must be integrer or = (1/2)^(-n) to mitigate render bugs
tested floats exemple :
0.5
0.125
0.625
0.03125
0.015625
*/
#define DEFAULT_SCALE              (long double)0.03125
#define MAX_FLOAT_SCALE            (long double)8
#define MIN_FLOAT_SCALE            (long double)0.0039062 //0.0078125
#define PANORAMA_SCALE_THRESHOLD   (long double)0.03125*0.5 //0.015625

//////// GAME STATE ////////
#define STATE_QUIT            0
#define STATE_CONSTRUCTION    1
#define STATE_WORLD_SELECTION 2
#define STATE_MAIN_MENU       3
#define STATE_BLOCK_SELECTION 4
#define STATE_ADVENTURE       5
#define STATE_OPTIONS         6
#define STATE_CONTROLS        7
#define STATE_CONTROLS_SELECT 8
#define STATE_NEW_MAP         9

//////// CHUNKS ////////
#define CHUNK_SIZE 8

//////// INPUTS ////////
#define INPUT_UNUSED    0
#define INPUT_CLEAR     1
#define INPUT_REFRESH_WORLD_RENDER 2
#define INPUT_SOUND_PLAYED 4

//////// SAVE/LOAD ERROR CODES ////////
#define SAVE_ERROR_NONE 0
#define SAVE_ERROR_FILE_NOT_OPEN 1
#define SAVE_ERROR_CANNOT_UNDO 2
#define SAVE_ERROR_CANNOT_REDO 3

//////// HIGHLIGHT ////////
#define HIGHLIGHT_MOD_NONE      0
#define HIGHLIGHT_MOD_DELETE    1
#define HIGHLIGHT_MOD_REPLACE   2
#define HIGHLIGHT_MOD_PLACE     3
#define HIGHLIGHT_MOD_PLACE_ALT 4

#define HIGHLIGHT_BLOCKS      1
#define HIGHLIGHT_FLOOR       2
#define HIGHLIGHT_WALL        3
#define HIGHLIGHT_VOLUME      4
#define HIGHLIGHT_PIPETTE     5

#define HIGHLIGHT_NOCOORD  -1

//////// SHADER FEATURES ////////
#define SFEATURE_GLOBAL_ILLUMINATION 1
#define SFEATURE_AMBIANT_OCCLUSION   2
#define SFEATURE_BLOCK_BORDERS       4
#define SFEATURE_SHADOWS             8
#define SFEATURE_BLOOM               16
#define SFEATURE_GRID                32

//////// RENDER FLAGS ////////
#define SHADOW_TOP      128
#define SHADOW_LEFT      64
#define SHADOW_RIGHT     32

//////// TEXTURES ////////
#define BLOCK_TEXTURE_SIZE    32
#define MOSAIC_BLOCK_TEXTURE_SIZE 32
#define MOSAIC_TEXTURE_SIZE       512
#define MOSAIC_TOTAL_BLOCK    256
#define MOSAIC_BLOCK_PER_LINE 16
#define MOSAIC_BPL_LOG        4
#define TEXTURE_MAX_NUMBER    0b11111111
#define TEXTURE_MIN_ID        0b00000000
#define TEXTURE_BLOCK_ID      0b00000000
#define TEXTURE_BACKGROUND_ID 0b11000000

#define TEXTURE_UI_DEBUG 256

#define MOSAIC            1
#define BLOCK_NORMAL      2
#define BLOCK_AO          3
#define BLOCK_HIGHLIGHT   4
#define BLOCK_BORDER      5 
#define BLOCK_LIGHT       6
#define SHADERTEXT_WATER  7

//////// CHUNK COMPRESSION ////////
#define CHUNK_EMPTY       0b000000000
#define CHUNK_NON_UNIFORM 0b100000000

//////// BLOCK ID ////////
#define BLOCK_TRANSPARENT_LIMIT 241
#define BLOCK_LIGHT_LIMIT       225
#define BLOCK_EMPTY     0
#define BLOCK_DEBUG     1
#define BLOCK_BLUE      2
#define BLOCK_RED       3
#define BLOCK_GREEN     4
#define BLOCK_SAND      17
#define BLOCK_WATER     241

//////// BIOMES ////////
enum Biomes_ID 
{
    BIOME_PLAINS        = 0,
    BIOME_DUNES         = 1,
    BIOME_SNOWVALLEY    = 2,
    BIOME_MOUNTAINS     = 3,
    BIOME_CANYONS       = 4,
    BIOME_ICEPEAKS      = 5,
    BIOME_DARK          = 6,
    BIOME_FANTASY       = 7,
    BIOME_FLAT          = 8,
};

//////// WORLD NAME CHECK ////////
#define WORLD_NAME_OK nullptr
#define WORLD_NAME_EXIST (int*) 1
#define WORLD_NAME_EMPTY (int*) 2

//////// PHYSICS EVENT IDs ////////
#define PHYSICS_EVENT_WATER             0
#define PHYSICS_EVENT_WATER_CHECK_CHUNK 1
#define PHYSICS_EVENT_WATER_CHECK_BLOCK 2


#endif