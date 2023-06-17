//
// Created by os on 6/17/23.
//

// dio Thread klase uzet sa vjezbi

#ifndef PROJEKAT_TCB_H
#define PROJEKAT_TCB_H

#include "../lib/hw.h"

using Body = void (*)(void*);

class Thread {

private:

    uint64 timeSlice;
    bool finished;

    Body body;
    void *argument;
    uint64 *stack;

    //kontekst se cuva unutar samog objekta niti
    uint64 *context;

    /*
    * context:
    * 0 * 8(registers) : sra  // sistemski ra koji je u prekidu i na koji se vraca prilikom promjene konteksta
    * 1 * 8(registers) : ra
    * 2 * 8(registers) : sp
    * 3 * 8(registers) : gp
    * 4 * 8(registers) : tp
    * 5 * 8(registers) : t0
    * 6 * 8(registers) : t1
    * 7 * 8(registers) : t2
    * 8 * 8(registers) : s0/fp
    * 9 * 8(registers) : s1
    * 10 * 8(registers) : a0
    * 11 * 8(registers) : a1
    * 12 * 8(registers) : a2
    * 13 * 8(registers) : a3
    * 14 * 8(registers) : a4
    * 15 * 8(registers) : a5
    * 16 * 8(registers) : a6
    * 17 * 8(registers) : a7
    * 18 * 8(registers) : s2
    * 19 * 8(registers) : s3
    * 20 * 8(registers) : s4
    * 21 * 8(registers) : s5
    * 22 * 8(registers) : s6
    * 23 * 8(registers) : s7
    * 24 * 8(registers) : s8
    * 25 * 8(registers) : s9
    * 26 * 8(registers) : s10
    * 27 * 8(registers) : s11
    * 28 * 8(registers) : t3
    * 29 * 8(registers) : t4
    * 30 * 8(registers) : t5
    * 31 * 8(registers) : t6
    */

    Thread(Body threadBody, void* threadArgument, uint64* allocatedStack, uint64 timeSlice, void (*wrapper)()) :
        body(threadBody), argument(threadArgument), timeSlice(timeSlice), stack(allocatedStack),
        context(new uint64[31]), finished(false)
    {
        if ( stack != nullptr) context[1] = (uint64) stack[DEFAULT_STACK_SIZE];
        else context[2] = 0;


        /*sistemski ra se inicijalizuje sa adreso threadWrappera u koji ce odmah skociti kada prvi put krene da se izvrsava
         * jer nema nikakav kontekst koji treba da restaurira
         * - zato se u threadWrapperu prvo treba izaci iz sistemskog rezima ( a i upisati odgovarajuca vrijednost u sepc jer se tu vraca )
         * prvi promjeni konteksta unutar prekida mora se takodje promijeniti stek, da bi nova nit koristila svoj stek za izvrsavanje unutar prekida ( a i kod nove niti da bi ona odmah dobila svoj stek, jer se ne vrsi restauracija registara )
        */

        context[0] = (uint64) &wrapper;

        this->next = 0;
        //main funkcija ce imati prazno tijelo, i ona ce prva da se pravi i odmah izvrsava, tako da se ne treba stavljati u scheduler
        if ( body != nullptr) {
            if ( schedulerHead != nullptr )
            {
                schedulerTail->next = this;
                schedulerTail = this;
            }
            else
            {
                schedulerHead = schedulerTail = this;
            }
        }
    }

    static uint64 timeSliceCounter;

    //pokazivac na narednu nit u nizu - taj niz moze biti niz niti u Scheduleru, uspavanih niti itd.
    Thread* next;

    static Thread* schedulerHead;
    static Thread* schedulerTail;

    static void threadWrapper();
    static void userMainWrapper();

    static void contextSwitch(uint64 *oldContext, uint64 *runningContext);

public:

    ~Thread()
    {
        delete[] stack;
        delete[] context;
    }

    bool isFinished() const { return finished; }

    void setFinished(bool value) { finished = value; }

    uint64 getTimeSlice() const { return timeSlice; }

    static Thread* running;

    static Thread* schedulerGet();
    static void schedulerPut(Thread * thread);

    static void dispatch();

    static Thread* create( Body body, void* arg, uint64* stack);

    static Thread* createAndSwitchToUser(Body body, void* arg, uint64* stack);


};






#endif //PROJEKAT_TCB_H
