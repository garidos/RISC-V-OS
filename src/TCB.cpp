#include "../h/TCB.hpp"
#include "../h/riscv.hpp"
#include "../h/syscall_c.hpp"

//wrapperi i dispatch slicni kao na vjezbama

TCB* TCB::schedulerHead = nullptr;
TCB* TCB::schedulerTail = nullptr;
TCB* TCB::sleepingHead = nullptr;
TCB* TCB::running = nullptr;
uint64 TCB::timeSliceCounter = 0;
int TCB::numOfUserThreads = 0;

void userMain();

TCB* TCB::create(Body body, void *arg, uint64 *stack) {
    return new TCB(body, arg, stack, DEFAULT_TIME_SLICE, TCB::threadWrapper, true);
}

TCB* TCB::createAndSwitchToUser(uint64 *stack) {
    // nije bitno sta se stavi u body (samo ako stavim nullptr moram da promjenim uslov za stavljanje u scheduler u konstruktoru), jer ce se userMain pozivati direktno
    return new TCB(nullptr, nullptr, stack, DEFAULT_TIME_SLICE, TCB::userMainWrapper, true);
}

//pravi nit bez startovanja ( stavljanja u scheduler )
TCB* TCB::justCreate(Body body, void *arg, uint64 *stack) {
    return new TCB(body, arg, stack, DEFAULT_TIME_SLICE, TCB::threadWrapper, false);
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

    if ( TCB::running->setUser) TCB::numOfUserThreads--;

    thread_dispatch();
}

void TCB::userMainWrapper() {
    //postavlja SPP bit u sstatus na 0, kako bi se pri povratku iz prekidne rutine preslo u koriscnicki rezim
    Riscv::mc_sstatus(Riscv::SSTATUS_SPP);
    Riscv::returnFromSMode();
    //poziv stvarnog tijela niti
    userMain();

    //nit je zavrsena
    TCB::running->setFinished(true);

    TCB::running->emptyWaiting();

    thread_dispatch();
}

//flag = true - ne pokusava se dodavanje niti u scheduler (koristi se npr kod niti koje se uspavljuju ili blokiraju na semaforu, pri cemu se vrsi promjena konteksta, a stara nit se ne stavlaj na scheduler )
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

void TCB::idleThreadBody(void*) {
    while(true) {
        thread_dispatch();
    };
}

void* TCB::operator new(size_t size) {
    return MemoryAllocator::malloc(size);
}

void TCB::operator delete (void* pointer) {
    MemoryAllocator::free(pointer);
}