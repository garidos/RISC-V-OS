#ifndef PROJEKAT_MEMORYALLOCATOR_HPP
#define PROJEKAT_MEMORYALLOCATOR_HPP

#include "../lib/hw.h"

class MemoryAllocator {

private:
    MemoryAllocator() {};

    //heder alociranog segmenta koji sadrzi njegovu velicinu ( bez hedera )
    struct BlockHeader {
        size_t size;
    };

public:

    //heder slobodnog segmenta koji sadrzi velicinu citavog segmenta i pokazivac na naredni u nizu
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
