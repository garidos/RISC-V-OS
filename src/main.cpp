#include "../h/riscv.hpp"
#include "../h/TCB.hpp"
#include "../h/syscall_c.hpp"
#include "../h/_console.hpp"



int main() {

    //zabranjuju se prekidi dok se zavrsi potrebna inicijalizacija i upisuje se odgovarajuca vrijednost u stvec
    Riscv::mc_sstatus(Riscv::SSTATUS_SIE);
    Riscv::w_stvec((uint64) & Riscv::vectorTable | (uint64) 1);
    thread_t mainThread, idle, userMainThread;

    //prva nit mora da se napravi bez poziva prekida, jer ce se tu dohvatati kontekst running niti koja jos ne postoji
    //iz istog razloga ne moze ni new da se korisiti
    mainThread = TCB::create( nullptr, nullptr, (uint64*)MemoryAllocator::malloc(DEFAULT_STACK_SIZE * sizeof(uint64)));
    TCB::running = mainThread;

    thread_create(&idle, TCB::idleThreadBody, nullptr);

    CCB::inputBuffer = new consoleBuffer(CCB::cap);
    CCB::outputBuffer = new consoleBuffer(CCB::cap);
    sem_open(&CCB::readyToRead, 0);
    sem_open(&CCB::readyToWrite, CCB::cap);

    thread_create(&CCB::consumer, CCB::outputThreadBody, nullptr);

    Riscv::ms_sstatus(Riscv::SSTATUS_SIE);

    userMainThread = TCB::createAndSwitchToUser(new uint64[DEFAULT_STACK_SIZE]);


    thread_join(userMainThread);

    //ceka se da se obrade svi zahtjevi za ispis pa se onda zavrsava sistem
    while(!CCB::outputBuffer->isEmpty()) thread_dispatch();
    while(TCB::numOfUserThreads > 0) thread_dispatch();

    CCB::consumer->setFinished(true);
    sem_close(CCB::readyToWrite);
    sem_close(CCB::readyToRead);
    delete CCB::inputBuffer;
    delete CCB::outputBuffer;
    idle->setFinished(true);
    mainThread->setFinished(true);
    delete idle;
    delete CCB::consumer;
    delete userMainThread;
    delete mainThread;


    return 0;
}