//
// Created by marko on 20.4.22..
//

#include "../h/riscv.hpp"
#include "../h/TCB.hpp"
#include "../h/SCB.hpp"
#include "../lib/hw.h"
#include "../lib/console.h"
#include "../h/_console.hpp"

/*
 * u jedan asm fajl sam stavio sve trap rutine koje cuvaju kontekst i pozivaju odgovarajuci handler
 * pokusao sam da u tabeli vektora odmah stavim adrese hendlera koji bi onda prvo sacuvali kontekst pa se izvrsavali
 * ovo nije moglo(moglo je ali bi bilo dosta teze) jer bi se registri cuvali pomocu odvojenih funkcija(ono za cuvanje registra
 * u riscv asembleru ne moze u inline c asembleru iz nekog razloga) i onda bi pozivi tih funkcija poremetili stek
 * i jos neke registre koji spadaju u trenutni kontekst, pa mi je zato najbolje bilo da odmah direktno idem u trap rutine
 * gdje cu sacuvati netaknut kontekst a onda pozvati handler
 */

//prekidna rutina tajmera koji izaziva softverski prekid
void Riscv::handleSupervisorSoftwareInterrupt()
{
    mc_sip(SIP_SSIP);

    if (TCB::running == nullptr) return;

    if ( TCB::sleepingHead != nullptr ) {
        TCB::sleepingHead->timeLeft--;

        while ( TCB::sleepingHead != nullptr && TCB::sleepingHead->timeLeft == 0) {
            TCB* temp = TCB::sleepingHead;
            TCB::sleepingHead = TCB::sleepingHead->next;
            TCB::putScheduler(temp);
        }
    }

    TCB::timeSliceCounter++;
    if (TCB::timeSliceCounter >= TCB::running->getTimeSlice()  ) {

        uint64 volatile sepc = r_sepc();
        uint64 volatile sstatus = r_sstatus();

        TCB::timeSliceCounter = 0;
        TCB::dispatch(false);

        w_sstatus(sstatus);
        w_sepc(sepc);

    }


}

//prekidna rutina kontrolera periferije koji izaziva spoljasnji prekid
void Riscv::handleSupervisorExternalInterrupt()
{
    mc_sip(SIP_SEIP);
    int volatile code = plic_claim();

    if ( code == 10) {

        //if ( *(char*)CONSOLE_STATUS & CONSOLE_TX_STATUS_BIT) {}

        //kada god kontroler izazove prekid citam karaktere iz nejgovog data registra sve dok je odgovarajuci statusni bit setovan
        while ( *(char*)CONSOLE_STATUS & CONSOLE_RX_STATUS_BIT && !CCB::inputBuffer->isFull()) {      //can write
            char volatile c = *((char*)CONSOLE_RX_DATA);
            //if ( c == 13) break;
            CCB::inputBuffer->put(c);
            CCB::readyToRead->signal();
        }

    }

    plic_complete(code);



   //console_handler();
}

