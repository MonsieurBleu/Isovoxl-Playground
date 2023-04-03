#include <string>

struct FileCircularBufferNode {
    FileCircularBufferNode* next;
    FileCircularBufferNode* prev;
    int id;
    std::string filepath;
    bool allocated;

    FileCircularBufferNode(FileCircularBufferNode* next, FileCircularBufferNode* prev, int id, std::string filepath, bool allocated=false) :
        next(next), prev(prev), id(id), filepath(filepath), allocated(allocated) {}

    FileCircularBufferNode* circularNext() {
        return next;
    }

    FileCircularBufferNode* circularPrev() {
        return prev;
    }

    FileCircularBufferNode* operator++() {
        return circularNext();
    }

    FileCircularBufferNode* operator--() {
        return circularPrev();
    }

    FileCircularBufferNode* operator++(int) {
        FileCircularBufferNode* tmp = circularNext();
        return tmp;
    }

    FileCircularBufferNode* operator--(int) {
        FileCircularBufferNode* tmp = circularPrev();
        return tmp;
    }

    operator bool() {
          return allocated;
    }
};

struct FileCircularBuffer {
    FileCircularBufferNode* head;

    FileCircularBuffer() {
        head = nullptr;
    }

    void init(int size, std::string folder) {
        head = new FileCircularBufferNode(nullptr, nullptr, 0, folder + "0.bak");
        FileCircularBufferNode* current = head;
        for(int i = 1; i < size; i++) {
            current->next = new FileCircularBufferNode(nullptr, current, i, folder + std::to_string(i) + ".bak");
            current = current->next;
        }
        current->next = head;
        head->prev = current;
    }

    void free() {
        if (!head) return;
        FileCircularBufferNode* current = head;
        do {
            FileCircularBufferNode* next = current->next;
            delete current;
            current = next;
        } while(current != head);
        head = nullptr;
    }

    ~FileCircularBuffer() {
        free();
    }

    void clear() {
        FileCircularBufferNode* current = head;
        do {
            current->allocated = false;
            remove(current->filepath.c_str());
            current = current->next;
        } while(current != head);
    }

    // prefix operator checks for allocation
    FileCircularBufferNode* operator++() {
        FileCircularBufferNode* tmp = head;
        do {
            head = head->next;
            if (head == tmp) return nullptr;
        } while (!head->allocated);

        return head;
    }

    // prefix operator checks for allocation
    FileCircularBufferNode* operator--() {
        FileCircularBufferNode* tmp = head;
        do {
            head = head->prev;
            if (head == tmp) return nullptr;
        } while (!head->allocated);
        return head;
    }

    // postfix operator doesnt check for allocation
    FileCircularBufferNode* operator++(int) {
        FileCircularBufferNode* tmp = head;
        head = head->next;
        return tmp;
    }

    // postfix operator doesnt check for allocation
    FileCircularBufferNode* operator--(int) {
        FileCircularBufferNode* tmp = head;
        head = head->prev;
        return tmp;
    }

    FileCircularBufferNode* operator[](int id) { // get the element with id
        FileCircularBufferNode* current = head;
        do {
            if(current->id == id) {
                return current;
            }
            current = current->next;
        } while(current != head);
        return nullptr;
    }

    // checks if any is allocated
    bool any() { 
        FileCircularBufferNode* current = head;
        do {
            if(current->allocated) {
                return true;
            }
            current = current->next;
        } while(current != head);
        return false;
    }

    // checks if all are allocated
    bool all() { 
        FileCircularBufferNode* current = head;
        do {
            if(!current->allocated) {
                return false;
            }
            current = current->next;
        } while(current != head);
        return true;
    }
};



