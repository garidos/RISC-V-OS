#include "../h/syscall_cpp.hpp"

void* operator new(size_t size) {
    return mem_alloc(size);
}

void* operator new[](size_t size) {
    return mem_alloc(size);
}

void operator delete(void* pointer) noexcept {
    mem_free(pointer);
}

void operator delete[](void*pointer) noexcept {
    mem_free(pointer);
}

Thread::Thread(void (*body)(void*), void* arg){
    this->body = body;
    this->arg = arg;
    thread_just_create(&myHandle, body, arg);
}

Thread::~Thread () {
    myHandle->setFinished(true);
    delete myHandle;
};

int Thread::start () {
    if (myHandle == nullptr) return -1;
    thread_start(myHandle);
    return 0;
}

void Thread::join() {
    thread_join(myHandle);
}

void Thread::dispatch () {
    thread_dispatch();
};

int Thread::sleep(time_t time) {
    return time_sleep(time);
};

void Thread::threadWrapper(void* arg) {
    Thread *thread = (Thread*)arg;
    thread->run();
};

Thread::Thread() {
    thread_just_create(&myHandle, Thread::threadWrapper, this);
};


Semaphore::Semaphore(unsigned int init) {
    sem_open(&myHandle, init);
};
Semaphore::~Semaphore() {
    sem_close(myHandle);
};

int Semaphore::wait() {
    return sem_wait(myHandle);
};
int Semaphore::signal() {
    return sem_signal(myHandle);
};

char Console::getc() {
    return ::getc();
};

void Console::putc(char c) {
    ::putc(c);
};

/*
 * s obzirom da nemamo pristup handleru niti smijemo dodavati nestaticka polja u klasu, morao sam na ovaj nacin da uradim
 * period se postavlja pri stvaranju objekta ove klase, i ne postoji nacin da se promjeni nakon toga
 * ja sam ovo iskoristio da bih realizovao operaciju terminate, odnosno gasenje periodicne niti, tako sto bih u terminate promjenio vrijednost perioda ( nije bitno na sta)
 * a u wrapperu bih se vrtio sve dok je period ne promjenjen, nakon cega bi se zavrsio rad niti
 */
void PeriodicThread::terminate() {
    period++;
    //this->join();
    //moze npr da se desi, da se tik pred kraj userMaina pozove terminate, nit ostaje i dalje uspavana, a sistem izlazi iz userMaina i zavrsava svoj rad, zatim se uspavana nit budi i pokusava da uradi nesto sto izaziva nelegalan izuzetak
    //pa da se ovo ne bi desilo ceka se da se nit probudi i skroz zavrsi
    //jako su male sanse da se ovo desi, ali eto
};

void PeriodicThread::periodicThreadWrapper(void *arg) {
    PeriodicThread* thread = (PeriodicThread*)arg;
    time_t startPeriod = thread->period;
    while(startPeriod == thread->period) {
        thread->periodicActivation();
        thread->sleep(thread->period);
    }
}

PeriodicThread::PeriodicThread(time_t period) : Thread(PeriodicThread::periodicThreadWrapper, this) {
    this->period = period;
};