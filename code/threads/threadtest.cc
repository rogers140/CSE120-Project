// threadtest.cc 
//	Test cases for the threads assignment.

#include "copyright.h"
#include "system.h"
#include "synch.h"

// testnum is set in main.cc
int testnum = 1;


//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
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
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

//-------------------------------------------------------------------
// variables for tests
//-------------------------------------------------------------------

// used in lock tests
Lock *locktest1 = NULL;
Lock *locktest2 = NULL;
Lock *locktest3 = NULL;
Lock *locktest4 = NULL;
Lock *locktest5 = NULL;

// used in CV tests
Lock *cvLock1 =NULL;
Condition *cv1 = NULL;
int sharedState = 0;


// used in mailbox test
Mailbox *mb = NULL;

//  yieldTimes is used in join tests
int yieldTimes = 3;

// used in priority tests
Lock *priLock = NULL;
Semaphore *priSemaphore = NULL;
Condition *priCV= NULL;
Lock *priCVLock= NULL;

//used in whale tests
Whale *whale = NULL;

//used for extra credit
Lock *extraLock = NULL;



//----------------------------------------------------------------------
// LockTest1 normal lock situation
//----------------------------------------------------------------------

void
LockThread1(int param)
{
    printf("L1:0 Thread 1 is created\n");
    locktest1->Acquire();
    printf("L1:1 Thread 1 has acquired the lock\n");
    currentThread->Yield();
    printf("L1:2 Thread 1 yield\n");
    locktest1->Release();
    printf("L1:3 Thread 1 has released the lock\n");
}

void
LockThread2(int param)
{
    printf("L2:0 Thread 2 is created\n");
    locktest1->Acquire();
    printf("L2:1 Thread 2 has acquired the lock\n");
    currentThread->Yield();
    printf("L2:2 Thread 2 yield\n");
    locktest1->Release();
    printf("L2:3 Thread 2 has released the lock\n");
}

void
LockTest1()
{
    DEBUG('t', "Entering LockTest1");

    locktest1 = new Lock("LockTest1");

    // fork two threads
    Thread *t = new Thread("one");
    t->Fork(LockThread1, 0);
    t = new Thread("two");
    t->Fork(LockThread2, 0);
}


//----------------------------------------------------------------------
// LockTest3 acquire the same lock twice
//----------------------------------------------------------------------
void
LockThread3(int param)
{
    printf("L3:0 \n");
    // acquire is called twice
    locktest3->Acquire();
    locktest3->Acquire();
}


void
LockTest3()
{
    DEBUG('t', "Acquire  Lock Twice");

    locktest3 = new Lock("LockTest3");
    //fork one new thread
    Thread *t = new Thread("one");
    t->Fork(LockThread3, 0);


}

//----------------------------------------------------------------------
// LockTest4 release the lock that is not held
//----------------------------------------------------------------------
void
LockThread4_1(int param)
{
    printf("L4:0 Thread 1 is created\n");
    locktest4->Acquire();
    printf("L4:1 Thread 1 has acquired the lock\n");
    currentThread->Yield();
}

void
LockThread4_2(int param)
{
    printf("L4:0 Thread 2 is created and tried to release the lock held by thread 1\n");
    locktest4->Release();
}

void
LockTest4()
{
    DEBUG('t', "Release Lock that is not held");

    locktest4 = new Lock("LockTest4");
    
    Thread *t = new Thread("one");
    // fork two threads
    t->Fork(LockThread4_1, 0);
    t = new Thread("two");
    t->Fork(LockThread4_2, 0);


}

//----------------------------------------------------------------------
// LockTest5 delete a lock that is not held
//----------------------------------------------------------------------
void
LockThread5(int param)
{
    printf("L5:0 Thread 1 is created\n");
    // acquire the lock
    locktest5->Acquire();
}

void
LockTest5()
{
    DEBUG('t', "Delete a lock that is held");

    locktest5 = new Lock("LockTest5");
    // fork one thread
    Thread *t = new Thread("one");
    t->Fork(LockThread5, 0);
    currentThread->Yield();
    // printf("%s",locktest5->holder); // for debug
    //delete the lock that is currently held
    delete locktest5;

}








//----------------------------------------------------------------------
// CvTest0 Condition variable not holding the lock
//----------------------------------------------------------------------
void
cvThread0(int param){
    printf("C1:0 thread 1 is created\n");
    // cvLock1->Acquire();
    printf("C1:1 thread 1 did not get the lock\n");
    while(!sharedState){
        printf("C1:2 thread 1 tried to wait on cv\n");
        cv1->Wait(cvLock1);
        printf("C1:3 thread 1 waited on cv sucessfully\n");
    }
    ASSERT(sharedState);
    printf("C1:4 thread 1 waked up\n");
    // cvLock1->Release();

}

