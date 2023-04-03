#include <undo.hpp>

#include <cstring>


chunk_save::chunk_save() {
    this->size = 0;
    this->csize = 0;
    this->filled = 0;
    chunks = nullptr;
    cchunks = nullptr;
    refs = std::make_shared<int>(1);
}

chunk_save::chunk_save(int size, int csize) {
    this->size = size;
    this->csize = csize;
    this->filled = 0;
    chunks = new chunk_data[size];
    cchunks = new compressed_chunk_data[csize];
    refs = std::make_shared<int>(1);
    // std::cout << this << " creating " << size << "\n";
}

chunk_save::chunk_save(const chunk_save& other)
{

    // std::cout << "Calling constructor that duplicate ALL the memory\n";
    *other.refs += 1;
    this->size = other.size;
    this->csize = other.csize;
    this->filled = other.filled;

    this->chunks = other.chunks;
    this->cchunks = other.cchunks;
    this->refs = other.refs;
    // std::cout << this << " creating by copy " << size << " with refs " << *refs << "\n";
}

chunk_save::~chunk_save()
{
    *refs -= 1;
    // std::cout << this << " ~chunk_save with refs " << *refs << "\n";
    if(*refs == 0)
    {
        // std::cout << this << " delete " << size << "\n";
        delete[] chunks;
        delete[] cchunks;
    }
}

chunk_coordonate chunk_save::get_smallest_chunk_coord_sum() const {
    chunk_coordonate smallest ;
    if(!size && csize) 
        smallest = cchunks[0].coord;
    else if(size) 
        smallest = chunks[0].coord;

    for (int i = 1; i < size; i++) {
        if (chunks[i].coord.x + chunks[i].coord.y + chunks[i].coord.z < smallest.x + smallest.y + smallest.z) {
            smallest = chunks[i].coord;
        }
    }

    for (int i = 0; i < csize; i++) {
        if (cchunks[i].coord.x + cchunks[i].coord.y + cchunks[i].coord.z < smallest.x + smallest.y + smallest.z) {
            smallest = cchunks[i].coord;
        }
    }
    return smallest;
}

chunk_coordonate chunk_save::get_highest_chunk_coord_sum() const {
    chunk_coordonate highest;
    if(!size && csize) 
        highest = cchunks[0].coord;
    else if(size) 
        highest = chunks[0].coord;

    for (int i = 1; i < size; i++) {
        if (chunks[i].coord.x + chunks[i].coord.y + chunks[i].coord.z > highest.x + highest.y + highest.z) {
            highest = chunks[i].coord;
        }
    }

    for (int i = 0; i < csize; i++) {
        if (cchunks[i].coord.x + cchunks[i].coord.y + cchunks[i].coord.z > highest.x + highest.y + highest.z) {
            highest = cchunks[i].coord;
        }
    }
    return highest;
}

void chunk_save::apply(chunk_save& other) {
    delete chunks;
    delete cchunks;
    *other.refs += 1;
    this->size = other.size;
    this->csize = other.csize;
    this->filled = other.filled;

    this->chunks = other.chunks;
    this->cchunks = other.cchunks;

    this->refs = other.refs;
}

void chunk_save::resize(int size, int csize) { 
    if (chunks != nullptr)
        delete[] chunks;
    if (cchunks != nullptr)
        delete[] cchunks;
    this->size = size;
    this->csize = csize;
    this->filled = 0;
    chunks = new chunk_data[size];
    cchunks = new compressed_chunk_data[csize];
}

UndoManager::UndoManager(int max_size) {
    this->max_size = max_size;
    this->current_size = 0;
}


void UndoManager::add_undo(chunk_save &data, bool clear_stack) {
    if (data.get_size_in_bytes() > max_size) { // if the data is bigger than the max size, don't add it - could cause bugs but it's mostly there just in case
        clear();
        return;
    }
    std::cout << "adding undo" << std::endl;
    std::cout << "current size: " << current_size << std::endl;


    if (clear_stack) {
        // clear the redo stack
        while (!redo_stack.empty()) {
            current_size -= redo_stack.front().get_size_in_bytes();
            redo_stack.pop_front();
        }
    }


    if (current_size + data.get_size_in_bytes() > max_size && !undo_stack.empty()) {
        // std::cout << "undo stack is too big, removing oldest undos" << std::endl;
        // remove the oldest undo (the oldest undo is at the bottom of the stack)
        std::deque<chunk_save> temp_stack;
        Uint64 temp_size = 0;
        while (temp_size + data.get_size_in_bytes() <= max_size && !undo_stack.empty())
        {
            temp_size += undo_stack.front().get_size_in_bytes();

            chunk_save cs = undo_stack.front();
            temp_stack.push_back(cs);
            undo_stack.pop_front();
        }

        // clear the undo stack
        while (!undo_stack.empty()) {
            current_size -= undo_stack.front().get_size_in_bytes();
            undo_stack.pop_front();
        }

        // add the undos back to the undo stack
        while (!temp_stack.empty()) {
            chunk_save cs = temp_stack.front();
            undo_stack.push_back(cs);
            temp_stack.pop_front();
        }
    }

    if(current_size + data.get_size_in_bytes() <= max_size) // if the precedent intructions didn't fail to get place for the new data
    {
        undo_stack.push_front(data);
        current_size += data.get_size_in_bytes();
    }
}


void UndoManager::add_redo(chunk_save &data) {
    if (data.get_size_in_bytes() > max_size) { // if the data is bigger than the max size, don't add it - could cause bugs but it's mostly there just in case
        clear();
        return;
    }

    /*  // commented out because it's not needed probably
    if (current_size + data.get_size_in_bytes() > max_size) {
        // remove the oldest undo
        while (current_size + data.get_size_in_bytes() > max_size) {
            current_size -= undo_stack.top().get_size_in_bytes();
            undo_stack.pop();
        }
    }
    */
    std::cout << "adding redo " << redo_stack.size() << "\n";
    redo_stack.push_front(data);
    current_size += data.get_size_in_bytes();
}

chunk_save empty_dat;

chunk_save UndoManager::undo() {
    if (undo_stack.empty()) {
        std::cout << "undo stack empty" << std::endl;
        return empty_dat;
    }
    std::cout << "undoing" << std::endl;

    chunk_save data = undo_stack.front();
    undo_stack.pop_front();
    current_size -= data.get_size_in_bytes();
    return data;
}

chunk_save UndoManager::redo() {
    if (redo_stack.empty()) {
        std::cout << "redo stack empty" << std::endl;
        return chunk_save(0, 0);
    }
    std::cout << "redoing" << std::endl;

    chunk_save data = redo_stack.front();
    redo_stack.pop_front();
    current_size -= data.get_size_in_bytes();
    return data;
}