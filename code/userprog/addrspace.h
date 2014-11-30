// addrspace.h
//	Data structures to keep track of executing user programs
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "machine.h"

#define UserStackSize		1024 	// increase this as necessary!

class AddrSpace {
public:
    AddrSpace();	// Create an address space,
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
    // before jumping to user code

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch
    void Initialize(OpenFile *executable, int numOfExtraPages);   // initializing it with the program
                                             // stored in the file "executable"  
    int TransPhyAddr(unsigned int virtAddr); //translate virtual address to physical address
    unsigned int TransPhyOffset(unsigned int virtAddr); //get physical offset
    unsigned int TransPhyNumpage(unsigned int virtAddr);//get physical number of page 
    bool success; //check if it successfully allocate pages to the thread
    unsigned int getArgStart();
private:
    TranslationEntry *pageTable;	// Assume linear page table translation
    // for now!
    unsigned int numPages;		// Number of pages in the virtual
    unsigned int argStart;               // If there are arguments, this is the start address of arguments
};

#endif // ADDRSPACE_H
