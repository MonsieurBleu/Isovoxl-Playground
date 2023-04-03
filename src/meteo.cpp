#include <meteo.hpp>

meteo meteos[METEO_COUNTER];

extern unsigned long long int timems;

void init_meteos()
{
    for(int id = 0; id < METEO_COUNTER; id++)
    {
        if(meteos[id].background_shader != nullptr)
        {
            meteos[id].background_shader = NULL;
        }
        switch (id)
        {
        case METEO_MAIN_MENU :
            meteos[id].global_illumination[0] = 0.85;
            meteos[id].global_illumination[1] = 0.85;
            meteos[id].global_illumination[2] = 0.85;
            meteos[id].global_illumination[3] = 1.0;

            meteos[id].background_shader = std::make_shared<Shader>();
            meteos[id].background_shader->load(
            "shader/background/background.vert",
            "shader/background/main_menu.frag",
            NULL);
            
            meteos[id].add_data = timems%512;
            break;

        case METEO_ANIMATED_SKY :
            meteos[id].global_illumination[0] = 1.0;
            meteos[id].global_illumination[1] = 1.0;
            meteos[id].global_illumination[2] = 1.0;
            meteos[id].global_illumination[3] = 1.0;

            meteos[id].background_shader = std::make_shared<Shader>();
            meteos[id].background_shader->load(
            "shader/background/background.vert",
            "shader/background/animated_sky.frag",
            NULL);

            meteos[id].add_data = 0;
            break;
        
        case METEO_ANIMATED_DUSK :
            meteos[id].global_illumination[0] = 0.80;
            meteos[id].global_illumination[1] = 0.75;
            meteos[id].global_illumination[2] = 0.60;
            meteos[id].global_illumination[3] = 1.0;

            meteos[id].background_shader = meteos[METEO_ANIMATED_SKY].background_shader;

            meteos[id].add_data = 1;
            break;

        case METEO_NEBULA :
            meteos[id].global_illumination[0] = 0.5;
            meteos[id].global_illumination[1] = 0.4;
            meteos[id].global_illumination[2] = 0.5;
            meteos[id].global_illumination[3] = 0.4;

            meteos[id].background_shader = std::make_shared<Shader>();
            meteos[id].background_shader->load(
            "shader/background/background.vert",
            "shader/background/aurora_night.frag",
            NULL);

            meteos[id].add_data = 128;
            break;
            
        case METEO_AZUR_AURORA :
            meteos[id].global_illumination[0] = 0.5;
            meteos[id].global_illumination[1] = 0.5;
            meteos[id].global_illumination[2] = 0.6;
            meteos[id].global_illumination[3] = 0.4;

            meteos[id].background_shader = meteos[METEO_NEBULA].background_shader;

            meteos[id].add_data = 0;
            break;

        case METEO_ORCHID_AURORA :
            meteos[id].global_illumination[0] = 0.6;
            meteos[id].global_illumination[1] = 0.4;
            meteos[id].global_illumination[2] = 0.5;
            meteos[id].global_illumination[3] = 0.4;

            meteos[id].background_shader = meteos[METEO_NEBULA].background_shader;

            meteos[id].add_data = 1;
            break;

        case METEO_SCARLET_AURORA :
            meteos[id].global_illumination[0] = 0.6;
            meteos[id].global_illumination[1] = 0.5;
            meteos[id].global_illumination[2] = 0.4;
            meteos[id].global_illumination[3] = 0.4;

            meteos[id].background_shader = meteos[METEO_NEBULA].background_shader;

            meteos[id].add_data = 2;
            break;

        case METEO_JADE_AURORA : 
            meteos[id].global_illumination[0] = 0.4;
            meteos[id].global_illumination[1] = 0.6;
            meteos[id].global_illumination[2] = 0.5;
            meteos[id].global_illumination[3] = 0.4;

            meteos[id].background_shader = meteos[METEO_NEBULA].background_shader;

            meteos[id].add_data = 3;
            break;

        default:
            meteos[id].background_shader = meteos[METEO_MAIN_MENU].background_shader;
            break;
        }
    }
}