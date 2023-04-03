#ifndef METEO_HPP
#define METEO_HPP

#include <Shader.hpp>
#include <memory>

#define METEO_COUNTER 16

#define METEO_MAIN_MENU      0

#define METEO_ANIMATED_SKY      1
#define METEO_ANIMATED_DUSK     2

#define METEO_NEBULA            3
#define METEO_AZUR_AURORA       4
#define METEO_ORCHID_AURORA     5
#define METEO_SCARLET_AURORA    6
#define METEO_JADE_AURORA       7

struct meteo
{
    float global_illumination[4];
    std::shared_ptr<Shader> background_shader;
    unsigned int add_data;
};

extern meteo meteos[METEO_COUNTER];

void init_meteos();

#endif