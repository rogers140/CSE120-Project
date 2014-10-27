// badtest.cc 
//	An example program in Nachos demonstrating a race condition.

#include "copyright.h"
#include "system.h"
#include "synch.h"

// testnum is set in main.cc
int testnum = 1;

int sharedValue;

//----------------------------------------------------------------------
// BadSimpleThread
// 	Each thread is supposed to increment sharedValue 5 times.
//	With two threads, the final value of sharedValue should be 10,
//      but it's not...
//----------------------------------------------------------------------

void
BadSimpleThread(int which)
{
    int num;
    int tmp;

    for (num = 0; num < 5; num++) {
      // No Acquire!

      tmp = sharedValue;
      currentThread->Yield();
      tmp = tmp + 1;
      currentThread->Yield();
      sharedValue = tmp;
      printf("Thread %d, sharedValue %d\n", which, sharedValue);

      // No Release!
      currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    int i;

    DEBUG('t', "Entering BadSimpleTest");

    sharedValue = 0;

    for (i = 0; i < 2; i++) {
      Thread *t = new Thread("forked thread");
      t->Fork(BadSimpleThread, i);
    }
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    default:
	printf("No test specified.\n");
	break;
    }
}

