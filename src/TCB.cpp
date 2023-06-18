//
// Created by os on 6/17/23.
//

#include "../h/TCB.hpp"
#include "../h/riscv.hpp"
#include "../lib/console.h"

Thread* Thread::schedulerHead = nullptr;
Thread* Thread::schedulerTail = nullptr;
Thread* Thread::running = nullptr;
uint64 Thread::timeSliceCounter = 0;

Thread* Thread::create(Body body, void *arg, uint64 *stack) {
    return new Thread(body, arg, stack, DEFAULT_TIME_SLICE, Thread::threadWrapper);
}

Thread* Thread::createAndSwitchToUser(Body body, void *arg, uint64 *stack) {
    return new Thread(body, arg, stack, DEFAULT_TIME_SLICE, Thread::userMainWrapper);
}

void Thread::threadWrapper() {
    Riscv::returnFromSMode();
    //poziv stvarnog tijela niti
    Thread::running->body(Thread::running->argument);

    //nit je zavrsena

    Thread::running->setFinished(true);
    //za sad nije problem sto se ovde poziva dispatch, ali kada se dodaju nniti koje se izvrsavaju u korisnickom rezimu, dispatch ce se morati pozivati iz sistemskog rezima
    Thread::dispatch();
}

void Thread::userMainWrapper() {
    //postavlja SPP bit u sstatus na 0, kako bi se pri povratku iz prekidne rutine preslo u koriscnicki rezim
    Riscv::mc_sstatus(Riscv::SSTATUS_SPP);
    Riscv::returnFromSMode();
    //poziv stvarnog tijela niti
    Thread::running->body(Thread::running->argument);
    //userMain();

    //nit je zavrsena
    Thread::running->setFinished(true);
    //za sad nije problem sto se ovde poziva dispatch, ali kada se dodaju nniti koje se izvrsavaju u korisnickom rezimu, dispatch ce se morati pozivati iz sistemskog rezima
    Thread::dispatch();
}

void Thread::dispatch() {

    if ( Thread::schedulerHead == nullptr) return;

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

//stavlja nit u scheduler
void Thread::schedulerPut(Thread *thread) {
    thread->next = nullptr; //nije potrebno jer se postavlja na nulu u schedulerGet, ali za svaki slucaj
    if ( Thread::schedulerHead == nullptr) Thread::schedulerHead = Thread::schedulerTail = thread;
    else {
        Thread::schedulerTail->next = thread;
        Thread::schedulerTail = thread;
    }
}

//vraca kontekst - korisit se pri cuvanju/restauraciji konteksta kod prekida
uint64* Thread::getContext() {
    return Thread::running->context;
}

void Thread::yield()
{
    __asm__ volatile ("ecall");
}