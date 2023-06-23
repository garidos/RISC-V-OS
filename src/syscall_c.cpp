//
// Created by os on 6/19/23.
//

#include "../h/syscall_c.hpp"
#include "../h/riscv.hpp"


int thread_create ( thread_t* handle, void(*start_routine)(void*), void* arg) {

    if ( handle == nullptr) return -1;

    uint64 * stack = new uint64[DEFAULT_STACK_SIZE];
    if ( stack == nullptr ) return -2;

    Riscv::load_a4((uint64) stack);
    Riscv::load_a3((uint64) arg);
    Riscv::load_a2((uint64) start_routine);
    Riscv::load_a1((uint64) handle);
    Riscv::load_a0((uint64)Riscv::syscallCodes::thread_create);

    int res;
    __asm__ volatile("ecall;");
    __asm__ volatile("mv %0, a0" : "=r" (res));

    return res;
}

int thread_exit () {

    Riscv::load_a0((uint64)Riscv::syscallCodes::thread_exit);

    int res;
    __asm__ volatile("ecall;");
    __asm__ volatile("mv %0, a0" : "=r" (res));

    return res;
}

void thread_dispatch () {

    Riscv::load_a0((uint64)Riscv::syscallCodes::thread_dispatch);

    __asm__ volatile("ecall;");
}

void thread_join (thread_t handle) {
    if ( handle == nullptr || handle == TCB::running || handle->isFinished()) return;

    Riscv::load_a1((uint64)handle);
    Riscv::load_a0((uint64)Riscv::syscallCodes::thread_join);

    __asm__ volatile("ecall;");
}

int sem_open (sem_t* handle, unsigned init) {
    if ( handle == nullptr) return -1;

    Riscv::load_a2((uint64)init);
    Riscv::load_a1((uint64)handle);
    Riscv::load_a0((uint64)Riscv::syscallCodes::sem_open);

    int res;
    __asm__ volatile("ecall;");
    __asm__ volatile("mv %0, a0" : "=r" (res));

    return res;
}

int sem_close(sem_t handle) {
    if ( handle == nullptr || !handle->active()) return -1;

    Riscv::load_a1((uint64)handle);
    Riscv::load_a0((uint64)Riscv::syscallCodes::sem_close);

    int res;
    __asm__ volatile("ecall;");
    __asm__ volatile("mv %0, a0" : "=r" (res));

    return res;
}

int sem_wait (sem_t id) {

    if ( id == nullptr || !id->active()) return -1;

    Riscv::load_a1((uint64)id);
    Riscv::load_a0((uint64)Riscv::syscallCodes::sem_wait);

    int res;
    __asm__ volatile("ecall;");
    __asm__ volatile("mv %0, a0" : "=r" (res));

    return res;
}

int sem_signal (sem_t id) {

    if ( id == nullptr || !id->active()) return -1;

    Riscv::load_a1((uint64)id);
    Riscv::load_a0((uint64)Riscv::syscallCodes::sem_signal);

    int res;
    __asm__ volatile("ecall;");
    __asm__ volatile("mv %0, a0" : "=r" (res));

    return res;
}

int time_sleep (time_t time) {
    if ( time == 0) return 0;

    Riscv::load_a1((uint64)time);
    Riscv::load_a0((uint64)Riscv::syscallCodes::time_sleep);

    int res;
    __asm__ volatile("ecall;");
    __asm__ volatile("mv %0, a0" : "=r" (res));

    return res;
}


char getc () {

    Riscv::load_a0((uint64)Riscv::syscallCodes::getc);

    char res;
    __asm__ volatile("ecall;");
    __asm__ volatile("mv %0, a0" : "=r" (res));

    return res;

}

//TODO: napraviti funkciju za citanje a0 u neku promjenljivu u Riscv

void putc (char c) {

    Riscv::load_a1((uint64)c);
    Riscv::load_a0((uint64)Riscv::syscallCodes::putc);

    __asm__ volatile("ecall;");
}


int thread_just_create(thread_t* handle, void(*start_routine)(void*), void* arg) {
    if ( handle == nullptr) return -1;

    uint64 * stack = new uint64[DEFAULT_STACK_SIZE];
    if ( stack == nullptr ) return -2;

    Riscv::load_a4((uint64) stack);
    Riscv::load_a3((uint64) arg);
    Riscv::load_a2((uint64) start_routine);
    Riscv::load_a1((uint64) handle);
    Riscv::load_a0((uint64)Riscv::syscallCodes::thread_just_create);

    int res;
    __asm__ volatile("ecall;");
    __asm__ volatile("mv %0, a0" : "=r" (res));

    return res;
}

void thread_start(thread_t thread) {
    if ( thread == nullptr || thread->isFinished()) return;

    Riscv::load_a1((uint64)thread);
    Riscv::load_a0((uint64)Riscv::syscallCodes::thread_start);

    __asm__ volatile("ecall;");
}