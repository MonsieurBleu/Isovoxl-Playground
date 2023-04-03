#include <world.hpp>
#include <sprites.hpp>
#include <constants.hpp>

//////// AGENTS ////////
#define AGENT_ANT 0
#define AGENT_PATHFINDER 1

//////// ANT STATES ////////
#define ANT_STATE_IDLE 0
#define ANT_FORWARD    1
#define ANT_TURN_LEFT  2
#define ANT_TURN_RIGHT 3
#define ANT_OBSTACLE_FOUND 4

//////// PATH FINDER ////////
#define PF_STATE_IDLE 0

class Agent
{
    protected:
        int type;
        World* world;
        Sprite* sprite;

        Uint64 last_tick;
        Uint64 tick_time;

    public:
        Agent(int type, 
              World* world, 
              Sprite* sprite, 
              Uint64 tick_time = 500) : type(type), 
                                        world(world), 
                                        sprite(sprite), 
                                        tick_time(tick_time){}
        
        virtual ~Agent() = default;
        virtual void tick(void*) = 0;
        virtual void draw() = 0;
};

class Ant : public Agent
{
    private:
        Uint8 state;

        int forward();
        void turn();

    public:
        Ant(World* world, Sprite* sprite, Uint64 tick_time = 500) : Agent(AGENT_ANT, world, sprite, tick_time), state(ANT_FORWARD) {}
        void tick(void*) override;
        void draw() override {}
};

class Pathfinder : public Agent 
{
    private:
        Uint8 state;

        coord3D next_target();
        coord3D foward();

        bool check_path();

        void define_next_target();


    public:
        Pathfinder(World* world, Sprite* sprite, Uint64 tick_time = 500) 
            : Agent(AGENT_PATHFINDER, world, sprite, tick_time), state(PF_STATE_IDLE) {}
        
        // call tick() every tick_time ms
        void handle();
        void tick(void*) override;
        void draw() override {}
};