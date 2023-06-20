//
// Created by os on 6/18/23.
//

#include "../h/riscv.hpp"
#include "../h/TCB.hpp"
#include "../lib/console.h"
#include "../h/print.hpp"
#include "../h/syscall_c.hpp"

void workerBodyA(void*)
{
    for (uint64 i = 0; i < 10; i++)
    {
        printString("A: i=");
        printInteger(i);
        printString("\n");
        for (uint64 j = 0; j < 10000; j++)
        {
            for (uint64 k = 0; k < 30000; k++)
            {
                // busy wait
            }
//            TCB::yield();
        }
    }
}

void workerBodyB(void*)
{
    uint64 i;
    for ( i = 0; i < 16; i++)
    {
        printString("B: i=");
        printInteger(i);
        printString("\n");
        for (uint64 j = 0; j < 10000; j++)
        {
            for (uint64 k = 0; k < 30000; k++)
            {
                // busy wait
            }
//            TCB::yield();
        }
    }
    if ( i == 13) printString("DA");
}

static uint64 fibonacci(uint64 n)
{
    if (n == 0 || n == 1) { return n; }
    if (n % 10 == 0) { thread_dispatch(); }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

void workerBodyC(void*)
{
    uint8 i = 0;
    for (; i < 3; i++)
    {
        printString("C: i=");
        printInteger(i);
        printString("\n");
    }

    printString("C: yield\n");
    __asm__ ("li t1, 7");
    thread_dispatch();

    uint64 t1 = 0;
    __asm__ ("mv %[t1], t1" : [t1] "=r"(t1));

    printString("C: t1=");
    printInteger(t1);
    printString("\n");

    uint64 result = fibonacci(12);
    printString("C: fibonaci=");
    printInteger(result);
    printString("\n");

    for (; i < 6; i++)
    {
        printString("C: i=");
        printInteger(i);
        printString("\n");
    }
//    TCB::yield();
}

void workerBodyD(void*)
{
    uint8 i = 10;
    for (; i < 13; i++)
    {
        printString("D: i=");
        printInteger(i);
        printString("\n");
    }

    printString("D: yield\n");
    __asm__ ("li t1, 5");
    thread_dispatch();

    uint64 result = fibonacci(16);
    printString("D: fibonaci=");
    printInteger(result);
    printString("\n");

    for (; i < 16; i++)
    {
        printString("D: i=");
        printInteger(i);
        printString("\n");
    }
//    TCB::yield();
}




int main() {

    Riscv::mc_sstatus(Riscv::SSTATUS_SIE);
    Riscv::w_stvec((uint64) & Riscv::vectorTable | (uint64) 1);
    //Riscv::ms_sstatus(Riscv::SSTATUS_SIE);
    thread_t threads[5];

    //prva nit mora da se napravi bez poziva prekida, jer ce se tu dohvatati kontekst running niti koja jos ne postoji
    threads[0] = TCB::create( nullptr, nullptr, new uint64[DEFAULT_STACK_SIZE]);
    TCB::running = threads[0];

    thread_create(&threads[1], &workerBodyA, nullptr);
    printString("ThreadA created\n");
    thread_create(&threads[2], &workerBodyB, nullptr);
    printString("ThreadB created\n");
    thread_create(&threads[3], &workerBodyC, nullptr);
    printString("ThreadC created\n");
    thread_create(&threads[4], &workerBodyD, nullptr);
    printString("ThreadD created\n");

    //Riscv::w_stvec((uint64) & Riscv::vectorTable | (uint64) 1);
    Riscv::ms_sstatus(Riscv::SSTATUS_SIE);

    /*for ( int i = 1; i < 5; i++) {
        thread_join(threads[i]);
    }*/

    //thread_join(threads[2]);

    while (!(threads[1]->isFinished() &&
             threads[2]->isFinished() &&
             threads[3]->isFinished() &&
             threads[4]->isFinished()))
    {
        thread_dispatch();
    }



    for (auto &thread: threads)
    {
        delete thread;
    }
    printString("Finished\n");

    return 0;
}