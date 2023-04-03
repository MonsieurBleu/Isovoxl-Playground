#include <audio_engine.hpp>
#include <iostream>
#include <SDL2/SDL.h>




bool music_finished = false;

void Play_next_music()
{
    music_finished = true;
}

IsoMusic::IsoMusic(const std::string &_name, const std::string &_autor, const std::string &extention)
{
    init(_name, _autor, extention);
}

IsoMusic::~IsoMusic()
{
    Mix_FreeMusic(music);
}

void IsoMusic::init(const std::string &_name, const std::string &_autor, const std::string &extention)
{
    name = _name;
    autor = _autor;

    music = Mix_LoadMUS(("ressources/musics/" + name + " by " + autor + "." + extention).c_str());
    if(!music)
        std::cout << name << " ERROR : " << Mix_GetError() << "\n";
}

void IsoSound::init(const std::string &_filename)
{
    filename = _filename;
    sound = Mix_LoadWAV(filename.c_str());

    if(!sound)
        std::cout << filename << " ERROR : " << Mix_GetError() << "\n";
}

IsoSound::~IsoSound()
{
    if(sound)
        Mix_FreeChunk(sound);
}

Audio_Engine::Audio_Engine()
{
    // if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1) //Initialisation de l'API Mixer
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1) //Initialisation de l'API Mixer
    {
        std::cout << Mix_GetError() << "\n";
    }

    musics.push_back(std::make_shared<IsoMusic>("Winter Story", "Aila Scott", "ogg"));
    musics.push_back(std::make_shared<IsoMusic>("Clouded Thoughts", "Wakapedia", "ogg"));
    musics.push_back(std::make_shared<IsoMusic>("Endless Space", "GeorgeTantchev", "ogg"));
    musics.push_back(std::make_shared<IsoMusic>("Repose", "Robotmeadows", "ogg"));
    musics.push_back(std::make_shared<IsoMusic>("Reunion", "Robotmeadows", "ogg"));
    musics.push_back(std::make_shared<IsoMusic>("Reflexive Loop", "OhRakuda", "ogg"));

    current_music = time(NULL)%musics.size();
    Mix_HookMusicFinished(Play_next_music);
    set_music_volume(50);

    // voxel_modif[0].init("ressources/sounds/BUTTON_05.ogg");
    // voxel_modif[1].init("ressources/sounds/BUTTON_12.ogg");


    // voxel_modif[0].init("ressources/sounds/Laptop_Spacebar_06.ogg");
    // voxel_modif[1].init("ressources/sounds/Laptop_Keystroke_82.ogg");
    // voxel_modif[2].init("ressources/sounds/Wallet Close.ogg");
    voxel_modif[0].init("ressources/sounds/08_Step_rock_02.ogg");  // https://leohpaz.itch.io/90-retro-player-movement-sfx
    voxel_modif[1].init("ressources/sounds/12_Step_wood_03.ogg");  // https://leohpaz.itch.io/90-retro-player-movement-sfx
    // voxel_modif[2].init("ressources/sounds/03_Step_grass_03.ogg"); // https://leohpaz.itch.io/90-retro-player-movement-sfx

    voxel_modif[3].init("ressources/sounds/45_Landing_01.ogg"); // https://leohpaz.itch.io/90-retro-player-movement-sfx

    //voxel_modif[3].init("ressources/sounds/PC Keyboard_Spacebar_06.ogg");  // https://shapeforms.itch.io/shapeforms-audio-free-sfx
    //voxel_modif[4].init("ressources/sounds/PC Keyboard_Keystroke_28.ogg"); // https://shapeforms.itch.io/shapeforms-audio-free-sfx

    voxel_modif[5].init("ressources/sounds/Wood Block1.ogg"); // https://ellr.itch.io/universal-ui-soundpack
    voxel_modif[6].init("ressources/sounds/Wood Block2.ogg"); // https://ellr.itch.io/universal-ui-soundpack
    voxel_modif[7].init("ressources/sounds/Wood Block3.ogg"); // https://ellr.itch.io/universal-ui-soundpack

    click[0].init("ressources/sounds/Minimalist11.ogg");
    click[1].init("ressources/sounds/Minimalist10.ogg"); //select hl_type 
    click[2].init("ressources/sounds/Minimalist1.ogg");  //select block
 
    woosh.init("ressources/sounds/03_Step_grass_03.ogg"); 
    woosh_reverse.init("ressources/sounds/03_Step_grass_03_reverse.ogg"); 
}

Audio_Engine::~Audio_Engine()
{
    Mix_CloseAudio();
}

void Audio_Engine::set_music_volume(float prcnt)
{
    Mix_VolumeMusic(MIX_MAX_VOLUME*prcnt/100.0);
}

void Audio_Engine::set_sound_volume(float prcnt)
{
    Mix_Volume(-1, MIX_MAX_VOLUME*prcnt/100.0);
}

void Audio_Engine::Play_voxel_modif(int id)
{
    // if(id == 0) id += rand()%3;
    if(id == EFFECT_POSE_BLOCK) 
        id += rand()%2;

    // else 
    // if(id == EFFECT_POSE_MULTIPLE_BLOCKS) 
    //     id += rand()%2;

    else 
    if(id == EFFECT_DELETE_BLOCK) 
        id += rand()%2;


    Mix_PlayChannel(-1, voxel_modif[id].sound, 0);
}

void Audio_Engine::Play_click(int id)
{
    Mix_PlayChannel(-1, click[id].sound, 0);
};

void Audio_Engine::Play_woosh(bool reverse)
{
    if(reverse)
        Mix_PlayChannel(-1, woosh_reverse.sound, 0);
    else 
        Mix_PlayChannel(-1, woosh.sound, 0);
}

void Audio_Engine::start_playlist()
{
    Mix_PlayMusic(musics[current_music]->music, 0);
}

void Audio_Engine::handle_playlist()
{
    if(music_finished)
    {
        time_last_track_ended = timems;
        rand_delay = 1000*(rand()%20 + 10);
        music_finished = false;
        is_waiting_playlist = true;
    }

    if(is_waiting_playlist)
    {
        if(timems-time_last_track_ended >= rand_delay)
        {
            int new_music = 0;
            do
            {
                new_music = rand()%musics.size();
                // std::cout << "trying " << new_music << "\n";
            }
            while(new_music == current_music);

            current_music = new_music;

            Mix_PlayMusic(musics[current_music]->music, 0);

            is_waiting_playlist = false;
        }
    }
}