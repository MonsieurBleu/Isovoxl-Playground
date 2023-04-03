// #include <Agent.hpp>
#include <game.hpp>


// Prpoblèmes de la fourmie : 
// - Elle ne voit que dans un espace de 1 block autour d'elle -> quasi aveugle 
// - A cause de cela, elle ne supporte pas les vitesses trop grandes, max 1 block par ticks 
// - Il reste très con et ne fait qu'avancé en ligne droite en collant les murs

void Ant::tick(void *) 
{
    if( (timems - last_tick) < tick_time )
    {
        return;
    }

    // std::cout << "executing new tick " << (int)state << "\n";

    last_tick = timems;

    int move = 0;
    bool refresh_rot = false;

    fcoord3D left  = {0.f, 0.f, 0.f};
    fcoord3D right = {0.f, 0.f, 0.f};

    float speed = 1.1;

    // switch (sprite->rotation)
    // {
    //     case 0 : left.y =  speed; right.y = -speed; break;
        
    //     case 1 : left.x =  speed; right.x = -speed; break;
        
    //     case 2 : left.x = -speed; right.x =  speed; break;
        
    //     case 3 : left.y = -speed; right.y =  speed; break;
        
    //     default: break;
    // }

    switch (sprite->rotation)
    {
        case 0 : left.x =  speed; right.x = -speed; break;
        
        case 1 : left.y = -speed; right.y = speed; break;
        
        case 2 : left.y = speed; right.y =  -speed; break;
        
        case 3 : left.x = -speed; right.x = speed; break;
        
        default: break;
    }

    bool void_left  = sprite->move(left, false, true, false, false, false) > 0;
    bool void_right = sprite->move(right, false, true, false, false, false) > 0;

    // system("cls");
    system("cls");
    std::cout << (int) state << " " << sprite->rotation << "\n";
    std::cout << sprite->move(left, false, true, false, false, false) 
    << " " << sprite->move(right, false, true, false, false, false) << "\n";

    // state = ANT_STATE_IDLE;
    // sprite->rotate_right();
    // sprite->rotate_left();
    // refresh_rot = true;

    switch (state)
    {
    case ANT_FORWARD :
    {
        if(!void_left)
        {
            state = ANT_OBSTACLE_FOUND;
            break;
        }

        move = forward();

        if(!move)
        {
            sprite->rotate_right();
            refresh_rot = true;

            // state = ANT_TURN_LEFT;
            state = ANT_OBSTACLE_FOUND;
        }
    }
    break;
    
    case ANT_OBSTACLE_FOUND :

        if(!void_left)
        {
            std::cout << "2222222222222222222222\n\n";
            move = forward();

            if(move == SPRITE_OBSTACLE_FOND)
            {
                std::cout << "OBSTACLE MON REUF!\n";
                sprite->rotate_right();
                refresh_rot = true;
                // move = forward();
            }
        }
        else if(void_left && void_right)
        {
            state = ANT_FORWARD;

            sprite->rotate_left();
            refresh_rot = true;
            // move = forward();
            // move = forward();
            // move = forward();
        }
        else
        {
            std::cout << "1111111111111111111111\n\n";
            // sprite->rotate_left();
            // refresh_rot = true;
            // move = forward();
            // state = ANT_FORWARD;
        }

    break;

    case ANT_TURN_LEFT :

        sprite->rotate_left();
        refresh_rot = true;
        state = ANT_FORWARD;

    break;

    case ANT_TURN_RIGHT :

        sprite->rotate_right();
        refresh_rot = true;
        state = ANT_FORWARD;

    break;


    default:
        break;
    }


    // bool refresh_anim = sprite->refresh_animation();
    bool refresh_anim = false;

    if((refresh_rot || refresh_anim) && move <= 0)
    {
        sprite->remove();
    }

    if((move > 0) || refresh_anim || refresh_rot)
    {
        sprite->update();
    }
}

int Ant::forward()
{
    fcoord3D direction = {0.f, 0.f, 0.f};

    // float speed = 0.0008*tick_time;

    float speed = 1.f;

    switch (sprite->rotation)
    {
        case 0 : direction.y = speed;  break;
        
        case 1 : direction.x = speed; break;
        
        case 2 : direction.x = -speed; break;
        
        case 3 : direction.y = -speed;  break;
        
        default: break;
    }

    return sprite->move(direction, true, true, false, true, false);
}