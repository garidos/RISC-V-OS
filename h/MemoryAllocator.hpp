#ifndef PROJEKAT_MEMORYALLOCATOR_HPP
#define PROJEKAT_MEMORYALLOCATOR_HPP

#include "../lib/hw.h"

class MemoryAllocator {

private:
    MemoryAllocator() {};


    struct BlockHeader {
        size_t size;
    };

public:

    struct FreeSegment {
        size_t size;
        FreeSegment* next;
    };

    static void* malloc(size_t size);

    static int free(void* addr);


private:

    static FreeSegment* freeMemHead;


};



#endif //PROJEKAT_MEMORYALLOCATOR_HPP
