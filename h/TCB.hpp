//
// Created by os on 6/17/23.
//

// dio TCB klase uzet sa vjezbi

#ifndef PROJEKAT_TCB_HPP
#define PROJEKAT_TCB_HPP

#include "../lib/hw.h"

using Body = void (*)(void*);

class TCB {

private:

    friend class Riscv;

    Body body;
    void *argument;
    uint64 *stack;

    uint64 timeSlice;
    bool finished;

    //kontekst se cuva unutar samog objekta niti
    uint64 *context;

    enum registerOffs {
        raOffs = 1, spOffs, gpOffs, tpOffs, t0Offs, t1Offs, t2Offs, s0Offs, s1Offs, a0Offs, a1Offs, a2Offs, a3Offs, a4Offs, a5Offs, a6Offs,
        a7Offs, s2Offs, s3Offs, s4Offs, s5Offs, s6Offs, s7Offs, s8Offs, s9Offs, s10Offs, s11Offs, t3Offs, t4Offs, t5Offs, t6Offs
    };

    /*
    * context:
    * 0 * 8(context) : sra  // sistemski ra koji je u prekidu i na koji se vraca prilikom promjene konteksta
    * 1 * 8(context) : ra
    * 2 * 8(context) : sp
    * 3 * 8(context) : gp
    * 4 * 8(context) : tp
    * 5 * 8(context) : t0
    * 6 * 8(context) : t1
    * 7 * 8(context) : t2
    * 8 * 8(context) : s0/fp
    * 9 * 8(context) : s1
    * 10 * 8(context) : a0
    * 11 * 8(context) : a1
    * 12 * 8(context) : a2
    * 13 * 8(context) : a3
    * 14 * 8(context) : a4
    * 15 * 8(context) : a5
    * 16 * 8(context) : a6
    * 17 * 8(context) : a7
    * 18 * 8(context) : s2
    * 19 * 8(context) : s3
    * 20 * 8(context) : s4
    * 21 * 8(context) : s5
    * 22 * 8(context) : s6
    * 23 * 8(context) : s7
    * 24 * 8(context) : s8
    * 25 * 8(context) : s9
    * 26 * 8(context) : s10
    * 27 * 8(context) : s11
    * 28 * 8(context) : t3
    * 29 * 8(context) : t4
    * 30 * 8(context) : t5
    * 31 * 8(context) : t6
    * 32 * 8(context) : ssp //nije sistemski stek vec pokazivac na vrh steka u trenutku prekida
    */

    TCB(Body threadBody, void* threadArgument, uint64* allocatedStack, uint64 timeSlice, void (*wrapper)()) :
            body(threadBody), argument(threadArgument), stack(allocatedStack), timeSlice(timeSlice),
            finished(false), context(new uint64[33]), waitingHead(nullptr), waitingTail(nullptr)
    {
        if ( stack != nullptr) context[32] = (uint64) &stack[DEFAULT_STACK_SIZE];
        else context[32] = 0;


        /*sistemski ra se inicijalizuje sa adreso threadWrappera u koji ce odmah skociti kada prvi put krene da se izvrsava
         * jer nema nikakav kontekst koji treba da restaurira
         * - zato se u threadWrapperu prvo treba izaci iz sistemskog rezima ( a i upisati odgovarajuca vrijednost u sepc jer se tu vraca )
         * prvi promjeni konteksta unutar prekida mora se takodje promijeniti stek, da bi nova nit koristila svoj stek za izvrsavanje unutar prekida ( a i kod nove niti da bi ona odmah dobila svoj stek, jer se ne vrsi restauracija registara )
        */

        context[0] = (uint64) wrapper;

        this->next = nullptr;
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
    TCB* next;

    static TCB* schedulerHead;
    static TCB* schedulerTail;

    TCB* waitingHead;
    TCB* waitingTail;


    static void threadWrapper();
    static void userMainWrapper();

    static void contextSwitch(uint64 *oldContext, uint64 *runningContext);



public:

    ~TCB()
    {
        delete[] stack;
        delete[] context;
    }

    bool isFinished() const { return finished; }

    void setFinished(bool value) { finished = value; }

    uint64 getTimeSlice() const { return timeSlice; }

    static TCB* schedulerGet();
    static void schedulerPut(TCB * thread);

    void putWaiting(TCB* thread);
    void emptyWaiting();

    static TCB* running;

    static void dispatch(bool flag);

    static void yield();

    static uint64* getContext();

    static TCB* create(Body body, void* arg, uint64* stack);

    static TCB* createAndSwitchToUser(Body body, void* arg, uint64* stack);


};






#endif //PROJEKAT_TCB_HPP