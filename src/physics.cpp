#include <thread>
#include <chrono>
#include <algorithm>
#include <cstring>

#include "physics.hpp"

bool* get_valid_adjacent_blocks(World* world, coord3D coord) {
    bool* result = new bool[5]; // 0: x+1, 1: y+1, 2: x-1, 3: y-1, 4: z-1
    
    if (coord.z - 1 >= 0 && world->get_block_id_wcoord_nowvp(coord + coord3D({0, 0, -1})) == BLOCK_EMPTY) {
        memset(result, false, 4);
        result[4] = true;
    }
    else {
        result[0] = coord.x + 1 < CHUNK_SIZE * (world->max_chunk_coord.x + 1) && world->get_block_id_wcoord_nowvp(coord + coord3D({1, 0, 0})) == BLOCK_EMPTY;
        result[1] = coord.y + 1 < CHUNK_SIZE * (world->max_chunk_coord.y + 1) && world->get_block_id_wcoord_nowvp(coord + coord3D({0, 1, 0})) == BLOCK_EMPTY;
        result[2] = coord.x - 1 >= 0 && world->get_block_id_wcoord_nowvp(coord + coord3D({-1, 0, 0})) == BLOCK_EMPTY;
        result[3] = coord.y - 1 >= 0 && world->get_block_id_wcoord_nowvp(coord + coord3D({0, -1, 0})) == BLOCK_EMPTY;
        result[4] = false;
    }

    return result;
}

void PhysicsEventWater::execute() {
    if (tick_delay > 0) {
        PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, coord, tick_delay - 1);
        // std::cout << "new water event at " << coord << " with delay " << tick_delay - 1 << std::endl;
        engine->add_event(event, false);
        return;
    }

    constexpr coord3D coords[5] = {
        {1, 0, 0},
        {0, 1, 0},
        {-1, 0, 0},
        {0, -1, 0},
        {0, 0, -1}
    };
    bool* valid_adjacent_blocks = get_valid_adjacent_blocks(world, coord);


    for (int i = 0; i < 5; i++) {
        if (!valid_adjacent_blocks[i]) continue;
        coord3D neighbour = coord + coords[i];
        // std::cout << "water spread to " << neighbour << std::endl;
        block_coordonate neighbour_block_coord = neighbour.to_block_coordonate();
        
        event_handler->add_event(GAME_EVENT_SINGLE_BLOCK_MOD_ALT, neighbour_block_coord, BLOCK_WATER);
        PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, neighbour);
        engine->add_event(event);
    }


    delete[] valid_adjacent_blocks;
}




bool PhysicsEventWater::operator==(const PhysicsEvent* event) const {
    if (event->get_id() != PHYSICS_EVENT_WATER) return false;
    const PhysicsEventWater* event_cast = static_cast<const PhysicsEventWater*>(event);
    return coord == event_cast->get_coord();
}

bool PhysicsEventWaterCheckChunk::operator==(const PhysicsEvent* other) const { // checks if the event has the same type and chunk coord
    if (other->get_id() != PHYSICS_EVENT_WATER) return false;
    const PhysicsEventWaterCheckChunk* other_cast = static_cast<const PhysicsEventWaterCheckChunk*>(other);
    return chunk == other_cast->get_chunk();
}

bool HasNeighbourWithID(World* world, const coord3D& coord, const int id) {
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            if (x == 0 && y == 0) continue; // skip self
            if (x != 0 && y != 0) continue; // skip diagonals

            coord3D neighbour = coord + coord3D({x, y, 0});

            if (
                // check if neighbour is in world
                neighbour.x >= 0 && neighbour.x < CHUNK_SIZE * world->max_chunk_coord.x &&
                neighbour.y >= 0 && neighbour.y < CHUNK_SIZE * world->max_chunk_coord.y &&

                // check if neighbour is empty
                world->get_block_id_wcoord_nowvp(neighbour) == id
                ) {
                return true;
            }

        }
    }
    // check {0, 0, -1}
    coord3D neighbour = coord + coord3D({0, 0, -1});
    if (
        // check if neighbour is in world
        neighbour.x >= 0 && neighbour.x < CHUNK_SIZE * world->max_chunk_coord.x &&
        world->get_block_id_wcoord_nowvp(neighbour) == id
        ) {
        return true;
    }
    return false;
}