void
CvTest0(){
    DEBUG('t', "Condition variable signal");

    cvLock1 = new Lock("cvLock0");
    cv1 = new Condition("cv0");
    //fork one thread
    Thread *t = new Thread("zero");
    t->Fork(cvThread0, 0);

}

//----------------------------------------------------------------------
// CvTest1 Condition variable signal
//----------------------------------------------------------------------
void
cvThread1(int param){
    printf("C1:0 thread 1 is created\n");
    cvLock1->Acquire();
    printf("C1:1 thread 1 has acquired the lock\n");
    while(!sharedState){
        printf("C1:2 thread 1 waiting on cv\n");
        cv1->Wait(cvLock1);
        printf("C1:3 thread 1 wake up\n");
    }
    ASSERT(sharedState);
    printf("C1:4 thread 1 finish\n");
    cvLock1->Release();

}

void
cvThread2(int param){
    printf("C2:0 thread 2 is created\n");
    cvLock1->Acquire();
    printf("C2:1 thread 2 has acquired the lock\n");
    sharedState=1;
    printf("C2:2 thread 2 change sharedState\n");
    cv1->Signal(cvLock1);
    printf("C2:3 thread 2 signal cv\n");
    printf("C2:4 thread 2 finish\n");
    cvLock1->Release();
}

void
CvTest1(){
    DEBUG('t', "Condition variable signal");

    cvLock1 = new Lock("cvLock1");
    cv1 = new Condition("cv1");
    // fork 2 threads
    Thread *t = new Thread("one");
    t->Fork(cvThread1, 0);
    t = new Thread("two");
    t->Fork(cvThread2, 0);

}

//----------------------------------------------------------------------
// CvTest2 Condition variable boradcast
//----------------------------------------------------------------------
void
cvThread3(int param){
    printf("C3:0 thread 3 is created\n");
    cvLock1->Acquire();
    printf("C3:1 thread 3 acquired the lock\n");
    while(!sharedState){
        printf("C3:2 thread 3 is waiting on cv\n");
        cv1->Wait(cvLock1);
        printf("C3:3 thread 3 wake up\n");
    }
    ASSERT(sharedState);
    printf("C3:4 thread 3 finish\n");
    cvLock1->Release();
}

void
cvThread4(int param){
    printf("C4:0 thread 4 is created\n");
    cvLock1->Acquire();
    printf("C4:1 thread 4 acquired the lock\n");
    while(!sharedState){
        printf("C4:2 thread 4 waiting on cv\n");
        cv1->Wait(cvLock1);
        printf("C4:3 thread 4 wake up\n");
    }
    ASSERT(sharedState);
    printf("C4:4 thread 4 finish\n");
    cvLock1->Release();
}

void
cvThread5(int param){
    printf("C5:0 thread 5 is created\n");
    cvLock1->Acquire();
    printf("C5:1 thread 5 acquired the lock\n");
    sharedState=1;
    printf("C5:2 thread 5 changed sharedState\n");
    cv1->Broadcast(cvLock1);
    printf("C5:3 thread 5 broadcast\n");
    cvLock1->Release();
}

void
CvTest2(){
    DEBUG('t', "Condition variable broadcast");

    cvLock1 = new Lock("cvLock2");
    cv1 = new Condition("cv2");
    // fork 3 threads
    Thread *t = new Thread("three");
    t->Fork(cvThread3, 0);
    t = new Thread("four");
    t->Fork(cvThread4, 0);
    t = new Thread("five");
    t->Fork(cvThread5, 0);

}

//----------------------------------------------------------------------
// CvTest3 Signal/broadcasting but no body is waiting
//----------------------------------------------------------------------

void
cvThread6(int param){
    printf("C6:0 thread 6 is created\n");
    cvLock1->Acquire();
    printf("C6:1 thread 6 has acquired the lock\n");
    cv1->Wait(cvLock1);
    printf("C6:2 thread 6 wake up\n");
    cvLock1->Release();
    printf("C6:3 thread 6 finish\n");
}

void
cvThread7(int param){
    printf("C7:0 thread7 is created\n");
    cvLock1->Acquire();
    printf("C7:1 thread7 has acquired the lock\n");
    cv1->Wait(cvLock1);
    printf("C7:2 thread7 wake up\n");
    cvLock1->Release();
    printf("C7:3 thread7 finish\n");
    
}

