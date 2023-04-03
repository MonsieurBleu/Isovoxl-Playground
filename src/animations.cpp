#include <game.hpp>

animation_loop::animation_loop()
{
    // frames.push_back({{0, 0}, 0});
    // frames.pop_back();

    duration = 0;
}

void animation_loop::init(int _loop_nb)
{
    loop_nb = _loop_nb;
    refresh_init_time();
}

void animation_loop::addframe(animation_frame new_animframe)
{
    duration += new_animframe.duration;

    frames.push_back(new_animframe);
}

void animation_loop::refresh_init_time()
{
    init_time = timems;
}

coord2D animation_loop::get_curr_frame()
{
    Uint64 time = (timems-init_time)%duration;

    for(Uint64 i = 0; i < frames.size(); i++)
    {

        if(time <= frames[i].duration)
        {  
            return frames[i].pos;
        }
        
        time -= frames[i].duration;
    }

    return frames[0].pos;
}