//
// Created by os on 6/17/23.
//

#include "../h/TCB.h"
#include "../h/riscv.hpp"

void Thread::threadWrapper() {
    Riscv::returnFromSMode();
    //poziv stvarnog tijela niti
    Thread::running->body(Thread::running->argument);

    //nit je zavrsena

    Thread::running->finished = true;
    //ne moze samo da se pozove dispatch koji bi vratio nit u prekidnu rutinu gjde je izgubila procesor, bez da prethodno predje u sistemski rezim
    Thread::dispatch();
}

void Thread::dispatch() {

    Thread* old = Thread::running;

    if ( !old->isFinished() ) Thread::schedulerPut(old);
    Thread::running = Thread::schedulerGet();

    contextSwitch(old->context, Thread::running->context);
}

//vraca narednu nit u scheduleru
Thread* Thread::schedulerGet() {
    Thread* temp = Thread::schedulerHead;
    Thread::schedulerHead = temp->next;
    temp->next = nullptr;
    if ( Thread::schedulerHead == nullptr) Thread::schedulerTail = nullptr;
    return temp;
}

void Thread::schedulerPut(Thread *thread) {
    thread->next = nullptr; //nije potrebno jer se postavlja na nulu u schedulerGet, ali za svaki slucaj
    if ( Thread::schedulerHead == nullptr) Thread::schedulerHead = Thread::schedulerTail = thread;
    else {
        Thread::schedulerTail->next = thread;
        Thread::schedulerTail = thread;
    }
}