void
CvTest3(){
    DEBUG('t', "Condition variable broadcast");

    cvLock1 = new Lock("cvLock3");
    cv1 = new Condition("cv3");
    //fork four threads
    Thread *t = new Thread("two");
    t->Fork(cvThread2, 0);
    t = new Thread("five");
    t->Fork(cvThread5, 0);
    t = new Thread("six");
    t->Fork(cvThread6, 0);
    t = new Thread("seven");
    t->Fork(cvThread7, 0);

}

//----------------------------------------------------------------------
// CvTest4 Condition delete queue that is not empty
//----------------------------------------------------------------------
void
cvThread8(int param){
    printf("C8:0 thread 8 is created\n");
    cvLock1->Acquire();
    printf("C8:1 thread 8 acquired the lock\n");
    cv1->Wait(cvLock1);
    printf("C8:2 thread 8 wake up\n");
    cvLock1->Release();
    printf("C8:3 thread 8 finish\n");
}

void
cvThread9(int param){
    printf("C9:0 thread 9 is created\n");
    cvLock1->Acquire();
    printf("C9:1 thread 9 acquired the lock\n");
    cv1->Wait(cvLock1);
    printf("C9:2 thread 9 wake up\n");
    cvLock1->Release();
    printf("C9:3 thread 9 finish\n");
}

void
cvThread10(int param){
    printf("C10:0 thread 10 is created\n");
    cvLock1->Acquire();
    printf("C10:1 thread 10 delete the lock\n");
    delete cv1;
    printf("C10:2 thread 10 finish\n");
    cvLock1->Release();
}

void
CvTest4(){
    DEBUG('t', "delete cv which is waited by others");

    cvLock1 = new Lock("cvLock4");
    cv1 = new Condition("cv4");
    // fork 3 threads
    Thread *t = new Thread("eight");
    t->Fork(cvThread8, 0);
    t = new Thread("nine");
    t->Fork(cvThread9, 0);
    t = new Thread("ten");
    t->Fork(cvThread10, 0);

}


//----------------------------------------------------------------------
// MailTest for two senders and two receivers
//----------------------------------------------------------------------
void
mailThread1(int param){
    printf("C1:0 this is sender 1\n");   
    printf("The mail sender 1 send is %d\n",1);
    mb->Send(1);
}

void
mailThread2(int param){
    printf("C2:0 this is sender 2\n");
    printf("The mail sender 2 send is %d\n",2);
    mb->Send(2);
}

void
mailThread3(int param){
    printf("C3:0 this is receiver1\n");
    int tmp = 0;
    int *result1 = &tmp;
    mb->Receive(result1);
    printf("The mail receiver1 get is %d\n",*result1);
}

void
mailThread4(int param){
    printf("C4:0 this is receiver2\n");
    int tmp = 0;
    int *result2 = &tmp;
    mb->Receive(result2);
    printf("The mail receiver2 get is %d\n",*result2);
}

void
MailTest(){
    DEBUG('t', "Mailbox Text");

    mb = new Mailbox("mailbox");
    // fork 4 threads 2 as sender 2 as receiver
    Thread *t = new Thread("one");
    t->Fork(mailThread1, 0);
    t = new Thread("two");
    t->Fork(mailThread2, 0);
    t = new Thread("three");
    t->Fork(mailThread3, 0);
    t = new Thread("four");
    t->Fork(mailThread4, 0);

}
//----------------------------------------------------------------------
// MailTest for one sender and two receivers
//----------------------------------------------------------------------
void
MailTest2(){
    DEBUG('t', "Mailbox Text");

    mb = new Mailbox("mailbox");
    // fork 4 threads 2 as sender 2 as receiver
    Thread *t = new Thread("one");
    t->Fork(mailThread1, 0);
    t = new Thread("three");
    t->Fork(mailThread3, 0);
    t = new Thread("four");
    t->Fork(mailThread4, 0);

}

//----------------------------------------------------------------------
// MailTest for two senders and one receiver
//----------------------------------------------------------------------
void
MailTest3(){
    DEBUG('t', "Mailbox Text");

    mb = new Mailbox("mailbox");
    // fork 4 threads 2 as sender 2 as receiver
    Thread *t = new Thread("one");
    t->Fork(mailThread1, 0);
    t = new Thread("two");
    t->Fork(mailThread2, 0);
    t = new Thread("three");
    t->Fork(mailThread3, 0);

}


