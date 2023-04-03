#ifndef AUDIO_ENGINE_HPP
#define AUDIO_ENGINE_HPP

#include <SDL2/SDL_mixer.h> 
#include <vector>
#include <string>
#include <memory>

#define EFFECT_POSE_BLOCK              0
#define EFFECT_POSE_MULTIPLE_BLOCKS    3
#define EFFECT_DELETE_BLOCK            5
#define EFFECT_DELETE_MULTIPLE_BLOCKS  7

extern uint64_t timems;

struct IsoMusic
{
    IsoMusic();
    IsoMusic(const std::string &_name, const std::string &_autor, const std::string &extention);
    ~IsoMusic();

    void init(const std::string &_name, const std::string &_autor, const std::string &extention);

    Mix_Music *music = NULL;
    std::string name;
    std::string autor;
};

struct IsoSound
{
    ~IsoSound();

    void init(const std::string &_filename);

    Mix_Chunk *sound = NULL;
    std::string filename;
};

class Audio_Engine
{
    private :   

        std::vector<std::shared_ptr<IsoMusic>> musics;

        IsoSound voxel_modif[8];

        IsoSound click[3];

        IsoSound woosh;
        IsoSound woosh_reverse;

        Uint64 time_last_track_ended;
        Uint64 rand_delay;

        bool is_waiting_playlist = false;
        int current_music;
        
    public :
        Audio_Engine();
        ~Audio_Engine();

        void set_music_volume(float prcnt);
        void set_sound_volume(float prcnt);

        void Play_voxel_modif(int id);
        void Play_click(int id = 0);
        void Play_woosh(bool reverse = false);       

        void handle_playlist(); 

        void start_playlist();

        const IsoMusic& get_current_music()
        {
            return *musics[current_music];
        }
        const bool is_playing_music()
        {
            return !is_waiting_playlist;
        }
};

#endif