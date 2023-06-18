//
// Created by os on 6/18/23.
//

#include "../h/riscv.hpp"
#include "../h/TCB.hpp"
#include "../lib/console.h"


void workerA(void* ) {
    for ( int i = 0; i < 100; i++) {
        __putc('A');
        Thread::yield();
    }

}

void workerB(void* ) {
   for ( int i = 0; i < 100; i++) {
        __putc('B');
        Thread::yield();
    }

}



int main() {

    Riscv::w_stvec((uint64) & Riscv::vectorTable | (uint64) 1);

    Thread *maine, *worker1, *worker2;

    maine = Thread::create(nullptr, nullptr, new uint64[DEFAULT_STACK_SIZE]);
    Thread::running = maine;
    worker1 = Thread::create( &workerA, nullptr, new uint64[DEFAULT_STACK_SIZE]);
    worker2 = Thread::create( &workerB, nullptr, new uint64[DEFAULT_STACK_SIZE]);

    Riscv::ms_sstatus(Riscv::SSTATUS_SIE);



    while(!(worker1->isFinished() && worker2->isFinished())) {
        Thread::yield();
    }

    __putc('\n');

    return 0;
}