//----------------------------------------------------------------------
// Join test
//----------------------------------------------------------------------

void
Joiner(Thread *joinee)
{
  for(int i=0;i < yieldTimes;i++){
    currentThread->Yield();
  }
  printf("Waiting for the Joinee to finish executing.\n");

  currentThread->Yield();
  currentThread->Yield();

  // Note that, in this program, the "joinee" has not finished
  // when the "joiner" calls Join().  You will also need to handle
  // and test for the case when the "joinee" _has_ finished when
  // the "joiner" calls Join().

  joinee->Join();
  
  currentThread->Yield();
  currentThread->Yield();

  printf("Joinee has finished executing, we can continue.\n");

  currentThread->Yield();
  currentThread->Yield();
}
// another version of joiner function in which join is called twice
void
Joiner2(Thread *joinee)
{
  for(int i=0;i<yieldTimes;i++){
    currentThread->Yield();
  }

  printf("Waiting for the Joinee to finish executing.\n");

  currentThread->Yield();
  currentThread->Yield();

  // Note that, in this program, the "joinee" has not finished
  // when the "joiner" calls Join().  You will also need to handle
  // and test for the case when the "joinee" _has_ finished when
  // the "joiner" calls Join().

  joinee->Join();
  joinee->Join();
  
  currentThread->Yield();
  currentThread->Yield();

  printf("Joinee has finished executing, we can continue.\n");

  currentThread->Yield();
  currentThread->Yield();
}
void
Joinee()
{
  int i;
  for (i = 0; i < 5; i++) {
    printf("Smell the roses.\n");
    currentThread->Yield();
  }

  currentThread->Yield();
  printf("Done smelling the roses!\n");
  currentThread->Yield();
}
//
void
Joinee3();
void
Joiner3(Thread *joinee)
{
  for(int i=0;i < yieldTimes;i++){
    currentThread->Yield();
  }
  printf("Waiting for the Joinee to finish executing.\n");

  currentThread->Yield();
  currentThread->Yield();

  // Note that, in this program, the "joinee" has not finished
  // when the "joiner" calls Join().  You will also need to handle
  // and test for the case when the "joinee" _has_ finished when
  // the "joiner" calls Join().
  joinee->Fork((VoidFunctionPtr) Joinee3, 0);
  joinee->Join();
  
  currentThread->Yield();
  currentThread->Yield();

  printf("Joinee has finished executing, we can continue.\n");

  currentThread->Yield();
  currentThread->Yield();
}

//another version of joinee in which finish is called before join is called
void
Joinee3()
{
  int i;
  currentThread->Finish();
  for (i = 0; i < 5; i++) {
    printf("Smell the roses.\n");
    currentThread->Yield();
  }

  currentThread->Yield();
  printf("Done smelling the roses!\n");
  currentThread->Yield();
}

//----------------------------------------------------------------------
// ForkerThread1 common join test
// when yieldTimes = 5 joinee is only destroy after join() is called
//----------------------------------------------------------------------

void
ForkerThread()
{
  Thread *joiner = new Thread("joiner", 0);  // will not be joined
  Thread *joinee = new Thread("joinee", 1);  // WILL be joined

  // fork off the two threads and let them do their business
  joiner->Fork((VoidFunctionPtr) Joiner, (int) joinee);
  joinee->Fork((VoidFunctionPtr) Joinee, 0);

  // this thread is done and can go on its merry way
  printf("Forked off the joiner and joiner threads.\n");
}

//----------------------------------------------------------------------
// ForkerThread2 join itself
//----------------------------------------------------------------------
void
ForkerThread2()
{
  Thread *joiner = new Thread("joiner", 1);  // will not be joined
  //Thread *joinee = new Thread("joinee", 1);  // WILL be joined

 
  joiner->Fork((VoidFunctionPtr) Joiner, (int) joiner);     // only one thread is forked and this thread join itself
  //joinee->Fork((VoidFunctionPtr) Joinee, 0);

  // this thread is done and can go on its merry way
  printf("Forked off the joiner and joiner threads.\n");
}

//----------------------------------------------------------------------
// ForkerThread3 finish before join() called
//----------------------------------------------------------------------
void
ForkerThread3()
{
  Thread *joiner = new Thread("joiner", 0);  // will not be joined
  Thread *joinee = new Thread("joinee", 1);  // WILL be joined

  // fork off the two threads and let them do their business
  joiner->Fork((VoidFunctionPtr) Joiner3, (int) joinee);
  

  // this thread is done and can go on its merry way
  printf("Forked off the joiner and joiner threads.\n");
}

