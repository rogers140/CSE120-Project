#include "copyright.h"

#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "synch.h"
#include "console.h"
#include "system.h"
class SynchConsole{
public:
	SynchConsole(char *name);
	~SynchConsole();
	//void ReadAvail(int arg);
	//void WriteDone(int arg);
	void Read(int phyAddr);
	void Write(int phyAddr);
	static Semaphore *readAvail;//= new Semaphore("read avail", 0);
	static Semaphore *writeDone;// = new Semaphore("write done", 0); 
private:
	Console *console;
	Lock *consoleLock;
	static void ReadAvail(int arg) {
		DEBUG('c', "Read available.\n");
		readAvail->V();
	}
	static void WriteDone(int arg) {
		DEBUG('c', "Write done.\n");
		writeDone->V();
	}
};
#endif