void PhysicsEventWaterCheckChunk::execute() {
    // engine->world_mutex.lock();
    if (world->chunks[chunk.x][chunk.y][chunk.z].compress_value == CHUNK_NON_UNIFORM) { // bound to break if the wvp is not normal 
        for (int x = 0; x < CHUNK_SIZE; x++) {
            for (int y = 0; y < CHUNK_SIZE; y++) {
                for (int z = 0; z < CHUNK_SIZE; z++) {
                    if (world->chunks[chunk.x][chunk.y][chunk.z].blocks[x][y][z].id == BLOCK_WATER) {
                        
                        if (HasNeighbourWithID(world, block_coordonate({x, y, z, chunk}), BLOCK_EMPTY)) {
                            PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({x, y, z, chunk}).to_coord3D());
                            // std::cout << "check_chunk at " << chunk.x << ", " << chunk.y << ", " << chunk.z << std::endl;
                            engine->add_event(event);
                        }
                    }
                }
            }
        }

        // check for water in the first blocks of neighbouring chunks
        for (int a = 0; a < CHUNK_SIZE; a++) { // iterate over the face of the chunk that is directly adjacent to the chunk we are checking
            for (int b = 0; b < CHUNK_SIZE; b++) {
                // check chunk above
                if (chunk.z < world->max_chunk_coord.z) {
                    if (world->chunks[chunk.x][chunk.y][chunk.z + 1].blocks[a][b][0].id == BLOCK_WATER) {
                        PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({a, b, 0, {chunk.x, chunk.y, chunk.z + 1}}).to_coord3D());
                        // std::cout << "check_chunk at " << chunk.x << ", " << chunk.y << ", " << chunk.z + 1 << std::endl;
                        engine->add_event(event);
                    }
                }
                // check chunk below
                if (chunk.z > 0) {
                    if (world->chunks[chunk.x][chunk.y][chunk.z - 1].blocks[a][b][CHUNK_SIZE - 1].id == BLOCK_WATER) {
                        PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({a, b, CHUNK_SIZE - 1, {chunk.x, chunk.y, chunk.z - 1}}).to_coord3D());
                        // std::cout << "check_chunk at " << chunk.x << ", " << chunk.y << ", " << chunk.z - 1 << std::endl;
                        engine->add_event(event);
                    }
                }
                // check chunk to the right
                if (chunk.x < world->max_chunk_coord.x) {
                    if (world->chunks[chunk.x + 1][chunk.y][chunk.z].blocks[0][a][b].id == BLOCK_WATER) {
                        PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({0, a, b, {chunk.x + 1, chunk.y, chunk.z}}).to_coord3D());
                        // std::cout << "check_chunk at " << chunk.x + 1 << ", " << chunk.y << ", " << chunk.z << std::endl;
                        engine->add_event(event);
                    }
                }
                // check chunk to the left
                if (chunk.x > 0) {
                    if (world->chunks[chunk.x - 1][chunk.y][chunk.z].blocks[CHUNK_SIZE - 1][a][b].id == BLOCK_WATER) {
                        PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({CHUNK_SIZE - 1, a, b, {chunk.x - 1, chunk.y, chunk.z}}).to_coord3D());
                        // std::cout << "check_chunk at " << chunk.x - 1 << ", " << chunk.y << ", " << chunk.z << std::endl;
                        engine->add_event(event);
                    }
                }
                // check chunk in front
                if (chunk.y < world->max_chunk_coord.y) {
                    if (world->chunks[chunk.x][chunk.y + 1][chunk.z].blocks[a][0][b].id == BLOCK_WATER) {
                        PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({a, 0, b, {chunk.x, chunk.y + 1, chunk.z}}).to_coord3D());
                        // std::cout << "check_chunk at " << chunk.x << ", " << chunk.y + 1 << ", " << chunk.z << std::endl;
                        engine->add_event(event);
                    }
                }
                // check chunk behind
                if (chunk.y > 0) {
                    if (world->chunks[chunk.x][chunk.y - 1][chunk.z].blocks[a][CHUNK_SIZE - 1][b].id == BLOCK_WATER) {
                        PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({a, CHUNK_SIZE - 1, b, {chunk.x, chunk.y - 1, chunk.z}}).to_coord3D());
                        // std::cout << "check_chunk at " << chunk.x << ", " << chunk.y - 1 << ", " << chunk.z << std::endl;
                        engine->add_event(event);
                    }
                }
            }
        }
    }
    else if (world->chunks[chunk.x][chunk.y][chunk.z].compress_value == BLOCK_WATER) { // only iterate over the edges and the botom of the chunk
        for (int a = 0; a < CHUNK_SIZE; a++) {
            for (int b = 0; b < CHUNK_SIZE; b++) {
                // check bottom
                if (HasNeighbourWithID(world, block_coordonate({a, b, 0, chunk}), BLOCK_EMPTY)) {
                    PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({a, b, 0, chunk}).to_coord3D());
                    // std::cout << "check_chunk at " << chunk.x << ", " << chunk.y << ", " << chunk.z << std::endl;
                    engine->add_event(event);
                }
                
                // check edges
                if (HasNeighbourWithID(world, block_coordonate({a, 0, b, chunk}), BLOCK_EMPTY)) {
                    PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({a, 0, b, chunk}).to_coord3D());
                    // std::cout << "check_chunk at " << chunk.x << ", " << chunk.y << ", " << chunk.z << std::endl;
                    engine->add_event(event);
                }
                if (HasNeighbourWithID(world, block_coordonate({a, CHUNK_SIZE - 1, b, chunk}), BLOCK_EMPTY)) {
                    PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({a, CHUNK_SIZE - 1, b, chunk}).to_coord3D());
                    // std::cout << "check_chunk at " << chunk.x << ", " << chunk.y << ", " << chunk.z << std::endl;
                    engine->add_event(event);
                }
                if (HasNeighbourWithID(world, block_coordonate({0, a, b, chunk}), BLOCK_EMPTY)) {
                    PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({0, a, b, chunk}).to_coord3D());
                    // std::cout << "check_chunk at " << chunk.x << ", " << chunk.y << ", " << chunk.z << std::endl;
                    engine->add_event(event);
                }
                if (HasNeighbourWithID(world, block_coordonate({CHUNK_SIZE - 1, a, b, chunk}), BLOCK_EMPTY)) {
                    PhysicsEventWater* event = new PhysicsEventWater(world, engine, event_handler, block_coordonate({CHUNK_SIZE - 1, a, b, chunk}).to_coord3D());
                    // std::cout << "check_chunk at " << chunk.x << ", " << chunk.y << ", " << chunk.z << std::endl;
                    engine->add_event(event);
                }
            }
        }
    }
    // engine->world_mutex.unlock();
}

