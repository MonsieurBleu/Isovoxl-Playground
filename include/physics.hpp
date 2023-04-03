#ifndef PHYSICS_HPP
#define PHYSICS_HPP

class Multithreaded_Event_Handler;
struct Texture;

#include <multithreaded_event_handler.hpp>
#include <world.hpp>
#include <deque>
#include <mutex>

class PhysicsEvent;

class PhysicsEngine {
private:
    std::deque<PhysicsEvent*> event_queue; // not a queue because we need to be able to search for duplicates

    constexpr static int MAX_EVENTS_PER_TICK = 4000;

    const Uint64 tick_delay = 50;
    SDL_Thread* thread;
    std::mutex queue_mutex;

    

    bool running;

    World* world;
    Multithreaded_Event_Handler* event_handler;

public:
    bool alive;
    std::mutex world_mutex;
    void toggle_running() { running = !running; }
    bool is_running() const { return running; }

    PhysicsEngine(World* world, Multithreaded_Event_Handler* event_handler);
    ~PhysicsEngine();

    void tick();
    void add_event(PhysicsEvent* event, bool check_duplicates = true);
    void add_event(int id, void* data); // for special events, like chunk checking

    void clear_events();
};

int EngineThread(void* arg);

class PhysicsEvent {
protected:
    World* world;
    PhysicsEngine* engine;
    Multithreaded_Event_Handler* event_handler;
    Uint8 id;
public:
    PhysicsEvent(World* world, PhysicsEngine* engine, Multithreaded_Event_Handler* event_handler, Uint8 id) : world(world), engine(engine), event_handler(event_handler), id(id) {}
    virtual ~PhysicsEvent() {}
    virtual void execute() = 0;
    virtual bool operator==(const PhysicsEvent* other) const = 0; // used to compare events in the queue to avoid duplicates
    Uint8 get_id() const { return id; }
};

class PhysicsEventWater : public PhysicsEvent {
private:
    coord3D coord;
    int tick_delay;
public:
    PhysicsEventWater(World* world, PhysicsEngine* engine, Multithreaded_Event_Handler* event_handler, coord3D coord, int tick_delay = 3) : PhysicsEvent(world, engine, event_handler, PHYSICS_EVENT_WATER), coord(coord), tick_delay(tick_delay) {}
    void execute();
    bool operator==(const PhysicsEvent* other) const;
    coord3D get_coord() const { return coord; }
};

// checks a chunk that has been modified for water to eventually spread to empty blocks
class PhysicsEventWaterCheckChunk : public PhysicsEvent {
private:
    chunk_coordonate chunk;
public:
    PhysicsEventWaterCheckChunk(World* world, PhysicsEngine* engine, Multithreaded_Event_Handler* event_handler, chunk_coordonate chunk) : PhysicsEvent(world, engine, event_handler, PHYSICS_EVENT_WATER_CHECK_CHUNK), chunk(chunk) {}
    void execute();
    bool operator==(const PhysicsEvent* other) const;
    chunk_coordonate get_chunk() const { return chunk; }
};

#endif