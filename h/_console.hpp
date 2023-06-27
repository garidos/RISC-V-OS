//
// Created by os on 6/22/23.
//

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
        arr = new char[capacity];

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

    //broj ispisa koji se trenutno obradjuju, da se sistem ne bi zavrsio prije nego sto se svi obrade
    static int cnt;

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
