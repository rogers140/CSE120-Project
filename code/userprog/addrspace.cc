// addrspace.cc
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "memorymanager.h"
#include "addrspace.h"
#include "noff.h"
#ifdef HOST_SPARC
#include <strings.h>
#endif
extern MemoryManager *TheMemoryManager;
//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------


static void
SwapHeader (NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace()
{
}

AddrSpace::~AddrSpace()
{
    DEBUG('a',"Deleteing address space\n");
    unsigned int i = 0;
    for(i = 0; i < numPages; ++i) {
        if(pageTable[i].physicalPage == -1) {
            continue;
        }
        else {
            TheMemoryManager->FreePage(pageTable[i].physicalPage);
        }   
    }
    delete [] pageTable;
}

void
AddrSpace::Initialize(OpenFile *executable, int numOfExtraPages)
{
    NoffHeader noffH;
    unsigned int i, size;
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
            (WordToHost(noffH.noffMagic) == NOFFMAGIC))
        SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);
    CodeStart = noffH.code.virtualAddr;
    DataStart = noffH.initData.virtualAddr;
    CodeEnd = noffH.code.virtualAddr + noffH.code.size;
    DataEnd = noffH.initData.virtualAddr + noffH.initData.size;   
    FileCodeStart = noffH.code.inFileAddr;
    FileDataStart = noffH.initData.inFileAddr;
    execFile = executable; //save executable file
    // how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
           + UserStackSize; // we need to increase the size
    // to leave room for the stack
    if(numOfExtraPages < 0) {
        success = false;
        return;
    }
    numPages = divRoundUp(size, PageSize)+numOfExtraPages;
    //DEBUG('c',"Number of extra page: %d\n", numOfExtraPages);
    size = numPages * PageSize;

    DEBUG('a', "Initializing address space, num pages %d, size %d\n",
          numPages, size);
// first, set up the translation
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
        pageTable[i].virtualPage = i;  
        pageTable[i].physicalPage = -1; //not initialized
        pageTable[i].valid = FALSE;
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;  // if the code segment was entirely on
    }
    if(numOfExtraPages>0){
        argStart = (numPages - numOfExtraPages) * PageSize;
    }else{
        argStart = 0;
    }
    success = true;

}

TranslationEntry *
AddrSpace::getPageTable() {
    return pageTable;
}
void
AddrSpace::LoadFromExec(int virtualPageNum) {//initialize the page
    //pageTable[virtualPageNum].physicalPage = phyPageNum; //may fail to throw eror!
    int currentVAddr = virtualPageNum * PageSize;
    //DEBUG('c', "Paging in page %d\n", virtualPageNum);
    for(int i = currentVAddr; i < currentVAddr+PageSize; ++i){
        if(i >= CodeStart && i < CodeEnd){
            //DEBUG('c', "Paging in code from %d to %d.\n", i - CodeStart + FileCodeStart, TransPhyAddr(i));
            execFile->ReadAt(&(machine->mainMemory[TransPhyAddr(i)]),
                                   1, i - CodeStart + FileCodeStart);

        }
        else if(i >= DataStart && i < DataEnd){
            //DEBUG('c', "Paging in data.\n");
            execFile->ReadAt(&(machine->mainMemory[TransPhyAddr(i)]),
                                   1, i - DataStart + FileDataStart);
        }
        else{
            //DEBUG('c', "Paging in stack.\n");
            bzero(&machine->mainMemory[TransPhyAddr(i)], 1);
        }
    }
    //pageTable[virtualPageNum].valid = true;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
        machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we don't
    // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{

}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

int AddrSpace::TransPhyAddr(unsigned int virtAddr)
{
    unsigned int vpn, offset, physAddr;
    TranslationEntry *entry;
    int pageFrame;

    vpn = (unsigned) virtAddr / PageSize;
    offset = (unsigned) virtAddr % PageSize;
    if(vpn >= numPages) {
        //error
        DEBUG('a', "illegal virtual address: out of bound.\n");
        return -1;
    }
    entry = &pageTable[vpn];
    pageFrame = entry->physicalPage;
    if(pageFrame == -1) {
        return -1;
    }
    physAddr = pageFrame * PageSize + offset;
    return physAddr;
}

unsigned int AddrSpace::TransPhyOffset(unsigned int virtAddr)
{
    unsigned int offset;
    offset = (unsigned) virtAddr % PageSize;
    return offset;
}

unsigned int AddrSpace::TransPhyNumpage(unsigned int virtAddr) 
{
    unsigned int vpn;
    TranslationEntry *entry;
    unsigned int pageFrame;

    vpn = (unsigned) virtAddr / PageSize;
    entry = &pageTable[vpn];
    pageFrame = entry->physicalPage;
    return pageFrame;
}

unsigned int AddrSpace::getArgStart(){
    return argStart;
}
unsigned int AddrSpace::getNumOfPages() {
    return numPages;
}