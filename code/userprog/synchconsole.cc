#include "copyright.h"
#include "synchconsole.h"
Semaphore* SynchConsole::readAvail;
Semaphore* SynchConsole::writeDone;

SynchConsole::SynchConsole(char *name){
	readAvail = new Semaphore("read avail", 0);
	writeDone = new Semaphore("write done", 0);
    consoleLock = new Lock("consoleLock");
    console = new Console(NULL, NULL, ReadAvail, WriteDone, 0);
}
SynchConsole::~SynchConsole() {
	delete console;
	delete consoleLock;
	delete readAvail;
	delete writeDone;
}
void 
SynchConsole::Read(int phyAddr) {
	DEBUG('a', "Trying to acquire the lock.\n");
	consoleLock->Acquire();
	DEBUG('a', "Lock has been acquired.\n");
	readAvail->P();
	DEBUG('a', "Reading from console.\n");
    machine->mainMemory[phyAddr] = console->GetChar();
    DEBUG('a', "Release the lock.\n");
	consoleLock->Release();
}

void 
SynchConsole::Write(int phyAddr){
	DEBUG('a', "Trying to acquire the lock.\n");
	consoleLock->Acquire();
	DEBUG('a', "Lock has been acquired.\n");
	DEBUG('a', "Writting to console.\n");
	console->PutChar((char) machine->mainMemory[phyAddr]);
    writeDone->P();
    DEBUG('a', "Release the lock.\n");
	consoleLock->Release();
}