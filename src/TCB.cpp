//
// Created by os on 6/17/23.
//

#include "../h/TCB.hpp"
#include "../h/riscv.hpp"

TCB* TCB::schedulerHead = nullptr;
TCB* TCB::schedulerTail = nullptr;
TCB* TCB::sleepingHead = nullptr;
TCB* TCB::running = nullptr;
uint64 TCB::timeSliceCounter = 0;

void userMain();

TCB* TCB::create(Body body, void *arg, uint64 *stack) {
    return new TCB(body, arg, stack, DEFAULT_TIME_SLICE, TCB::threadWrapper);
}

TCB* TCB::createAndSwitchToUser(uint64 *stack) {
    // nije bitno sta se stavi u body (samo ako stavim nullptr moram da promjenim uslov za stavljanje u scheduler u konstruktoru), jer ce se userMain pozivati direktno
    return new TCB(nullptr, nullptr, stack, DEFAULT_TIME_SLICE, TCB::userMainWrapper);
}

void TCB::threadWrapper() {
    //da bi bio siguran da sve niti koje su napravljene u korisnickom rezimu rade u istom tom rezimu ( setUser se postavlja u thread_create ako se nit pravi  iz korisnickog )
    if ( TCB::running->setUser) Riscv::mc_sstatus(Riscv::SSTATUS_SPP);
    Riscv::returnFromSMode();
    //poziv stvarnog tijela niti
    TCB::running->body(TCB::running->argument);

    //nit je zavrsena

    TCB::running->setFinished(true);
    //oslobadjaju se sve niti koje su cekale na ovu
    TCB::running->emptyWaiting();
    //za sad nije problem sto se ovde poziva dispatch, ali kada se dodaju nniti koje se izvrsavaju u korisnickom rezimu, dispatch ce se morati pozivati iz sistemskog rezima
    //TCB::timeSliceCounter = 0;
    //ovo je u stvari thread_dispatch(), ali da ne bi ovde pozivao c api funkcije
    Riscv::load_a0((uint64)Riscv::syscallCodes::thread_dispatch);
    __asm__ volatile("ecall;");

}

void TCB::userMainWrapper() {
    //postavlja SPP bit u sstatus na 0, kako bi se pri povratku iz prekidne rutine preslo u koriscnicki rezim
    Riscv::mc_sstatus(Riscv::SSTATUS_SPP);
    Riscv::returnFromSMode();
    //poziv stvarnog tijela niti
    userMain();

    //nit je zavrsena
    TCB::running->setFinished(true);
    //za sad nije problem sto se ovde poziva dispatch, ali kada se dodaju nniti koje se izvrsavaju u korisnickom rezimu, dispatch ce se morati pozivati iz sistemskog rezima
    TCB::running->emptyWaiting();
    //ovo je u stvari thread_dispatch(), ali da ne bi ovde pozivao c api funkcije
    Riscv::load_a0((uint64)Riscv::syscallCodes::thread_dispatch);
    __asm__ volatile("ecall;");

}

void TCB::dispatch(bool flag) {

    if (TCB::schedulerHead == nullptr) return;

    TCB* old = TCB::running;


    if ( !old->isFinished() && !flag) TCB::putScheduler(old);
    TCB::running = TCB::getScheduler();

    contextSwitch(old->context, TCB::running->context);
}

//vraca narednu nit u scheduleru
TCB* TCB::getScheduler() {
    TCB* temp = TCB::schedulerHead;
    TCB::schedulerHead = temp->next;
    temp->next = nullptr;
    if (TCB::schedulerHead == nullptr) TCB::schedulerTail = nullptr;
    return temp;
}

//stavlja nit u scheduler
void TCB::putScheduler(TCB *thread) {
    thread->next = nullptr; //nije potrebno jer se postavlja na nulu u getScheduler, ali za svaki slucaj
    if (TCB::schedulerHead == nullptr) TCB::schedulerHead = TCB::schedulerTail = thread;
    else {
        TCB::schedulerTail->next = thread;
        TCB::schedulerTail = thread;
    }
}

//stavlja nit u red niti koje cekaju da se nit nad kojom se poziva zavrsi ( koje su pozvale join nad tom niti )
void TCB::putWaiting(TCB *thread) {
    thread->next = nullptr;
    if ( waitingHead == nullptr) waitingHead = waitingTail = thread;
    else {
        waitingTail->next = thread;
        waitingTail = thread;
    }
}

//funkcija koja se poziva pri zavrsetku date niti da bi sve niti koje su cekale na njen zavrsetak
void TCB::emptyWaiting() {
    TCB* temp = waitingHead;

    while(temp != nullptr) {
        TCB* nextTemp = temp->next; //jer ce putScheduler da psotavi next na nullptr
        TCB::putScheduler(temp);  // u jednom trenutku se moze cekati samo na jednu nit pa mi ne treba nista dodatno, samo stavljam u Scheduler
        temp = nextTemp;
    }

    waitingHead = waitingTail = nullptr;
}

//stavlja nit koja se uspavljuje na odgovarajuce mjesto u nizu uspavanih niti
/*
 * prva nit u nizu sadrzi broj preostalih perioda tajmera koje su joj ostale, ostale niti sadrze relativno vrijeme na koje su uspavane u odnosu na nit koja je ispred njiih u nizu
 * prvi svakom prekidu od tajmera vrijeme u prvoj niti se smanjuje za jedan, i kada to vrijeme dostigne nulu, ta nit se odblokira ( stavlja u scheduler ), a ostale niti se ne mijenjaju ( samo se naravno pomjera glava na narednu nit i eventualno oslobadjaju te naredne niti ako su imale isto vrijeme cekanja kao i prva nit, odnosno ako im je vrijednost u timeLEft bila 0)
 */
void TCB::putSleeping(TCB *thread) {

    thread->next = nullptr;
    if ( sleepingHead == nullptr) sleepingHead = thread;
    else {
        TCB* temp = sleepingHead, *prev = nullptr;
        while ( temp != nullptr && thread->timeLeft >= temp->timeLeft  ) {
            thread->timeLeft -= temp->timeLeft;
            prev = temp;
            temp = temp->next;
        }

        if ( temp == nullptr) prev->next = thread;
        else {
            thread->next = temp;
            if ( prev) prev->next = thread;
            else sleepingHead = thread;

            temp->timeLeft -= thread->timeLeft;
        }
    }

}


//vraca kontekst - korisit se pri cuvanju/restauraciji konteksta kod prekida
uint64* TCB::getContext() {
    return TCB::running->context;
}

void TCB::yield()
{
    __asm__ volatile ("ecall");
}

void TCB::idleThreadBody(void*) {
    while(true) {
        Riscv::load_a0((uint64)Riscv::syscallCodes::thread_dispatch);
        __asm__ volatile("ecall;");
    };
}