PhysicsEngine::~PhysicsEngine() {
    while (!event_queue.empty()) {
        delete event_queue.front();
        event_queue.pop_front();
    }
    running = false;
    alive = false;
}

void PhysicsEngine::tick() {
    Uint64 start = Get_time_ms();
   
    // std::cout << "attempting to lock queue_mutex\n";
    queue_mutex.lock();
    // std::cout << "queue_mutex lock in tick" << std::endl;

    std::deque <PhysicsEvent*> event_queue_copy;

    int i = 0;
    for (PhysicsEvent* event : event_queue) {
        if (i++ == MAX_EVENTS_PER_TICK) {
            break;
        }
        event_queue_copy.push_back(event);
        event_queue.pop_front();
    }



    // std::cout << "queue_mutex unlock in tick" << std::endl;
    queue_mutex.unlock();

    // std::cout << "attempting to lock world_mutex\n";
    world_mutex.lock();
    // std::cout << "world_mutex lock in tick" << std::endl;  
    // std::cout << "executing " << event_queue_copy.size() << " events" << std::endl;
    while(!event_queue_copy.empty()) {
        event_queue_copy.front()->execute();
        delete event_queue_copy.front();
        event_queue_copy.pop_front();
    }
    // std::cout << "world_mutex unlock in tick" << std::endl;
    world_mutex.unlock();

    Uint64 end = Get_time_ms();
    Uint64 delta = end - start;

    if (delta < tick_delay) {
        // std::cout << "sleeping for " << tick_delay - delta << " ms" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(tick_delay - delta));
    }
    
}

void PhysicsEngine::add_event(PhysicsEvent* event, bool check_duplicates) {
    // std::cout << "adding event" << std::endl;
    // std::cout << "attempting to lock queue_mutex\n";
    queue_mutex.lock();
    // std::cout << "queue_mutex lock in add_event" << std::endl;

    if (!check_duplicates) {
        event_queue.push_back(event);
        queue_mutex.unlock();
        return;
    }

    // check if event is already in queue
    auto it = std::find_if(event_queue.begin(), event_queue.end(), [event](PhysicsEvent* event_in_queue) {
        return *event_in_queue == event;
    });

    if (it != event_queue.end()) {
        // event is already in queue
        // std::cout << "event already in queue" << std::endl;
        delete event;
    }
    else {
        // event is not in queue
        // std::cout << "inserted event" << std::endl;
        event_queue.push_back(event);
    }
    // std::cout << "queue_mutex unlock in add_event" << std::endl;
    queue_mutex.unlock();
}

void PhysicsEngine::add_event(int id, void* data) {
    // std::cout << "adding event " << id << std::endl;
    queue_mutex.lock();
    switch (id) {
        case PHYSICS_EVENT_WATER_CHECK_CHUNK: {
            PhysicsEventWaterCheckChunk* event = new PhysicsEventWaterCheckChunk(world, this, event_handler, *(chunk_coordonate*)data);
            event_queue.push_back(event);
            break;
        }
        case PHYSICS_EVENT_WATER_CHECK_BLOCK: {
            PhysicsEventWater* event = new PhysicsEventWater(world, this, event_handler, *(block_coordonate*)data);
            event_queue.push_back(event);
            break;
        }
    }
    queue_mutex.unlock();
}

void PhysicsEngine::clear_events() {
    queue_mutex.lock();
    while (!event_queue.empty()) {
        delete event_queue.front();
        event_queue.pop_front();
    }
    queue_mutex.unlock();
}

int EngineThread(void* arg) {
    PhysicsEngine* engine = (PhysicsEngine*)arg;
    while (engine->alive) {
        if (engine->is_running())
            engine->tick();
        else 
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return 0;
}

PhysicsEngine::PhysicsEngine(World* world, Multithreaded_Event_Handler* event_handler) : world(world), event_handler(event_handler) {
    running = true;
    alive = true;
    // thread = SDL_CreateThread(EngineThread, "PhysicsEngine", this);
}