//prekidna rutina za izuzetke - prije svega za ecall
void Riscv::handleExceptions()
{
    uint64 volatile sepc = r_sepc() + 4;
    uint64 volatile sstatus = r_sstatus();
    uint64 volatile sscause = r_scause();

    //nit se blokira ako dodje do nekog ne ocekivanog izuzetka
    if ( sscause != 8 && sscause != 9) {
        while(true);
    }

    uint64 volatile code = TCB::running->context[TCB::registerOffs::a0Offs];

    switch (code) {
        case syscallCodes::thread_create: {

            TCB** volatile handle = (TCB**)TCB::running->context[TCB::registerOffs::a1Offs];
            Body volatile body = (Body)TCB::running->context[TCB::registerOffs::a2Offs];
            void* volatile arg = (void*)TCB::running->context[TCB::registerOffs::a3Offs];
            uint64* volatile stack = (uint64*)TCB::running->context[TCB::registerOffs::a4Offs];

            *handle = TCB::create(body, arg, stack);

            //ako je nit napravljena iz korisnickog rezima, onda i ona treba da bude u korisnickom rezimu, pa se postavlja polje setUser na osnovu koga ce se podesiti sstatus pri startovanju niti
            if ( (sstatus & Riscv::SSTATUS_SPP) == 0) (*handle)->setUser = true;

            int res = 0;
            if ( *handle == nullptr) res = -3;
            //load_a0((uint64) res);
            TCB::running->context[TCB::registerOffs::a0Offs] = res;

            break;
        }

        case syscallCodes::thread_exit: {

            TCB::running->setFinished(true);
            TCB::running->emptyWaiting();

            TCB::timeSliceCounter = 0;
            TCB::dispatch(false);

            //ako se ispravno izvrsi nikada nece doci do ovog dijela

            TCB::running->context[TCB::registerOffs::a0Offs] = 1;

            break;
        }

        case syscallCodes::thread_dispatch: {

            TCB::timeSliceCounter = 0;
            TCB::dispatch(false);

            break;
        }

        case syscallCodes::thread_join: {

            TCB* volatile thread = (TCB*)TCB::running->context[TCB::registerOffs::a1Offs];

            //nema logike da se ceka na neki nit ako se ona i ni jedna druga ne izvrsavaju
            if (TCB::schedulerHead != nullptr) {

                thread->putWaiting(TCB::running);
                TCB::timeSliceCounter = 0;
                TCB::dispatch(true);
                /*
                 * iz nekog razloga se promjene konteksta unutar prekida pravilno izvrsavaju samo ako se eksplicitno pozove dispatch koji ce to raditi
                 * u suprotnom se poremeti s0 ( frame pointer ) i ne izvrsava se pravilno
                 * ovo je i logicno, ali mi nije jasno zasto se pravilno izvrsava kada se poziva dispatch...
                 * vjerovatno se pri povratku iz dispatch frame pointer vraca na osnovu sp ( koji je ispravan )
                 */
               /* TCB *old = TCB::running;
                TCB::running = TCB::getScheduler();

                TCB::contextSwitch(old->context, TCB::running->context);*/
            }

            break;
        }

        case syscallCodes::sem_open: {

            SCB** volatile handle = (SCB**)TCB::running->context[TCB::registerOffs::a1Offs];
            unsigned volatile init = (unsigned)TCB::running->context[TCB::registerOffs::a2Offs];

            *handle = SCB::create((int)init);

            int res = 0;
            if ( *handle == nullptr) res = -2;

            TCB::running->context[TCB::registerOffs::a0Offs] = res;

            break;
        }

        case syscallCodes::sem_close: {

            SCB* volatile handle = (SCB*)TCB::running->context[TCB::registerOffs::a1Offs];

            handle->release();

            TCB::running->context[TCB::registerOffs::a0Offs] = 0;

            break;
        }

        case syscallCodes::sem_wait: {

            SCB* volatile id = (SCB*)TCB::running->context[TCB::registerOffs::a1Offs];

            id->wait();

            int res = 0;
            //ako je semafor dealociran dok je nit bila blokirana na njemu
            if ( !id->active() ) res = -2;

            TCB::running->context[TCB::registerOffs::a0Offs] = res;

            break;
        }

        case syscallCodes::sem_signal: {

            SCB* volatile id = (SCB*)TCB::running->context[TCB::registerOffs::a1Offs];

            id->signal();

            TCB::running->context[TCB::registerOffs::a0Offs] = 0;

            break;
        }

        case syscallCodes::time_sleep: {

            time_t time = (time_t)TCB::running->context[TCB::registerOffs::a1Offs];

            TCB::running->timeLeft = time;
            TCB::putSleeping(TCB::running);

            TCB::dispatch(true);

            TCB::running->context[TCB::registerOffs::a0Offs] = 0;

            break;
        }

        case syscallCodes::getc: {

            CCB::readyToRead->wait();

            char volatile c = CCB::inputBuffer->get();

            //load_a0((uint64)c);
            TCB::running->context[TCB::registerOffs::a0Offs] = c;
            // da se a0 ne bi slucajno promjenilo do trenutka restauracije, ovako ga upisem u sacuvani kontekst i znam da ce ispravna vrijednost da bude u a0 kada se restauira u trap rutini

            break;
        }

        case syscallCodes::putc: {

            char volatile c = (char)TCB::running->context[TCB::registerOffs::a1Offs];

            // TODO: Prijaviti gresku ako je izlazni bafer pun
            if ( CCB::outputBuffer->isFull());  //treba 'prijaviti gresku'
            else {
                CCB::outputBuffer->put(c);
                CCB::cnt++;
            }

            break;
        }
    }

    w_sstatus(sstatus);
    w_sepc(sepc);

}


void Riscv::returnFromSMode()
{
    __asm__ volatile("csrw sepc, ra");
    __asm__ volatile("sret");
}
