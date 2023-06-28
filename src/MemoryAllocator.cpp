#include "../h/MemoryAllocator.hpp"

MemoryAllocator::FreeSegment* MemoryAllocator::freeMemHead = nullptr;

/*
 *                          _________________________
 *start ->                  |size___________________| <- BlockHeader
 *				            |_______________________| <- unused memory
 *start + sizeof(MBS) ->	|		                | <- start of allocated block
 *				            |_______________________|
 *				            |		                |
 *				            |_______________________|
 *				            |		                |
 *				            |_______________________|
 *				            |		                |
 *				            |_______________________|
 *				            |		                |
 *				            |_______________________|
 *				            |		                |
 *				            |_______________________|
 *				            |		                |
 *				            |_______________________|
 *				            |		                |
 *				            |_______________________|
 *				            |		                |
 *				            |_______________________|
 *				            |		                |
 *				            |_______________________|
 *
 *
 */

void* MemoryAllocator::malloc(size_t size) {

    if ( freeMemHead == nullptr) {

        freeMemHead = (FreeSegment*)(((uint64)HEAP_START_ADDR + MEM_BLOCK_SIZE - 1) & ~(MEM_BLOCK_SIZE - 1) );
        freeMemHead->size = (char*)HEAP_END_ADDR - (char*)HEAP_START_ADDR;
        freeMemHead->next = (FreeSegment*)HEAP_END_ADDR;

    }

    if (freeMemHead == HEAP_END_ADDR) return nullptr; //puna memorija

    size_t sizeInBlocks = (size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    sizeInBlocks+= 1; // dodatni blok za header ( znam da ce biti samo jedan jer je velicina headera jednaka minimalnoj velicini bloka )
    size_t realSize = sizeInBlocks * MEM_BLOCK_SIZE;

    FreeSegment* temp = freeMemHead, *prev = nullptr;

    while( temp != HEAP_END_ADDR) {
        if ( realSize <= temp->size) break;
        prev = temp;
        temp = temp->next;
    }

    if ( temp == HEAP_END_ADDR ) return nullptr; // ne postoji dovoljno velik slobodan blok

    if ( sizeof(FreeSegment) > MEM_BLOCK_SIZE && realSize < sizeof(FreeSegment) ) realSize += sizeof(FreeSegment);
    if ( realSize % MEM_BLOCK_SIZE ) realSize = (realSize + MEM_BLOCK_SIZE - 1) & ~( MEM_BLOCK_SIZE - 1 ); // zaokruzivanje na MEM_BLOCK_SIZE

    if ( temp->size - realSize < sizeof(FreeSegment)) {
        if (prev) prev->next = temp->next;
        else freeMemHead = temp->next;
        temp->size = temp->size - sizeof(BlockHeader);
        return (char*)temp + MEM_BLOCK_SIZE;
    } else {
        FreeSegment* newSegment = (FreeSegment*)((char*)temp + realSize);
        newSegment->next = temp->next;
        if (prev) prev->next = newSegment;
        else freeMemHead = newSegment;
        newSegment->size = temp->size - realSize;
        temp->size = realSize - sizeof(BlockHeader);
        return (char*)temp + MEM_BLOCK_SIZE;
    }


}

int MemoryAllocator::free(void *addr) {

    if ( (char*)addr < (char*)HEAP_START_ADDR || (char*)addr > (char*)HEAP_END_ADDR ) return -1;

    char* realAdr = (char*)addr - MEM_BLOCK_SIZE;

    FreeSegment *temp = freeMemHead, *prev = nullptr;

    while( temp != HEAP_END_ADDR) {
        if ( realAdr < (char*)temp ) break;
        if ( realAdr < (char*)temp + temp->size) return -1;
        prev = temp;
        temp = temp->next;
    }

    FreeSegment* curr = nullptr;

    if ( prev != nullptr && (char*)prev + prev->size == realAdr) {
        prev->size += ((BlockHeader*)realAdr)->size + sizeof(BlockHeader);
        curr = prev;

    } else {
        curr = (FreeSegment*)realAdr;
        curr->size+= sizeof(BlockHeader);
        if (prev) prev->next = curr;
        else freeMemHead = curr;

        curr->next = temp;
    }

    if ( (char*)curr + curr->size == (char*)temp) {

        curr->size += temp->size;
        curr->next = temp->next;
    }


    return 0;
}