//----------------------------------------------------------------------
// ForkerThread4 join() can only be called after fork() is called
//----------------------------------------------------------------------
void
ForkerThread4()
{
  Thread *joiner = new Thread("joiner", 0);  // will not be joined
  Thread *joinee = new Thread("joinee", 1);  // WILL be joined

  // fork off the two threads and let them do their business
  joiner->Fork((VoidFunctionPtr) Joiner, (int) joinee);    // no forked has been called on joinee so it cannot join!
  // joinee->Fork((VoidFunctionPtr) Joinee, 0);



  // this thread is done and can go on its merry way
  printf("Forked off the joiner and joiner threads.\n");
}

//----------------------------------------------------------------------
// ForkerThread5 only thread created to be joined can be joined
//----------------------------------------------------------------------
void
ForkerThread5()
{
  Thread *joiner = new Thread("joiner", 0);  // will not be joined
  Thread *joinee = new Thread("joinee", 1);  // WILL be joined

  // fork off the two threads and let them do their business
  joiner->Fork((VoidFunctionPtr) Joiner, (int) joinee);
  joinee->Fork((VoidFunctionPtr) Joinee, 0);
  joiner->Join();     // joiner cannot join as it is not created to join!



  // this thread is done and can go on its merry way
  printf("Forked off the joiner and joiner threads.\n");
}

//----------------------------------------------------------------------
// ForkerThread6 join can only be called once
//----------------------------------------------------------------------
void
ForkerThread6()
{
  Thread *joiner = new Thread("joiner", 0);  // will not be joined
  Thread *joinee = new Thread("joinee", 1);  // WILL be joined

  // fork off the two threads and let them do their business
  joiner->Fork((VoidFunctionPtr) Joiner2, (int) joinee);
  joinee->Fork((VoidFunctionPtr) Joinee, 0);
  //joinee->Join();     // joinee is joined first here then second time in function Joiner which is wrong
}

//----------------------------------------------------------------------
// PriTest threads with different priority
//----------------------------------------------------------------------
// this is thread one and its priority is 10.
void
priThread1(int param){

    printf("C1:0: low priority thread coming and then yield\n");
    for(int i=0;i<3;i++){
            printf("low priority is running\n");
            currentThread->Yield();
    }
    
    printf("C1:1: low priority thread finish\n");
    
}

// this is thread two and its priority is 25.
void
priThread2(int param){
    printf("C2:0: mid priority thread coming and then yield\n");
    for(int i=0;i<3;i++){
            printf("mid priority is running\n");
            currentThread->Yield();
    }
    
    printf("C2:1: mid priority thread finish\n");  
}

// this is thread three and its priority is 50.
void
priThread3(int param){
    printf("C3:0: high priority thread coming and then yield\n"); 
    for(int i=0;i<3;i++){
            printf("high priority is running\n");
            currentThread->Yield();
    }
    printf("C3:1: high priority thread finish\n");  
}

// context swich between different priority threads, high priority one always runs first
void
PriTest(){
    DEBUG('t', "Priority Text");
    
   
    Thread *t = new Thread("one");
    t->setPriority(10);
    t->Fork(priThread1, 0);
    
    t = new Thread("two");
    t->setPriority(25);
    t->Fork(priThread2, 0);
    
    t = new Thread("three");
    t->setPriority(50);
    t->Fork(priThread3, 0);
    
}



//----------------------------------------------------------------------
// PriSema threads with different priority while waiting semaphore
//----------------------------------------------------------------------
// this is thread two and its priority is 50.
void
priSemaThread1(int para){
    printf("pS1:0: low priority thread waiting\n");
    priSemaphore->P();
    printf("pS1:1: low priority thread get signaled\n");

}

// this is thread two and its priority is 25.
void
priSemaThread2(int para){
    printf("pS2:0: high priority thread waiting\n");
    priSemaphore->P();
    printf("pS2:1: high priority thread get signaled\n");
}

// this is thread two and its priority is 10.
void
priSemaThread3(int para){
    printf("pS3:0: thread to signal coming\n");
    priSemaphore->V();
    printf("pS3:1: thread to signal signaled\n");

}

// this is thread two and its priority is 10.
void
priSemaThread4(int para){
    printf("pS4:0: another thread to signal coming\n");
    priSemaphore->V();
    printf("pS4:1: another thread to signal signaled\n");

}

