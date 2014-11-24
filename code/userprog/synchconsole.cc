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
	consoleLock->Acquire();
	readAvail->P();
    machine->mainMemory[phyAddr] = console->GetChar();
	consoleLock->Release();
}

void 
SynchConsole::Write(int phyAddr){
	consoleLock->Acquire();
	console->PutChar((char) machine->mainMemory[phyAddr]);
    writeDone->P();
	consoleLock->Release();
}