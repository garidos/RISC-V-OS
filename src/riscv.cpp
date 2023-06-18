//
// Created by marko on 20.4.22..
//

#include "../h/riscv.hpp"
#include "../h/TCB.hpp"
#include "../lib/hw.h"
#include "../lib/console.h"

/*
 * u jedan asm fajl sam stavio sve trap rutine koje cuvaju kontekst i pozivaju odgovarajuci handler
 * pokusao sam da u tabeli vektora odmah stavim adrese hendlera koji bi onda prvo sacuvali kontekst pa se izvrsavali
 * ovo nije moglo(moglo je ali bi bilo dosta teze) jer bi se registri cuvali pomocu odvojenih funkcija(ono za cuvanje registra
 * u riscv asembleru ne moze u inline c asembleru iz nekog razloga) i onda bi pozivi tih funkcija poremetili stek
 * i jos neke registre koji spadaju u trenutni kontekst, pa mi je zato najbolje bilo da odmah direktno idem u trap rutine
 * gdje cu sacuvati netaknut kontekst a onda pozvati handler
 */

void Riscv::handleSupervisorSoftwareInterrupt()
{



}

void Riscv::handleSupervisorExternalInterrupt()
{

   console_handler();
}

void Riscv::handleExceptions()
{
    uint64 volatile sepc = r_sepc() + 4;
    uint64 volatile sstatus = r_sstatus();

    Thread::timeSliceCounter = 0;
    Thread::dispatch();


    w_sstatus(sstatus);
    w_sepc(sepc);

}


void Riscv::returnFromSMode()
{
    __asm__ volatile("csrw sepc, ra");
    __asm__ volatile("sret");
}