// thread one and two waiting for signal
// once signaled, the high priority thread gets first.
void
PriSema(){
    DEBUG('t', "PrioritySemaphore Text");
    priSemaphore = new Semaphore("priSemaphore", 0);
    Thread *t = new Thread("one");
    t->setPriority(25);
    t->Fork(priSemaThread1, 0);
    currentThread->Yield();
    t = new Thread("two");
    t->setPriority(50);
    t->Fork(priSemaThread2, 0);
    currentThread->Yield();
    t = new Thread("three");
    t->setPriority(10);
    t->Fork(priSemaThread3, 0);
    currentThread->Yield();
    t = new Thread("four");
    t->setPriority(10);
    t->Fork(priSemaThread4, 0);
}


//----------------------------------------------------------------------
// PriCV threads with different priority while waiting condition variable
//----------------------------------------------------------------------
// this is thread two and its priority is 25.
void
priCVThread1(int para){
    printf("pCV1:0: low priority thread coming\n");
    priCVLock->Acquire();
    printf("pCV1:1: low priority thread waiting\n");
    priCV->Wait(priCVLock);
    printf("pCV1:2: low priority thread get signaled\n");
    priCVLock->Release();
    printf("pCV1:3: low priority thread release the lock\n");
}

// this is thread two and its priority is 50.
void
priCVThread2(int para){
    printf("pCV2:0: high priority thread coming\n");
    priCVLock->Acquire();
    printf("pCV2:1: high priority thread waiting\n");
    priCV->Wait(priCVLock);
    printf("pCV2:2: high priority thread get signaled\n");
    priCVLock->Release();
    printf("pCV2:3: high priority thread release the lock\n");
}

// this is thread two and its priority is 10.
void
priCVThread3(int para){
    printf("pCV3:0: thread to signal coming\n");
    priCVLock->Acquire();
    printf("pCV3:1: thread to signal holding the lock\n");
    priCV->Signal(priCVLock);
    printf("pCV3:2: thread to signal signaled\n");
    priCVLock->Release();
    printf("pCV3:3: thread to signal release lock \n");
}

// this is thread two and its priority is 10.
void
priCVThread4(int para){
    printf("pCV4:0: another thread to signal coming\n");
    priCVLock->Acquire();
    printf("pCV4:1: another thread to signal holding the lock\n");
    priCV->Signal(priCVLock);
    printf("pCV4:2: another thread to signal signaled\n");
    priCVLock->Release();
    printf("pCV4:3: another thread to signal release lock\n");

}

// thread one and two waiting for signal
// once signaled, the high priority thread gets first.
void
PriCV(){
    DEBUG('t', "Priority Condition Variable Text");
    priCV = new Condition("priCV");
    priCVLock = new Lock("priCVLock");
    Thread *t = new Thread("one");
    t->setPriority(25);
    t->Fork(priCVThread1, 0);
    currentThread->Yield();
    t = new Thread("two");
    t->setPriority(50);
    t->Fork(priCVThread2, 0);
    currentThread->Yield();
    t = new Thread("three");
    t->setPriority(10);
    t->Fork(priCVThread3, 0);
    currentThread->Yield();
    t = new Thread("four");
    t->setPriority(10);
    t->Fork(priCVThread4, 0);
}


//----------------------------------------------------------------------
// PriLock threads with different priority while competing to acquire Lock
//----------------------------------------------------------------------
void priLockThread1(int param);
void priLockThread2(int param);
void priLockThread3(int param);
void
priLockThread1(int param){
    printf("pL1:0: low priority thread coming\n");
    priLock->Acquire();
    printf("pL1:1: low priority thread holding the lock\n");

    Thread *t = new Thread("two");
    t->setPriority(50);
    t->Fork(priLockThread2, 0);//fork a high priority thread
    currentThread->Yield();
    printf("pL1:2\n");

    t = new Thread("three");
    t->setPriority(25);
    t->Fork(priLockThread3, 0);//fork a medium priority thread
    currentThread->Yield();
    priLock->Release();

    printf("pL1:3: low priority thread release the lock\n");
}

// this is thread two and its priority is 50.
void
priLockThread2(int param){
    printf("pL2:0: high priority thread coming\n");
    priLock->Acquire();
    printf("pL2:1: high priority thread holding the lock\n");
    printf("pL2:2\n");
    priLock->Release();
    printf("pL2:3: high priority thread release the lock\n");
}

