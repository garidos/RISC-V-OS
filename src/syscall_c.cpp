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

    if ( handle == nullptr) Riscv::load_a1((uint64) handle);
}