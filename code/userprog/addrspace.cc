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


//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    DEBUG('a',"Deleteing address space\n");
    unsigned int i = 0;
    for(i = 0; i < numPages; ++i) {
        TheMemoryManager->FreePage(pageTable[i].physicalPage);
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

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
           + UserStackSize; // we need to increase the size
    // to leave room for the stack
    ASSERT(numOfExtraPages >= 0);
    numPages = divRoundUp(size, PageSize)+numOfExtraPages;
    size = numPages * PageSize;


    ASSERT(numPages <= NumPhysPages);       // check we're not trying
    // to run anything too big --
    // at least until we have
    // virtual memory

    //ASSERT(numPages <= (TheMemoryManager->NumFreePage()));
    if(numPages > (TheMemoryManager->NumFreePage())) {
        DEBUG('a', "No enough pages.\n");
        success = false;
        return;
    }
    else {
        success = true;
    }

    DEBUG('a', "Initializing address space, num pages %d, size %d\n",
          numPages, size);
// first, set up the translation
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
        pageTable[i].virtualPage = i;   // for now, virtual page # = phys page #
        pageTable[i].physicalPage = TheMemoryManager->AllocPage();
        pageTable[i].valid = TRUE;
        pageTable[i].use = FALSE;
        pageTable[i].dirty = FALSE;
        pageTable[i].readOnly = FALSE;  // if the code segment was entirely on
        // a separate page, we could set its
        // pages to be read-only
    }
    if(numOfExtraPages>0){
        argStart = (numPages - numOfExtraPages) * PageSize;
    }else{
        argStart = 0;
    }

// zero out the entire address space page by page, to zero the unitialized data segment
// and the stack segment
    
    for(i=0; i < numPages; i++)
    {
        int physPage = pageTable[i].physicalPage;
        bzero(&machine->mainMemory[physPage*PageSize], PageSize);
    }

// then, copy in the code and data segments into memory.

   
    int currentCodeSize = noffH.code.size;
    int currentDataSize = noffH.initData.size;
    int currentCodePosition = noffH.code.inFileAddr;
    int currentDataPosition = noffH.initData.inFileAddr;
    int currentCodeVirtualAddr=noffH.code.virtualAddr;
    int currentDataVirtualAddr=noffH.initData.virtualAddr;
    DEBUG('a', "code VA starts: 0x%.2x, size is: %d\n",currentCodeVirtualAddr, currentCodeSize); 
    DEBUG('a', "data VA starts: 0x%.2x, size is: %d\n",currentDataVirtualAddr, currentDataSize);    

    if(noffH.code.size > 0)
    {

        if(TransPhyOffset(currentCodeVirtualAddr)!=0&&
           (currentCodeSize+TransPhyOffset(currentCodeVirtualAddr))<PageSize)
        {
            executable->ReadAt(&(machine->mainMemory[TransPhyAddr(currentCodeVirtualAddr)]),
                               currentCodeSize, currentCodePosition);
            currentCodeSize=0;
        } 

        if(TransPhyOffset(currentCodeVirtualAddr)!=0&&
           (currentCodeSize+TransPhyOffset(currentCodeVirtualAddr))>=PageSize)
        {
            executable->ReadAt(&(machine->mainMemory[TransPhyAddr(currentCodeVirtualAddr)]),
                               (PageSize-TransPhyOffset(currentCodeVirtualAddr)), currentCodePosition); 
            currentCodeSize-=(PageSize-TransPhyOffset(currentCodeVirtualAddr));
            currentCodePosition+=(PageSize-TransPhyOffset(currentCodeVirtualAddr));
            currentCodeVirtualAddr+=(PageSize-TransPhyOffset(currentCodeVirtualAddr));
        }

        while (currentCodeSize >0) 
        {
            if(currentCodeSize < PageSize)
            {

                executable->ReadAt(&(machine->mainMemory[TransPhyAddr(currentCodeVirtualAddr)]),
                                   currentCodeSize, currentCodePosition); 
                currentCodeSize-=PageSize;
            }
            else
            {
                executable->ReadAt(&(machine->mainMemory[TransPhyAddr(currentCodeVirtualAddr)]),
                                    PageSize, currentCodePosition);
        
                currentCodeSize-=PageSize;
                currentCodePosition+=PageSize;
                currentCodeVirtualAddr+=PageSize;
            }
        }       

    }

    if(noffH.initData.size > 0)
    {
  
        if(TransPhyOffset(currentDataVirtualAddr)!=0&&currentDataSize<PageSize)
        {

            executable->ReadAt(&(machine->mainMemory[TransPhyAddr(currentDataVirtualAddr)]),
                                currentDataSize, currentDataPosition);
            currentDataSize=0;
        } 

        if(TransPhyOffset(currentDataVirtualAddr)!=0&&currentDataSize>=PageSize)
        {

            executable->ReadAt(&(machine->mainMemory[TransPhyAddr(currentDataVirtualAddr)]),
                               (PageSize-TransPhyOffset(currentDataVirtualAddr)), currentDataPosition); 
            currentDataSize-=(PageSize-TransPhyOffset(currentDataVirtualAddr));
            currentDataPosition+=(PageSize-TransPhyOffset(currentDataVirtualAddr));
            currentDataVirtualAddr+=(PageSize-TransPhyOffset(currentDataVirtualAddr));
        
        }

        while (currentDataSize >0) 
        {

            if(currentDataSize < PageSize)
            {

                executable->ReadAt(&(machine->mainMemory[TransPhyAddr(currentDataVirtualAddr)]),
                                   currentDataSize, currentDataPosition); 
                currentDataSize-=PageSize;
            }
            else
            {

                executable->ReadAt(&(machine->mainMemory[TransPhyAddr(currentDataVirtualAddr)]),
                                   PageSize, currentDataPosition);
        
                currentDataSize-=PageSize;
                currentDataPosition+=PageSize;
                currentDataVirtualAddr+=PageSize;
            }

        }    

    }

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
//void AddrSpace::ReleasePages()
//{
//    for(int i = 0; i < numPages; i++)
//    {
//
//    }

int AddrSpace::TransPhyAddr(unsigned int virtAddr)
{
    unsigned int vpn, offset, physAddr;
    TranslationEntry *entry;
    unsigned int pageFrame;

    vpn = (unsigned) virtAddr / PageSize;
    offset = (unsigned) virtAddr % PageSize;
    if(vpn >= numPages) {
        //error
        DEBUG('a', "illegal virtual address: out of bound.\n");
        return -1;
    }
    entry = &pageTable[vpn];
    pageFrame = entry->physicalPage;
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