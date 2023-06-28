#include "../h/riscv.hpp"
#include "../h/TCB.hpp"
#include "../h/SCB.hpp"
#include "../h/_console.hpp"
#include "../h/MemoryAllocator.hpp"


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

    //ovaj dio je ispao isti kao onaj sa vejzbi
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
        Riscv::handleIllegalException(sscause);
    }

    uint64 volatile code = TCB::running->context[TCB::registerOffs::a0Offs];

    switch (code) {
        case syscallCodes::mem_alloc: {

            size_t volatile sizeInBlocks = (size_t)TCB::running->context[TCB::registerOffs::a1Offs];

            void* volatile pointer = MemoryAllocator::malloc(sizeInBlocks * MEM_BLOCK_SIZE);

            TCB::running->context[TCB::registerOffs::a0Offs] = (uint64)pointer;

            break;
        }

        case syscallCodes::mem_free: {

            void* volatile pointer = (void*)TCB::running->context[TCB::registerOffs::a1Offs];

            TCB::running->context[TCB::registerOffs::a0Offs] = MemoryAllocator::free(pointer);

            break;
        }

        case syscallCodes::thread_create: {

            TCB** volatile handle = (TCB**)TCB::running->context[TCB::registerOffs::a1Offs];
            Body volatile body = (Body)TCB::running->context[TCB::registerOffs::a2Offs];
            void* volatile arg = (void*)TCB::running->context[TCB::registerOffs::a3Offs];
            uint64* volatile stack = (uint64*)TCB::running->context[TCB::registerOffs::a4Offs];

            *handle = TCB::create(body, arg, stack);

            //ako je nit napravljena iz korisnickog rezima, onda i ona treba da bude u korisnickom rezimu, pa se postavlja polje setUser na osnovu koga ce se podesiti sstatus pri startovanju niti
            //ukoliko se ovo ne uradi, kada ova nova nit dobije procesor, ona ce 'preuzeti' sstatus niti koja se izvrsavala prije nje, jer se ni na koji drugi nacin ne postavlja sstatus pri prvljenju niti
            if ( (sstatus & Riscv::SSTATUS_SPP) == 0) {
                (*handle)->setUser = true;
                TCB::numOfUserThreads++;
            }

            int res = 0;
            if ( *handle == nullptr) res = -3;
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

            //nema logike da se ceka na neki nit ako se ona i ni jedna druga ne izvrsavaju ( nije botrebno otkad sam dodao idle )
            if (TCB::schedulerHead != nullptr) {

                thread->putWaiting(TCB::running);
                TCB::timeSliceCounter = 0;
                TCB::dispatch(true);
                /*
                 * promjena kontesta mora da se uradi unutar funkcije, ne moze direktno ovde
                 * jer se pri promjeni zamjenjuje ra (koje je prtethodno sacuvano u prekidnoj rutini) i izvrsava instrukcija ret ( koja vraca pc na tu vrijednost u ra )
                 * kada se udje u funkciju ra ce pokazivati ovde, odmah posle funkcije - kada se izvrsi promjena vratice se ovde i pravilno izvrsiti ostatak ( upis u sepc i sstatus )
                 * u suprotnom, u ra ce ili biti neka vrijednost koaj je ostala od poziva neke funkcije prije ove, pa ce se tu vratiti, sto nije dobro
                 * ili ce biti ra koje odgovara pozivu samog hendlera, pri cemu ce se pri povratku potpuno rpeskociti upis u sepc i sstatus, sto ponovo nije dobro
                 */
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

            CCB::readyToWrite->wait();

            char volatile c = (char)TCB::running->context[TCB::registerOffs::a1Offs];

            CCB::outputBuffer->put(c);

            break;
        }

        case syscallCodes::thread_just_create: {
            TCB** volatile handle = (TCB**)TCB::running->context[TCB::registerOffs::a1Offs];
            Body volatile body = (Body)TCB::running->context[TCB::registerOffs::a2Offs];
            void* volatile arg = (void*)TCB::running->context[TCB::registerOffs::a3Offs];
            uint64* volatile stack = (uint64*)TCB::running->context[TCB::registerOffs::a4Offs];

            *handle = TCB::justCreate(body, arg, stack);

            //ako je nit napravljena iz korisnickog rezima, onda i ona treba da bude u korisnickom rezimu, pa se postavlja polje setUser na osnovu koga ce se podesiti sstatus pri startovanju niti
            //ukoliko se ovo ne uradi, kada ova nova nit dobije procesor, ona ce 'preuzeti' sstatus niti koja se izvrsavala prije nje, jer se ni na koji drugi nacin ne postavlja sstatus pri prvljenju niti
            if ( (sstatus & Riscv::SSTATUS_SPP) == 0) {
                (*handle)->setUser = true;
                TCB::numOfUserThreads++;
            }

            int res = 0;
            if ( *handle == nullptr) res = -3;
            //load_a0((uint64) res);
            TCB::running->context[TCB::registerOffs::a0Offs] = res;

            break;
        }

        case syscallCodes::thread_start: {

            TCB* volatile thread = (TCB*)TCB::running->context[TCB::registerOffs::a1Offs];

            TCB::putScheduler(thread);

            break;
        }
    }

    w_sstatus(sstatus);
    w_sepc(sepc);

}

//sa vjezbi
void Riscv::returnFromSMode()
{
    __asm__ volatile("csrw sepc, ra");
    __asm__ volatile("sret");
}

void Riscv::handleIllegalException(uint64 scause) {
    char const* part1 = "Kernel stopped!\nCause: ";
    char const* part2_cause = nullptr;
    switch(scause) {
        case 0: {
            part2_cause = "Instruction address misaligned\n";
            break;
        }
        case 1: {
            part2_cause = "Instruction access fault\n";
            break;
        }
        case 2: {
            part2_cause = "Illegal instruction\n";
            break;
        }
        case 3: {
            part2_cause = "Breakpoint\n";
            break;
        }
        case 4: {
            part2_cause = "Load address misaligned\n";
            break;
        }
        case 5: {
            part2_cause = "Load access fault\n";
            break;
        }
        case 6: {
            part2_cause = "Store address misaligned\n";
            break;
        }
        case 7: {
            part2_cause = "Store access fault\n";
            break;
        }
    }
    //ispis sam radio na ovaj nacin, jer u trenutku kada se ispisuje ova poruka znam da ce se sistem blokirati i samim tim nisu bitne stvari koje se nalaze ( pa necu preko consumer niti ici )
    //u output baferu, vec trebam samo direktno periferiji da saljem sta zelim da ipisem
    while ( *part1 != '\0') {
        if (*((char*)CONSOLE_STATUS) & CONSOLE_TX_STATUS_BIT) {
            *((char*)CONSOLE_TX_DATA) = *part1;
            part1++;
        }
    }
    while (*part2_cause != '\0') {
        if (*((char*)CONSOLE_STATUS) & CONSOLE_TX_STATUS_BIT) {
            *((char*)CONSOLE_TX_DATA) = *part2_cause;
            part2_cause++;
        }
    }
    while(true);
}