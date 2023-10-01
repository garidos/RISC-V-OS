#include "../h/MemoryAllocator.hpp"

MemoryAllocator::FreeSegment* MemoryAllocator::freeMemHead = nullptr;

/*
 *
 *                          _________________________
 *start ->                  |size___________________| <- BlockHeader
 *                          |_______________________| <- neiskoriscena memorija
 *start + sizeof(MBS) ->    |                       | <- pocetak alociranog dijela
 *                          |_______________________|
 *                          |                       |
 *                          |_______________________|
 *                          |                       |
 *                          |_______________________|
 *                          |                       |
 *                          |_______________________|
 *                          |                       |
 *                          |_______________________|
 *                          |                       |
 *                          |_______________________|
 *
 */

void* MemoryAllocator::malloc(size_t size) {

    if ( freeMemHead == nullptr) {
        //poravnjava se na MEM_BLOCK_SIZE
        freeMemHead = (FreeSegment*)(((uint64)HEAP_START_ADDR + MEM_BLOCK_SIZE - 1) & ~(MEM_BLOCK_SIZE - 1) );
        freeMemHead->size = (char*)HEAP_END_ADDR - (char*)freeMemHead;
        freeMemHead->next = (FreeSegment*)HEAP_END_ADDR;

    }

    if (freeMemHead == HEAP_END_ADDR) return nullptr; //puna memorija

    size_t sizeInBlocks = (size + MEM_BLOCK_SIZE - 1) / MEM_BLOCK_SIZE;
    sizeInBlocks+= 1; // dodatni blok za header ( znam da ce biti samo jedan jer je velicina headera jednaka minimalnoj velicini bloka )
    size_t realSize = sizeInBlocks * MEM_BLOCK_SIZE;

    //if ( realSize % MEM_BLOCK_SIZE ) realSize = (realSize + MEM_BLOCK_SIZE - 1) & ~( MEM_BLOCK_SIZE - 1 ); // zaokruzivanje na MEM_BLOCK_SIZE

    FreeSegment* temp = freeMemHead, *prev = nullptr;

    while( temp != HEAP_END_ADDR) {
        if ( realSize <= temp->size) break;
        prev = temp;
        temp = temp->next;
    }

    if ( temp == HEAP_END_ADDR ) return nullptr; // ne postoji dovoljno velik slobodan blok

    //s obzirom da je velicina slobodnog segmenta svakako manja od MEM_BLOCK_SIZE, to ni ne treba da provjeravam
    //ovde provjeravam da li je manje od 2 * MEM_BLOCK_SIZE, jer nema poente da ostane samo jedan blok, jer se u jedan mora staviti heder pri alokaciji, tako da je minimum velicina slobodnog segmenta dva bloka
    if ( temp->size - realSize < 2 * MEM_BLOCK_SIZE) {
        if (prev) prev->next = temp->next;
        else freeMemHead = temp->next;
        //u size ostaje postojeca velicina (samo smanjena za jedan blok u kom se nalazi heder ) jer sam uzeo citav slobodan blok
        temp->size = temp->size - MEM_BLOCK_SIZE;
        return (char*)temp + MEM_BLOCK_SIZE;    //vracam adresu pocetka alociranog dijela
    } else {
        FreeSegment* newSegment = (FreeSegment*)((char*)temp + realSize);
        newSegment->next = temp->next;
        if (prev) prev->next = newSegment;
        else freeMemHead = newSegment;
        newSegment->size = temp->size - realSize;
        //u size se stavlja prava velicina ( smanjena za jedan blok )
        temp->size = realSize - MEM_BLOCK_SIZE;
        return (char*)temp + MEM_BLOCK_SIZE;    //vracam adresu pocetka alociranog dijela
    }


}

int MemoryAllocator::free(void *addr) {

    //ako je manje od donje (poravnate) adreses, ili vece od gornje adres - greska
    if ( (char*)addr < (char*)(((uint64)HEAP_START_ADDR + MEM_BLOCK_SIZE - 1) & ~(MEM_BLOCK_SIZE - 1)) || (char*)addr > (char*)HEAP_END_ADDR ) return -1;

    //adresa koja pokazuje na citav segment ( alocirani dio + jedan blok za heder )
    char* realAdr = (char*)addr - MEM_BLOCK_SIZE;

    FreeSegment *temp = freeMemHead, *prev = nullptr;

    while( temp != HEAP_END_ADDR) {
        if ( realAdr < (char*)temp ) break;
        if ( realAdr < (char*)temp + temp->size) return -2;  //pokusava se oslobadjanje vec slobodne adrese
        prev = temp;
        temp = temp->next;
    }

    FreeSegment* curr = nullptr;

    //pokusava se spajanje sa slobodnim blokom koji se nalazi ispred
    if ( prev != nullptr && (char*)prev + prev->size == realAdr) {
        prev->size += ((BlockHeader*)realAdr)->size + MEM_BLOCK_SIZE;
        curr = prev;

    } else {
        curr = (FreeSegment*)realAdr;
        //ovde ne treba kast jer obe ove strukture imaju velicinu u prvom polju
        curr->size += MEM_BLOCK_SIZE;
        if (prev) prev->next = curr;
        else freeMemHead = curr;
        curr->next = temp;
    }

    //pokusava se spajanje sa slobodnim blokom koji se nalazi iza
    if ( (char*)curr + curr->size == (char*)temp && temp != HEAP_END_ADDR) {
        curr->size += temp->size;
        curr->next = temp->next;
    }


    return 0;
}