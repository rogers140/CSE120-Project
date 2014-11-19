// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"
#include "memorymanager.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------
void ProcessStart(char *filename);
void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    

    if ((which == SyscallException) && (type == SC_Halt)) {
        DEBUG('a', "Shutdown, initiated by user program.\n");
        interrupt->Halt();
    }else if ((which == SyscallException) && (type == SC_Exit)) {
    	int arg1 = machine->ReadRegister(4); //read the arg of exit
        Exit(arg1);
    }
    else if ((which == SyscallException) && (type == SC_Exec)) {
        int arg1 = machine->ReadRegister(4); //read the arg of exit
        //int arg2 = machine->ReadRegister(5); //read the arg of exit
        //int arg3 = machine->ReadRegister(6); //read the arg of exit
        //int arg4 = machine->ReadRegister(7); //read the arg of exit
        Thread* t = new Thread("new");
        machine->WriteRegister(8,1);
        t ->Fork((VoidFunctionPtr)ProcessStart,arg1);
        machine->WriteRegister(8,2);
        currentThread->Yield();
    }
     else {
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
    
}


void
ProcessStart(char *filename)
{
    machine->WriteRegister(8,1);
    OpenFile *executable = fileSystem->Open("../test/exittest");
    machine->WriteRegister(8,2);
    AddrSpace *space;
    machine->WriteRegister(8,3);
    MemoryManager *TheMemoryManager;
    machine->WriteRegister(8,4);
    ASSERT(executable != NULL);
    if (executable == NULL) {
        printf("Unable to open file %s\n", filename);
        return;
    }
    TheMemoryManager = new MemoryManager(NumPhysPages);
    space = new AddrSpace(TheMemoryManager);
    space->Initialize(executable);
    currentThread->space = space;

    delete executable;          // close file

    //space->InitRegisters();     // set the initial register values
    //space->RestoreState();      // load page table register

    machine->WriteRegister(2,(int)space);

    int pc; 
    machine->WriteRegister(8,255);
    pc=machine->ReadRegister(PCReg); 
    machine->WriteRegister(8,254);
    machine->WriteRegister(PrevPCReg,pc); 
    machine->WriteRegister(8,253);
    pc=machine->ReadRegister(NextPCReg); 
    machine->WriteRegister(8,252);
    machine->WriteRegister(PCReg,pc); 
    machine->WriteRegister(8,251);
    pc += 4; 
    machine->WriteRegister(8,250);
    machine->WriteRegister(NextPCReg,pc); 
    machine->Run();         // jump to the user progam

    //ASSERT(FALSE);          // machine->Run never returns;
    // the address space exits
    // by doing the syscall "exit"
}
