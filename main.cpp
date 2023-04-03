#include <game.hpp>

Uint64 timems = 0;
Uint64 timems_start = 0;

//find -iname desktop.ini -delete

void print_features()
{
    GPU_RendererID test[32];
    GPU_GetRegisteredRendererList(test);

    for(int i = 0; i < GPU_GetNumRegisteredRenderers(); i++)
    {
        std::cout << test[i].name << '\n';
    }
    std::cout << GPU_GetCurrentRenderer()->id.name << '\n';

    std::cout << "vsync : " << GPU_IsFeatureEnabled(GPU_INIT_ENABLE_VSYNC) << "\n";    
    std::cout << "double buff : " << !GPU_IsFeatureEnabled(GPU_INIT_DISABLE_DOUBLE_BUFFER) << "\n";    
    std::cout << "geometry shaders : " << GPU_IsFeatureEnabled(GPU_FEATURE_ALL_SHADERS) << "\n";

}

int main(int argc, char *argv[])
{
    timems_start = Get_time_ms();

    srand(timems);

    system("cls");

    int statut = EXIT_FAILURE;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    GPU_Target *screen =  GPU_InitRenderer(GPU_RENDERER_OPENGL_4, 
                                           800,
                                           600,
                                           GPU_DEFAULT_INIT_FLAGS);

    GPU_SetDebugLevel(GPU_DEBUG_LEVEL_MAX);

    GPU_FeatureEnum test = GPU_INIT_DISABLE_VSYNC;

    GPU_SetRequiredFeatures(test);

    SDL_SetWindowTitle(SDL_GL_GetCurrentWindow(), "Isovoxl | Playground");

    for(int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if(arg == "-transrights")
        {
            timems_start -= timems_start%128;
        }
    }

    if(screen)
    {
        Game Iso(screen);
        statut = Iso.mainloop();
    }
    
    GPU_Quit();
    SDL_Quit();
    
    std::cout << "Game closed without error.\n";

    return statut;
}