// this is thread three and its priority is 25.
void
priLockThread3(int param){
    printf("pL3:0: mid priority thread coming\n");
    priLock->Acquire();
    printf("pL3:1: mid priority thread holding the lock\n");
    printf("pL3:2\n");
    priLock->Release();
    printf("pL3:3: mid priority thread release the lock\n");
}

// thread one holds the lock and other two threads wait.
// once thread one releases, the high priority thread gets the lock.
void
PriLock(){
    DEBUG('t', "PriorityLock Text");
    priLock = new Lock("priLock");
    Thread *t = new Thread("one");
    t->setPriority(10);
    t->Fork(priLockThread1, 0);   
}


//----------------------------------------------------------------------
// ExtraLock deal with priority inversion in Lock (Extra credit)
//----------------------------------------------------------------------
// this is thread two and its priority is 50.
void extraLockThread1(int param);
void extraLockThread2(int param);
void extraLockThread3(int param);
void
extraLockThread1(int para){
    printf("eL1:0: Low priority thread coming\n");
    extraLock->Acquire();
    printf("eL1:1: Low priority thread holding lock\n");
    Thread *t = new Thread("two");
    
    t->setPriority(25);
    t->Fork(extraLockThread2, 0);
    //currentThread->Yield();

    t = new Thread("three");
    t->setPriority(50);
    t->Fork(extraLockThread3, 0);
    currentThread->Yield();
    printf("eL1:2\n");
    
    extraLock->Release();
    printf("eL1:3: Low priority thread release lock\n");
}
// this is thread two and its priority is 25.
void
extraLockThread2(int para){//mid
    printf("eL2:0: mid priority thread coming\n");
    printf("eL2:1\n");
    for(int i = 0; i < 5; ++i) {
        printf("mid is running %d\n", i);
        currentThread->Yield();
    }
    printf("eL2:2\n");
    printf("mid priority thread finish\n");
}

// this is thread two and its priority is 10.
void
extraLockThread3(int para){//high
    printf("eL3:0: high priority thread coming\n");
    extraLock->Acquire();
    printf("eL3:1: high priority thread holding lock\n");
    extraLock->Release();
    printf("eL3:2: high priority thread release lock\n");

}

// thread one with low priority holding the lock,
// thread two with high priority waiting the lock,
// thread three with mid priority running
// this algorithm avoid this priority inversion.
void
ExtraLock(){
    DEBUG('t', "Priority inversion in Lock");
    extraLock = new Lock("extraLock");
    Thread *t = new Thread("one");
    t->setPriority(10);
    t->Fork(extraLockThread1, 0);
    
 
    
  
}

//----------------------------------------------------------------------
// ExtraJoin deal with priority inversion in Join (Extra credit)
//----------------------------------------------------------------------
void extraJoinThread2();
void extraJoinThread3(Thread* joinee);
void
extraJoinThread1(int para){//joinee
    Thread *joiner = new Thread("joiner", 0);  // will not be joined
    joiner->setPriority(50);  // joiner thread priority 10
    Thread *t = new Thread("middle");
    t->setPriority(25);   // middle thread priority 25
    printf("eJ1:0 this is joinee thread 3 with priority %d STEP ONE\n", (-1) * currentThread->getPriority());
    joiner->Fork((VoidFunctionPtr)extraJoinThread3, (int) currentThread);
    currentThread->Yield();
    t->Fork((VoidFunctionPtr)extraJoinThread2,0);
    currentThread->Yield();
    
    printf("eL1:1 this is joinee thread 3 with priority %d STEP TWO\n", (-1) * currentThread->getPriority());
    printf("eL1:2 this is joinee thread 3 with priority %d STEP THREE\n", (-1) * currentThread->getPriority());

}
void
extraJoinThread2(){//middle
    printf("eJ2:0 this is middle thread coming\n");
    for(int i = 0; i < 5; ++i) {
        printf("mid is running %d\n", i);
        currentThread->Yield();
    }
}
void
extraJoinThread3(Thread* joinee){//joiner
    printf("eJ3:0 this is joiner thread 1 with priority %d STEP ONE\n", (-1) * currentThread->getPriority());
    printf("eJ3:0 and my joinee has the priority %d\n",(-1) * joinee->getPriority());
    currentThread->Yield();
    currentThread->Yield();
    currentThread->Yield();
    printf("eJ3:0 my joinee is going to join me!\n");
    joinee->Join();
    printf("eJ3:1 this is joiner thread 1 with priority %d STEP TWO\n", (-1) * currentThread->getPriority());
    printf("eJ3:2 this is joiner thread 1 with priority %d STEP THREE\n", (-1) * currentThread->getPriority());
}

