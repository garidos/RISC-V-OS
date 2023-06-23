//
// Created by os on 6/22/23.
//

#include "../h/syscall_c.hpp"
#include "../h/syscall_cpp.hpp"

class Moja : public Thread {
    virtual void run() override {
        for ( int i = 0; i < 10; i++) {
            Console::putc('M');
            Console::putc('\n');
            sleep(20);
        }
    }
};

void userMain() {

    Moja *t = new Moja();
    t->start();

    t->join();
}