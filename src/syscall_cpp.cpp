//
// Created by os on 6/23/23.
//
#include "../h/syscall_cpp.hpp"

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

void PeriodicThread::terminate() {
    //Thread::~Thread();
};

void PeriodicThread::periodicThreadWrapper(void *arg) {
    PeriodicThread* thread = (PeriodicThread*)arg;
    while(true) {
        thread->periodicActivation();
        thread->sleep(thread->period);
    }
}

PeriodicThread::PeriodicThread(time_t period) : Thread(PeriodicThread::periodicThreadWrapper, this) {
    this->period = period;
};