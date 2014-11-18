#ifndef MEMORYMANAGER_H
#define MEMORYMANAGER_H

#include "copyright.h"
#include "bitmap.h"
#include "synch.h"
class MemoryManager{
public:

/* Create a manager to track the allocation of numPages of physical memory.  
   You will create one by calling the constructor with NumPhysPages as
   the parameter.  All physical pages start as free, unallocated pages. */
    MemoryManager(unsigned int numPages);
    ~MemoryManager();

/* Allocate a free page, returning its physical page number or -1
   if there are no free pages available. */
    int AllocPage();

/* Free the physical page and make it available for future allocation. */
    void FreePage(int physPageNum);

/* True if the physical page is allocated, false otherwise. */
    bool PageIsAllocated(int physPageNum);

/* Return the number of free pages. */
    unsigned int NumFreePage();

private:
	BitMap* bitMap;
  Lock* safeLock;
};

#endif // MEMORYMANAGER_H
