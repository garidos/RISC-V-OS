#ifndef PROJEKAT_SCB_HPP
#define PROJEKAT_SCB_HPP

#include "../h/TCB.hpp"

// Semaphore Control Block
class SCB {
public:

    void wait();
    void signal();

    int getVal() const { return val; };

    int active() const { return isActive;};

    static SCB* create(int i = 1);

    void* operator new(size_t size);

    void operator delete (void* pointer);

protected:

    friend class Riscv;

    bool isActive;

    void release();
    void block();
    void unblock();

private:

    //explicit da bi se izbjegla implicitna konverzija, jer prima jedna argument ( konverzioni konstruktor )
    explicit SCB( int i = 1) : isActive(true), val(i), blockedHead(nullptr), blockedTail(nullptr)  {};

    int val;

    TCB* blockedHead;
    TCB* blockedTail;

};

#endif //PROJEKAT_SCB_HPP