// thread 1 has the lowest priority and is the joinee
// thread 2 has the middle priority
// thread 3 has the highest priorty and is the joiner
void 
ExtraJoin()
{
    
    Thread *joinee = new Thread("joinee", 1);  // WILL be joined
  
    
    joinee->setPriority(10);  // joinee thread priority 50

   
    

    //fork those three
    joinee->Fork(extraJoinThread1, 0);
    currentThread->Yield();
    
    
}

//----------------------------------------------------------------------
// Whale Test
//----------------------------------------------------------------------
void Whale1(int arg1) {
    printf("A male 1 enters: \n");
    whale->Male();
    printf("A Matchmaker 1 enters: \n");
    whale->Matchmaker();
}
void Whale2(int arg1) {
    printf("A Matchmaker 2 enters: \n");
    whale->Matchmaker();
    printf("A female 2 enters: \n");
    whale->Female();
}
void Whale3(int arg1) {
    printf("A female 3 enters: \n");
    whale->Female();
}
void Whale4(int arg1) {
    printf("A male 4 enters: \n");
    whale->Male();
}
void Whale5(int arg1) {
    printf("A male 5 enters: \n");
    whale->Male();
}
void Whale6(int arg1) {
    printf("A male 6 enters: \n");
    whale->Male();
}
void Whale7(int arg1) {
    printf("A male 7 enters: \n");
    whale->Male();
}
void WhaleMateTest() {
    DEBUG('t', "Whale Text");
    whale = new Whale("whaleclass");
    Thread *t = new Thread("WhaleOne");
    t->Fork(Whale1, 0);
    t = new Thread("WhaleTwo");
    t->Fork(Whale2, 0);
    t = new Thread("WhaleThree");
    t->Fork(Whale3, 0);
    t = new Thread("WhaleFour");
    t->Fork(Whale4, 0);
    t = new Thread("WhaleFive");
    t->Fork(Whale5, 0);
    t = new Thread("WhaleSeven");
    t->Fork(Whale6, 0);
    t = new Thread("WhaleEight");
    t->Fork(Whale7, 0);
}

//----------------------------------------------------------------------
// Main Test
//----------------------------------------------------------------------
void
ThreadTest()
{
    switch (testnum) {
    case 1:
	    ThreadTest1();
	    break;
    case 2:
        LockTest1(); //noraml lock test
	    break;
    case 3:
        LockTest3(); //acquire same lock twice
        break;
    case 4:
        LockTest4(); //release lock that is not held
        break;
    case 5:
        LockTest5(); //delete a lock that is not released
        break;
    case 6:
        CvTest0(); //condition variable not holding the lock
        break;
    case 7:
        CvTest1(); //condition variable signal
        break;
    case 8:
        CvTest2(); //condition variable broadcast
        break;
    case 9:
        CvTest3(); //Signal/broadcasting but no body is waiting
        break;
    case 10:
        CvTest4(); //Condition delete queue that is not empty
        break;
    case 11:
        MailTest(); //MailTest for two senders and two receivers
        break;
    case 12:
        ForkerThread(); //common join test
        break;
    case 13:
        yieldTimes=10;  // this test is special as it allows the joinee to finish first and join after the joinee finishes
        ForkerThread();
        break;
    case 14:
        ForkerThread2(); //join itself
        break;
    case 15:
        ForkerThread3(); //finish before join() called
        break;
    case 16:
        ForkerThread4(); //join() can only be called after fork() is called
        break;
    case 17:
        ForkerThread5(); //only thread created to be joined can be joined
        break;
    case 18:
        ForkerThread6(); //join can only be called once
        break;
    case 19:
        PriTest(); //context swich between different priority threads
        break;
    case 20:
        PriLock(); //threads with different priority while competing to acquire Lock
        break;
    case 21:
        PriSema(); //threads with different priority while competing semaphore
        break;
    case 22:
        PriCV(); //threads with different priority while competing condition variable
        break; 
    case 23:
        WhaleMateTest(); //Whale Test
        break;
    case 24:
        ExtraLock(); //deal with priority inversion in Lock (Extra credit)
        break; 
    case 25:
        ExtraJoin(); //deal with priority inversion in Join (Extra credit)
        break;
    case 26:
        MailTest2(); //MailTest for one sender and two receivers
        break;
    case 27:
        MailTest3(); //MailTest for two senders and one receiver
        break;
    default:
	    printf("No test specified.\n");
	    break;
    }
}
