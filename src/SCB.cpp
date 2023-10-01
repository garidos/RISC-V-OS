#include "../h/SCB.hpp"

SCB* SCB::create(int i) {
    return new SCB(i);
}

void SCB::signal() {

    if ( ++val <= 0 ) unblock();

}

void SCB::wait() {

    if ( --val < 0 ) block();

}

void SCB::block() {

    TCB::running->next = nullptr;
    if ( blockedHead == nullptr) blockedHead = blockedTail = TCB::running;
    else {
        blockedTail->next = TCB::running;
        blockedTail = blockedTail->next;
    }

    TCB::timeSliceCounter = 0;
    TCB::dispatch(true);

}

void SCB::unblock() {

    if ( blockedHead == nullptr) return;

    TCB* temp = blockedHead;
    blockedHead = blockedHead->next;
    if ( blockedHead == nullptr) blockedTail = nullptr;

    TCB::putScheduler(temp);

}

void SCB::release() {

    TCB* temp = blockedHead;

    while ( temp != nullptr) {
        TCB* nextTemp = temp->next;
        TCB::putScheduler(temp);
        temp = nextTemp;
    }

    blockedHead = blockedTail = nullptr;
    isActive = false;

}

void* SCB::operator new(size_t size) {
    return MemoryAllocator::malloc(size);
}

void SCB::operator delete (void* pointer) {
    MemoryAllocator::free(pointer);
}