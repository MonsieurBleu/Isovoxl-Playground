#include <game.hpp>

// double mticks()
// {
//     typedef std::chrono::high_resolution_clock clock;
//     typedef std::chrono::duration<float, std::milli> duration;

//     static clock::time_point start = clock::now();
//     duration elapsed = clock::now() - start;
//     return elapsed.count();
// }

typedef std::chrono::high_resolution_clock clockmicro;
typedef std::chrono::duration<float, std::milli> duration;

clockmicro::time_point start;
// clock::time_point end;

void Game::init_framerate()
{
    if(vsync) return;
    start = clockmicro::now();
}

const int AVG_NBFRAMES = 50;
float avg_frame_buff[AVG_NBFRAMES]; 
int curr_avg_frame = 0;

void Game::handle_framerate()
{
    // I have to forced it every frame because the effect randomly reset in some weird conditions.
    SDL_GL_SetSwapInterval(vsync);

    // std::cout << "\t vsync = " << SDL_GL_GetSwapInterval() << "\n";
    if(vsync) return;

    duration elapsed = clockmicro::now() - start;

    // while(elapsed.count() < 1000.0/200.0)
    while(elapsed.count() < max_frametime)
    {
        elapsed = clockmicro::now() - start;
    }

    if(Show_FPS)
    {
        avg_frame_buff[curr_avg_frame] = elapsed.count();
        curr_avg_frame ++;

        if(curr_avg_frame == AVG_NBFRAMES)
        {
            float buff = 0;

            for(int i = 0; i < AVG_NBFRAMES; i++)
            {
                buff += avg_frame_buff[i];
            }

            buff /= (float)(AVG_NBFRAMES);

            std::cout << "avg framerate : " << 1000.0/buff << "\t\t";
            std::cout << "avg frametime : " << buff << "\n";

            curr_avg_frame = 0;
        }
    }

    // std::cout << elapsed.count() << "\t" << SDL_GL_GetSwapInterval() << "\n";
}

clockmicro::time_point benchrono;

void startbenchrono()
{
    benchrono = clockmicro::now();
}

void endbenchrono()
{
    duration elapsed = clockmicro::now() - benchrono;

    std::cout << " in " << elapsed.count() << " ms \n";
}
