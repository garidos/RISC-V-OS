//
// Created by os on 6/22/23.
//

#include "../h/syscall_c.hpp"

void userMain() {

    char c = getc();
    putc(c);
    putc('\n');

}