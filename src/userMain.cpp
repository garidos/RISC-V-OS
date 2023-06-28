#include "../h/syscall_c.hpp"
#include "../h/syscall_cpp.hpp"
#include "../h/printing.hpp"

class Moja : public Thread {
    virtual void run() override {
        for ( int i = 0; i < 10; i++) {
            Console::putc('M');
            Console::putc('\n');
            sleep(10);
        }
    }
};

class Per : public PeriodicThread {
public:
    Per() : PeriodicThread(10) {};
protected:
    virtual void periodicActivation() {
        printString("OP\n");
    }
};

void userMain() {
    /*
    Moja *t = new Moja();
    t->start();

    t->join();*/

    Per *p = new Per();
    p->start();

    Thread::sleep(51);

    p->terminate();

    //Thread::sleep(20);

    printString("OS Projekat\n");


    /*
    uint64 volatile sepc;
    __asm__ volatile ("csrr %[sepc], sepc" : [sepc] "=r"(sepc));*/


}