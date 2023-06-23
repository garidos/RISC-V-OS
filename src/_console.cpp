//
// Created by os on 6/22/23.
//

#include "../h/_console.hpp"
#include "../h/syscall_c.hpp"

int CCB::cap = 1000;
int CCB::cnt = 0;
Buffer *CCB::inputBuffer = nullptr;
Buffer *CCB::outputBuffer = nullptr;
SCB *CCB::readyToRead = nullptr;
TCB* CCB::consumer = nullptr;


//nit potrosac koja uzima karaktere iz output bafera i stavlja ih u data registar kontrolera pperiferije
//ova nit se vrti u beskonacnoj petlji i kad god je postavljen odgovarajuci statusni bit ona radi svoj posao
void CCB::outputThreadBody(void *) {

    while(true) {

        while(*((char*)CONSOLE_STATUS) & CONSOLE_TX_STATUS_BIT) {

            if(CCB::outputBuffer->isEmpty()) break;
            *((char*)CONSOLE_TX_DATA) = CCB::outputBuffer->get();
            CCB::cnt--;

        }
        thread_dispatch();
    }
}


void Buffer::put(char c) {

    if ( isEmpty()) head = tail = 0;
    arr[head] = c;
    head = ( head + 1 ) % capacity;

}

char Buffer::get() {

    char c;

    c = arr[tail];

    tail = ( tail + 1 ) % capacity;

    if ( tail == head ) tail = head = -1;

    return c;
}