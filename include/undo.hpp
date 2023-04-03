#ifndef UNDO_HPP
#define UNDO_HPP

#include <deque>
#include <memory>
#include <blocks.hpp>
#include <coords.hpp>

struct chunk_data {
    chunk_coordonate coord;
    block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    int compress_value;
};

struct compressed_chunk_data
{
    chunk_coordonate coord;
    int compress_value;
};

struct chunk_save {
    int size;
    int csize;
    int filled; // I completely forgot what this is for :skull:
    chunk_data* chunks;
    compressed_chunk_data* cchunks;
    std::shared_ptr<int> refs;

    chunk_save();
    chunk_save(int size, int csize);
    chunk_save(const chunk_save& other);

    ~chunk_save();

    chunk_coordonate get_smallest_chunk_coord_sum() const; // used to get the corner of a rectangle that contains all the chunks
    chunk_coordonate get_highest_chunk_coord_sum() const; // used to get the other corner of a rectangle that contains all the chunks

    // resize with data loss
    void resize(int size, int csize);

    
    chunk_data* get_chunk(int index) {
        return &chunks[index];
    }


    compressed_chunk_data* get_compressed(int index) {
        return &cchunks[index];
    }

    Uint64 get_size_in_bytes() const {
        return size * sizeof(chunk_data) + csize * sizeof(compressed_chunk_data);
    }

    void apply(chunk_save& other);
};


class UndoManager {
private:
    std::deque<chunk_save> undo_stack;
    std::deque<chunk_save> redo_stack;


    
    Uint64 max_size; // max size in bytes of the chunks saved in the undo manager

    Uint64 current_size; // current size in bytes of the chunks saved in the undo manager

public:
    UndoManager(int max_size);

    void add_undo(chunk_save &data, bool clear = true);

    void add_redo(chunk_save &data);

    chunk_save undo();

    chunk_save redo();

    bool can_undo() {
        return !undo_stack.empty();
    }

    bool can_redo() {
        return !redo_stack.empty();
    }

    void clear() {
        // std::cout << "CLEARING!!!\n";
        while (!undo_stack.empty()) {
            undo_stack.pop_back();
        }
        while (!redo_stack.empty()) {
            redo_stack.pop_back();
        }
    }

    Uint64 get_max_size() {
        return max_size;
    }

    Uint64 get_current_size() {
        return current_size;
    }

    void set_max_size(Uint64 max_size) {
        this->max_size = max_size;

        // if the current size is bigger than the max size, clear the undo stack
        if (current_size > max_size) {
            clear();
        }
    }
};






#endif // UNDO_HPP