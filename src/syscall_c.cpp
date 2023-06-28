#include "../h/syscall_c.hpp"
#include "../h/riscv.hpp"

void* mem_alloc (size_t size) {
    if ( size <= 0) return nullptr;

    size_t sizeInBlocks = ( size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    Riscv::load_a1((uint64)sizeInBlocks);
    Riscv::load_a0((uint64)Riscv::syscallCodes::mem_alloc);

    Riscv::ecall();
    return (void*)Riscv::return_a0();
}

int mem_free (void* ptr) {
    if ( ptr == nullptr) return -2;

    Riscv::load_a1((uint64)ptr);
    Riscv::load_a0((uint64)Riscv::syscallCodes::mem_free);

    Riscv::ecall();
    return (int)Riscv::return_a0();
}


int thread_create ( thread_t* handle, void(*start_routine)(void*), void* arg) {

    if ( handle == nullptr) return -1;

    //nije unutar prekdia, pa moze new da se koristi
    uint64 * stack = new uint64[DEFAULT_STACK_SIZE];
    if ( stack == nullptr ) return -2;

    Riscv::load_a4((uint64) stack);
    Riscv::load_a3((uint64) arg);
    Riscv::load_a2((uint64) start_routine);
    Riscv::load_a1((uint64) handle);
    Riscv::load_a0((uint64)Riscv::syscallCodes::thread_create);

    Riscv::ecall();
    return (int)Riscv::return_a0();
}

int thread_exit () {

    Riscv::load_a0((uint64)Riscv::syscallCodes::thread_exit);

    Riscv::ecall();
    return (int)Riscv::return_a0();
}

void thread_dispatch () {

    Riscv::load_a0((uint64)Riscv::syscallCodes::thread_dispatch);

    Riscv::ecall();
}

void thread_join (thread_t handle) {
    if ( handle == nullptr || handle == TCB::running || handle->isFinished()) return;

    Riscv::load_a1((uint64)handle);
    Riscv::load_a0((uint64)Riscv::syscallCodes::thread_join);

    Riscv::ecall();
}

int sem_open (sem_t* handle, unsigned init) {
    if ( handle == nullptr) return -1;

    Riscv::load_a2((uint64)init);
    Riscv::load_a1((uint64)handle);
    Riscv::load_a0((uint64)Riscv::syscallCodes::sem_open);

    Riscv::ecall();
    return (int)Riscv::return_a0();
}

int sem_close(sem_t handle) {
    if ( handle == nullptr || !handle->active()) return -1;

    Riscv::load_a1((uint64)handle);
    Riscv::load_a0((uint64)Riscv::syscallCodes::sem_close);

    Riscv::ecall();
    return (int)Riscv::return_a0();
}

int sem_wait (sem_t id) {
    if ( id == nullptr || !id->active()) return -1;

    Riscv::load_a1((uint64)id);
    Riscv::load_a0((uint64)Riscv::syscallCodes::sem_wait);

    Riscv::ecall();
    return (int)Riscv::return_a0();
}

int sem_signal (sem_t id) {

    if ( id == nullptr || !id->active()) return -1;

    Riscv::load_a1((uint64)id);
    Riscv::load_a0((uint64)Riscv::syscallCodes::sem_signal);

    Riscv::ecall();
    return (int)Riscv::return_a0();
}

int time_sleep (time_t time) {
    if ( time == 0) return 0;

    Riscv::load_a1((uint64)time);
    Riscv::load_a0((uint64)Riscv::syscallCodes::time_sleep);

    Riscv::ecall();
    return (int)Riscv::return_a0();
}


char getc () {

    Riscv::load_a0((uint64)Riscv::syscallCodes::getc);

    Riscv::ecall();
    return (char)Riscv::return_a0();
}


void putc (char c) {

    Riscv::load_a1((uint64)c);
    Riscv::load_a0((uint64)Riscv::syscallCodes::putc);

    Riscv::ecall();
}


int thread_just_create(thread_t* handle, void(*start_routine)(void*), void* arg) {
    if ( handle == nullptr) return -1;

    //nije unutar prekdia, pa moze new da se koristi
    uint64 * stack = new uint64[DEFAULT_STACK_SIZE];
    if ( stack == nullptr ) return -2;

    Riscv::load_a4((uint64) stack);
    Riscv::load_a3((uint64) arg);
    Riscv::load_a2((uint64) start_routine);
    Riscv::load_a1((uint64) handle);
    Riscv::load_a0((uint64)Riscv::syscallCodes::thread_just_create);

    Riscv::ecall();
    return (int)Riscv::return_a0();
}

void thread_start(thread_t thread) {
    if ( thread == nullptr || thread->isFinished()) return;

    Riscv::load_a1((uint64)thread);
    Riscv::load_a0((uint64)Riscv::syscallCodes::thread_start);

    Riscv::ecall();
}