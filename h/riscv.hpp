//
// Created by marko on 20.4.22..
//

//preuzeto sa ps vjezbi

#ifndef OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_RISCV_HPP
#define OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_RISCV_HPP

#include "../lib/hw.h"

class Riscv
{
public:

    //kodovi sistemskih poziva iz C API-ja koji se koriste pri pozivu sistemskih poziva iz ABI-ja
    enum syscallCodes {
        mem_alloc = 0x01, mem_free = 0x02, thread_create = 0x11, thread_exit = 0x12, thread_dispatch = 0x13, thread_join = 0x14,
        sem_open = 0x21, sem_close = 0x22, sem_wait = 0x23, sem_signal = 0x24, time_sleep = 0x31, getc = 0x41, putc = 0x42, thread_just_create = 0x51, thread_start = 0x52
    };

    static void load_a0(uint64 value) {
        __asm__ volatile("mv a0, %0" : : "r" (value));
    }

    static void load_a1(uint64 value) {
        __asm__ volatile("mv a1, %0" : : "r" (value));
    }

    static void load_a2(uint64 value) {
        __asm__ volatile("mv a2, %0" : : "r" (value));
    }

    static void load_a3(uint64 value) {
        __asm__ volatile("mv a3, %0" : : "r" (value));
    }

    static void load_a4(uint64 value) {
        __asm__ volatile("mv a4, %0" : : "r" (value));
    }


    // vraca se iz sistemskog rezima cime popuje spp i spie bite sstatus registra, pa se u njoj mora postaviti i sepc
    static void returnFromSMode();

    // read register scause
    static uint64 r_scause();

    // write register scause
    static void w_scause(uint64 scause);

    // read register sepc
    static uint64 r_sepc();

    // write register sepc
    static void w_sepc(uint64 sepc);

    // read register stvec
    static uint64 r_stvec();

    // write register stvec
    static void w_stvec(uint64 stvec);

    // read register stval
    static uint64 r_stval();

    // write register stval
    static void w_stval(uint64 stval);

    enum BitMaskSip
    {
        SIP_SSIP = (1 << 1),
        SIP_STIP = (1 << 5),
        SIP_SEIP = (1 << 9),
    };

    // mask set register sip
    static void ms_sip(uint64 mask);

    // mask clear register sip
    static void mc_sip(uint64 mask);

    // read register sip
    static uint64 r_sip();

    // write register sip
    static void w_sip(uint64 sip);

    // mask set register sie
    static void ms_sie(uint64 mask);

    // mask clear register sie
    static void mc_sie(uint64 mask);

    // read register sie
    static uint64 r_sie();

    // write register sie
    static void w_sie(uint64 sie);

    enum BitMaskSstatus
    {
        SSTATUS_SIE = (1 << 1),
        SSTATUS_SPIE = (1 << 5),
        SSTATUS_SPP = (1 << 8),
    };

    // mask set register sstatus
    static void ms_sstatus(uint64 mask);

    // mask clear register sstatus
    static void mc_sstatus(uint64 mask);

    // read register sstatus
    static uint64 r_sstatus();

    // write register sstatus
    static void w_sstatus(uint64 sstatus);

    // tabela prekidnih rutina
    static void vectorTable();

    // rutine na koje se skace iz tabele prekidnih rutina
    // one cuvaju kontekst i pozivaju odgovarajuci rukovalac nakon cega ucitavaju novi kontekst
    // ???? da li onda za prekid koji izaziva konzola treba uopste da se cuva i restauira kontekst ( i ako kod ostala dva nit ostane ista nakon rukovaoca)????
    static void trapSupervisorSoftwareInterrupt();
    static void trapSupervisorExternalInterrupt();
    static void trapExceptions();

private:

    // rukovaoci za razlicite vrste prekida i izuzetke
    static void handleSupervisorSoftwareInterrupt();
    static void handleSupervisorExternalInterrupt();
    static void handleExceptions();



};

inline uint64 Riscv::r_scause()
{
    uint64 volatile scause;
    __asm__ volatile ("csrr %[scause], scause" : [scause] "=r"(scause));
    return scause;
}

inline void Riscv::w_scause(uint64 scause)
{
    __asm__ volatile ("csrw scause, %[scause]" : : [scause] "r"(scause));
}

inline uint64 Riscv::r_sepc()
{
    uint64 volatile sepc;
    __asm__ volatile ("csrr %[sepc], sepc" : [sepc] "=r"(sepc));
    return sepc;
}

inline void Riscv::w_sepc(uint64 sepc)
{
    __asm__ volatile ("csrw sepc, %[sepc]" : : [sepc] "r"(sepc));
}

inline uint64 Riscv::r_stvec()
{
    uint64 volatile stvec;
    __asm__ volatile ("csrr %[stvec], stvec" : [stvec] "=r"(stvec));
    return stvec;
}

inline void Riscv::w_stvec(uint64 stvec)
{
    __asm__ volatile ("csrw stvec, %[stvec]" : : [stvec] "r"(stvec));
}

inline uint64 Riscv::r_stval()
{
    uint64 volatile stval;
    __asm__ volatile ("csrr %[stval], stval" : [stval] "=r"(stval));
    return stval;
}

inline void Riscv::w_stval(uint64 stval)
{
    __asm__ volatile ("csrw stval, %[stval]" : : [stval] "r"(stval));
}

inline void Riscv::ms_sip(uint64 mask)
{
    __asm__ volatile ("csrs sip, %[mask]" : : [mask] "r"(mask));
}

inline void Riscv::mc_sip(uint64 mask)
{
    __asm__ volatile ("csrc sip, %[mask]" : : [mask] "r"(mask));
}

inline uint64 Riscv::r_sip()
{
    uint64 volatile sip;
    __asm__ volatile ("csrr %[sip], sip" : [sip] "=r"(sip));
    return sip;
}

inline void Riscv::w_sip(uint64 sip)
{
    __asm__ volatile ("csrw sip, %[sip]" : : [sip] "r"(sip));
}

//sets bits in sstatus register using the given mask
inline void Riscv::ms_sstatus(uint64 mask)
{
    __asm__ volatile ("csrs sstatus, %[mask]" : : [mask] "r"(mask));
}

//clears bits in sstatus register using the given mask
inline void Riscv::mc_sstatus(uint64 mask)
{
    __asm__ volatile ("csrc sstatus, %[mask]" : : [mask] "r"(mask));
}

inline uint64 Riscv::r_sstatus()
{
    uint64 volatile sstatus;
    __asm__ volatile ("csrr %[sstatus], sstatus" : [sstatus] "=r"(sstatus));
    return sstatus;
}

inline void Riscv::w_sstatus(uint64 sstatus)
{
    __asm__ volatile ("csrw sstatus, %[sstatus]" : : [sstatus] "r"(sstatus));
}

inline void Riscv::ms_sie(uint64 mask)
{
    __asm__ volatile ("csrs sie, %[mask]" : : [mask] "r"(mask));
}

inline void Riscv::mc_sie(uint64 mask)
{
    __asm__ volatile ("csrc sie, %[mask]" : : [mask] "r"(mask));
}

inline uint64 Riscv::r_sie()
{
    uint64 volatile sie;
    __asm__ volatile ("csrr %[sie], sie" : [sie] "=r"(sie));
    return sie;
}

inline void Riscv::w_sie(uint64 sie)
{
    __asm__ volatile ("csrw sie, %[sie]" : : [sie] "r"(sie));
}




#endif //OS1_VEZBE07_RISCV_CONTEXT_SWITCH_2_INTERRUPT_RISCV_HPP
