#include <light_handler.hpp>
#include <world.hpp>

int LightHandler::add(light_block lb)
{
    int freespace = -1;

    // std::cout << "Insertion of light " << (int)lb.id << " at pos " << lb.pos << "\n";
    
    int i = 0;
    while( i < MAX_LIGHT_NUMBER && buff[i].id)
    {
        // std::cout << (int)buff[i].id << " " << buff[i].pos << "\n";

        if(buff[i].pos == lb.pos && buff[i].id == lb.id)
            return LIGHT_DUPLICATE;
        
        i++;
    }

    freespace = i;

    if(freespace == MAX_LIGHT_NUMBER)
    {
        time_since_last_full = timems;
        is_full = true;
        return LIGHT_BUFFER_FULL;
    }

    is_full = false;

    // std::cout << "\t => allocated at pos " << freespace << "\n";

    buff[freespace].id = lb.id;
    coord3D tmp = lb.pos.to_coord3D();
    buff[freespace].pos.x = tmp.x;
    buff[freespace].pos.y = tmp.y;
    buff[freespace].pos.z = tmp.z;

    return 0;
}

void World::check_light_trash_bin()
{
    std::cout << "Checking lights trash bin with size " << lights.trash_bin.size();
    startbenchrono();

    for(auto i = lights.trash_bin.begin(); i != lights.trash_bin.end();)
    {
        Uint8 wid = get_block_id_wcoord_nowvp(i->pos);

        if(i->id == wid)
        {
            auto j = i;
            i++;
            lights.add(*j);
            lights.trash_bin.erase(j);
        }
        else
            i++;
    }

    endbenchrono();

    lights.is_full = true;
    lights.time_since_last_full = timems;
    for(int i = 0; i < MAX_LIGHT_NUMBER; i++)
        if(!lights.buff[i].id)
        {
            lights.time_since_last_full = 0;
            lights.is_full = false;
            return;
        }
}