#ifndef PROJEKAT__CONSOLE_HPP
#define PROJEKAT__CONSOLE_HPP

#include "../h/TCB.hpp"
#include "../h/SCB.hpp"

class consoleBuffer {

private:

    int capacity;
    int head, tail;

    char* arr;

public:

    consoleBuffer(int cap) : capacity(cap) {

        head = tail = -1;
        arr = (char*)MemoryAllocator::malloc(capacity * sizeof(char));
        //moze i new ovde

    }

    ~consoleBuffer() {
        head = tail = -1;
        delete[] arr;
    }

    bool isFull() { return (head != -1 && head == tail);};
    bool isEmpty() { return head == -1;};

    void put(char c);

    char get();

};


class CCB {
private:

    static int cap;

    static SCB *readyToRead, *readyToWrite;
    static consoleBuffer *inputBuffer, *outputBuffer;

public:

    static TCB* consumer;

    friend class Riscv;
    friend int main();

    static void outputThreadBody(void*);

};




#endif //PROJEKAT__CONSOLE_HPP
