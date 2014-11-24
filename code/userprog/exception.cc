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

// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "syscall.h"
#include "table.h"
#include "synchconsole.h"

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
extern Table *processTable;
//extern SynchConsole *synchConsole;
void exit(int exitCode);
void ProcessStart(char *filename);

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
    

    if ((which == SyscallException) && (type == SC_Halt)) {
        DEBUG('a', "Shutdown, initiated by user program.\n");
        interrupt->Halt();
    }else if ((which == SyscallException) && (type == SC_Exit)) {
    	int arg1 = machine->ReadRegister(4);                           //read the arg of exit
        exit(arg1);
    }
    else if ((which == SyscallException) && (type == SC_Exec)) {
        DEBUG('a', "Enter Exec call.\n");
        int arg1 = machine->ReadRegister(4);                           //read the arg of Exec
        // int arg2 = machine->ReadRegister(5);
        // int arg3 = machine->ReadRegister(6);
        // int arg4 = machine->ReadRegister(7);
        //reading filename
        const int maxLength = 100;
        char *filename = new char[maxLength];                          //no longer than 100 letters
        int index = 0;

        int phyAddr = 0;
        char c = '\0';
        
        while(1) {
            phyAddr = (currentThread->space)->TransPhyAddr(arg1);
            if(phyAddr == -1) {
                //error
                DEBUG('a', "File name reaches illegal virtual address.\n");
                delete [] filename;
                return;
            }
            c = (char) machine->mainMemory[phyAddr];
            if(index == maxLength - 1) {// exceed the max length
                if(c != '\0') {
                    //error
                    DEBUG('a', "Too long file name.\n");
                    delete [] filename;
                    return;
                }
                else {
                    filename[index] = '\0';
                    break;
                }
            }
            else if(c == '\0') {
                filename[index] = '\0';
                DEBUG('a', "string end.\n");
                break;
            }
            else {
                filename[index] = c;
                arg1 = arg1 + 1;
                index += 1;
            }
            
        }
        //DEBUG('a', "File name is: %s\n", filename);
        Thread* t = new Thread("Exec");
        int spaceID = processTable->Alloc((void *) t);
        if(spaceID == -1) {                                                 // no enough slot in process table
            //error
            DEBUG('a', "No enough process table slot.\n");
            delete t;
            delete [] filename;
            return;
        }
        else {
            t->setSpaceID(spaceID);
        }
        OpenFile *executable = fileSystem->Open(filename);
        if(executable == NULL) {
            //error
            DEBUG('a', "Executable file is NUll.\n");
            delete t;
            delete [] filename;
            return;
        }

        t->space = new AddrSpace();
        t->space->Initialize(executable);
        delete executable;                                                  // close file
        machine->WriteRegister(PCReg, machine->ReadRegister(PCReg) + 4);    //increment PC and NextPC
        machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
        t->Fork((VoidFunctionPtr)ProcessStart,arg1);
        machine->WriteRegister(2, (int)(spaceID));
    }
    else if((which == SyscallException) && (type == SC_Read)) {
        DEBUG('a', "Enter Read.\n");
        int buffer = machine->ReadRegister(4);                           //virtual address of buffer
        int size = machine->ReadRegister(5);                           //size to read
        //int id = machine->ReadRegister(6);                           // id
        int i = 0;
        SynchConsole *synchConsole = new SynchConsole("read console");
        for(i = 0; i < size; ++i) {
            int phyAddr = currentThread->space->TransPhyAddr(buffer);
            if(phyAddr == -1) {
                //error
                DEBUG('a', " Read illegal virtual address.\n");
                delete synchConsole;
                return;
            }
            synchConsole->Read(phyAddr);
            buffer += 1;
        }
        machine->WriteRegister(PCReg, machine->ReadRegister(PCReg) + 4);    //increment PC and NextPC
        machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);

    }
    else if((which == SyscallException) && (type == SC_Write)) {
        DEBUG('a', "Enter Write.\n");
        int buffer = machine->ReadRegister(4);                           //virtual address of buffer
        int size = machine->ReadRegister(5);                           //size to read
        //int id = machine->ReadRegister(6);                           // id
        int i = 0;
        SynchConsole *synchConsole = new SynchConsole("read console");
        for(i = 0; i < size; ++i) {
            int phyAddr = currentThread->space->TransPhyAddr(buffer);
            if(phyAddr == -1) {
                //error
                DEBUG('a', " Write illegal virtual address.\n");
                delete synchConsole;
                return;
            }
            synchConsole->Write(phyAddr);
            buffer += 1;
        }
        machine->WriteRegister(PCReg, machine->ReadRegister(PCReg) + 4);    //increment PC and NextPC
        machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg) + 4);
    }
    else {
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
    
}
void exit(int exitCode) {
    printf("I am going to exit %d\n",exitCode);
    Thread *t = currentThread;
    delete t->space;                        // release the address space of current thread
    processTable->Release(t->getSpaceID()); // release process table slot before leaving
    t->Finish();
}

void
ProcessStart(char *filename)
{
    DEBUG('a', "Enter child process\n");
    currentThread->space->InitRegisters();     // set the initial register values
    currentThread->space->RestoreState();      // load page table register
    machine->Run();                            // jump to the user progam
}
