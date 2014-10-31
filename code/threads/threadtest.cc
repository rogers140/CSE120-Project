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




void
LockThread1(int param)
{
    printf("L1:0\n");
    locktest1->Acquire();
    printf("L1:1\n");
    currentThread->Yield();
    printf("L1:2\n");
    locktest1->Release();
    printf("L1:3\n");
}

void
LockThread2(int param)
{
    printf("L2:0\n");
    locktest1->Acquire();
    printf("L2:1\n");
    currentThread->Yield();
    printf("L2:2\n");
    locktest1->Release();
    printf("L2:3\n");
}

void
LockThread3(int param)
{
    printf("L3:0\n");
    locktest3->Acquire();
    locktest3->Acquire();
}

void
LockThread4(int param)
{
    printf("L4:0\n");
    locktest4->Release();
}

void
LockThread4_1(int param)
{
    printf("L4:0\n");
    locktest4->Acquire();
    printf("L4:1\n");
    currentThread->Yield();
}

void
LockThread4_2(int param)
{
    printf("L4:0\n");
    locktest4->Release();
}

void
LockThread5(int param)
{
    printf("L5:0\n");
    locktest5->Acquire();
}

void
cvThread0(int param){
    printf("C1:0\n");
    // cvLock1->Acquire();
    printf("C1:1\n");
    while(!sharedState){
        printf("C1:2\n");
        cv1->Wait(cvLock1);
        printf("C1:3\n");
    }
    ASSERT(sharedState);
    printf("C1:4\n");
    // cvLock1->Release();

}

void
cvThread1(int param){
    printf("C1:0\n");
    cvLock1->Acquire();
    printf("C1:1\n");
    while(!sharedState){
        printf("C1:2\n");
        cv1->Wait(cvLock1);
        printf("C1:3\n");
    }
    ASSERT(sharedState);
    printf("C1:4\n");
    cvLock1->Release();

}

void
cvThread2(int param){
    printf("C2:0\n");
    cvLock1->Acquire();
    printf("C2:1\n");
    sharedState=1;
    printf("C2:2\n");
    cv1->Signal(cvLock1);
    printf("C2:3\n");
    cvLock1->Release();
}

void
cvThread3(int param){
    printf("C3:0\n");
    cvLock1->Acquire();
    printf("C3:1\n");
    while(!sharedState){
        printf("C3:2\n");
        cv1->Wait(cvLock1);
        printf("C3:3\n");
    }
    ASSERT(sharedState);
    printf("C3:4\n");
    cvLock1->Release();
}

void
cvThread4(int param){
    printf("C4:0\n");
    cvLock1->Acquire();
    printf("C4:1\n");
    while(!sharedState){
        printf("C4:2\n");
        cv1->Wait(cvLock1);
        printf("C4:3\n");
    }
    ASSERT(sharedState);
    printf("C4:4\n");
    cvLock1->Release();
}

void
cvThread5(int param){
    printf("C5:0\n");
    cvLock1->Acquire();
    printf("C5:1\n");
    sharedState=1;
    printf("C5:2\n");
    cv1->Broadcast(cvLock1);
    // delete cv1;
    printf("C5:3\n");
    cvLock1->Release();
}

void
cvThread8(int param){
    printf("C8:0\n");
    cvLock1->Acquire();
    printf("C8:1\n");
    while(!sharedState){
        printf("C8:2\n");
        cv1->Wait(cvLock1);
        printf("C8:3\n");
    }
    ASSERT(sharedState);
    printf("C8:4\n");
    cvLock1->Release();
}

void
cvThread9(int param){
    printf("C9:0\n");
    cvLock1->Acquire();
    printf("C9:1\n");
    while(!sharedState){
        printf("C9:2\n");
        cv1->Wait(cvLock1);
        printf("C9:3\n");
    }
    ASSERT(sharedState);
    printf("C9:4\n");
    cvLock1->Release();
}

void
cvThread10(int param){
    printf("C10:0\n");
    cvLock1->Acquire();
    printf("C10:1\n");
    sharedState=1;
    printf("C10:2\n");
    // cv1->Broadcast(cvLock1);
    delete cv1;
    printf("C10:3\n");
    cvLock1->Release();
}

void
mailThread1(int param){
    printf("C1:0\n");   
    printf("%d\n",1);
    mb->Send(1);
}

void
mailThread2(int param){
    printf("C2:0\n");
    printf("%d\n",2);
    mb->Send(2);
}

void
mailThread3(int param){
    printf("C3:0\n");
    int tmp = 0;
    int *result1 = &tmp;
    mb->Receive(result1);
    printf("%d\n",*result1);
}

void
mailThread4(int param){
    printf("C4:0\n");
    int tmp = 0;
    int *result2 = &tmp;
    mb->Receive(result2);
    printf("%d\n",*result2);
}

void
priThread1(int param){
    printf("%d\n",currentThread->getPriority());
    printf("C1:0\n");
    printf("%d\n",currentThread->getPriority());
    currentThread->Yield();
    printf("C1:1\n");
    
}

void
priThread2(int param){
    printf("%d\n",currentThread->getPriority());
    printf("C2:0\n");
    currentThread->Yield();
    printf("%d\n",currentThread->getPriority());
    printf("C2:1\n");  
}

void
priThread3(int param){
    printf("%d\n",currentThread->getPriority());
    printf("C3:0\n"); 
}

void
priLockThread1(int param){
    printf("pL1:0\n");
    priLock->Acquire();
    printf("pL1:1\n");
    currentThread->Yield();
    printf("pL1:2\n");
    priLock->Release();
    printf("pL1:3\n");
}

void
priLockThread2(int param){
    printf("pL2:0\n");
    priLock->Acquire();
    printf("pL2:1\n");
    currentThread->Yield();
    printf("pL2:2\n");
    priLock->Release();
    printf("pL2:3\n");
}

void
priLockThread3(int param){
    printf("pL3:0\n");
    priLock->Acquire();
    printf("pL3:1\n");
    currentThread->Yield();
    printf("pL3:2\n");
    priLock->Release();
    printf("pL3:3\n");
}

void
priSemaThread1(int para){
    printf("pS1:0\n");
    priSemaphore->P();
    printf("pS1:1\n");

}

void
priSemaThread2(int para){
    printf("pS2:0\n");
    priSemaphore->P();
    printf("pS2:1\n");
}

void
priSemaThread3(int para){
    printf("pS3:0\n");
    priSemaphore->V();
    printf("pS3:1\n");

}

void
priSemaThread4(int para){
    printf("pS4:0\n");
    priSemaphore->V();
    printf("pS4:1\n");

}

void
priCVThread1(int para){
    printf("pCV1:0\n");
    priCVLock->Acquire();
    printf("pCV1:1\n");
    priCV->Wait(priCVLock);
    printf("pCV1:2\n");
    priCVLock->Release();
    printf("pCV1:3\n");
}

void
priCVThread2(int para){
    printf("pCV2:0\n");
    priCVLock->Acquire();
    printf("pCV2:1\n");
    priCV->Wait(priCVLock);
    printf("pCV2:2\n");
    priCVLock->Release();
    printf("pCV2:3\n");
}

void
priCVThread3(int para){
    printf("pCV3:0\n");
    priCVLock->Acquire();
    printf("pCV3:1\n");
    priCV->Signal(priCVLock);
    printf("pCV3:2\n");
    priCVLock->Release();
    printf("pCV3:3\n");
}

void
priCVThread4(int para){
    printf("pCV4:0\n");
    priCVLock->Acquire();
    printf("pCV4:1\n");
    priCV->Signal(priCVLock);
    printf("pCV4:2\n");
    priCVLock->Release();
    printf("pCV4:3\n");

}

//----------------------------------------------------------------------
// LockTest1 normal lock situation
//----------------------------------------------------------------------

void
LockTest1()
{
    DEBUG('t', "Entering LockTest1");

    locktest1 = new Lock("LockTest1");

    Thread *t = new Thread("one");
    t->Fork(LockThread1, 0);
    t = new Thread("two");
    t->Fork(LockThread2, 0);
}

//----------------------------------------------------------------------
// LockTest3 acquire the lock twice
//----------------------------------------------------------------------

void
LockTest3()
{
    DEBUG('t', "Acquire  Lock Twice");

    locktest3 = new Lock("LockTest3");

    Thread *t = new Thread("one");
    t->Fork(LockThread3, 0);


}

//----------------------------------------------------------------------
// LockTest4 release the lock that is not held
//----------------------------------------------------------------------
void
LockTest4()
{
    DEBUG('t', "Release Lock that is not held");

    locktest4 = new Lock("LockTest4");
    
    Thread *t = new Thread("one");
    //NULL situation
    //t->Fork(LockThread4, 0);
    // Two thread situation
    t->Fork(LockThread4_1, 0);
    t = new Thread("two");
    t->Fork(LockThread4_2, 0);


}

//----------------------------------------------------------------------
// LockTest5 delete a lock that is not held
//----------------------------------------------------------------------

void
LockTest5()
{
    DEBUG('t', "Delete a lock that is held");

    locktest5 = new Lock("LockTest5");

    Thread *t = new Thread("one");
    t->Fork(LockThread5, 0);
    currentThread->Yield();
    printf("%s",locktest5->holder);
    delete locktest5;

}

//----------------------------------------------------------------------
// CvTest0 Condition variable not holding the lock
//----------------------------------------------------------------------
void
CvTest0(){
    DEBUG('t', "Condition variable signal");

    cvLock1 = new Lock("cvLock0");
    cv1 = new Condition("cv0");

    Thread *t = new Thread("zero");
    t->Fork(cvThread0, 0);

}

//----------------------------------------------------------------------
// CvTest1 Condition variable signal
//----------------------------------------------------------------------
void
CvTest1(){
    DEBUG('t', "Condition variable signal");

    cvLock1 = new Lock("cvLock1");
    cv1 = new Condition("cv1");

    Thread *t = new Thread("one");
    t->Fork(cvThread1, 0);
    t = new Thread("two");
    t->Fork(cvThread2, 0);

}

//----------------------------------------------------------------------
// CvTest2 Condition variable boradcast
//----------------------------------------------------------------------
void
CvTest2(){
    DEBUG('t', "Condition variable broadcast");

    cvLock1 = new Lock("cvLock2");
    cv1 = new Condition("cv2");

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
CvTest3(){
    DEBUG('t', "Condition variable broadcast");

    cvLock1 = new Lock("cvLock3");
    cv1 = new Condition("cv3");

    Thread *t = new Thread("two");
    t->Fork(cvThread2, 0);
    t = new Thread("five");
    t->Fork(cvThread5, 0);
    t = new Thread("three");
    t->Fork(cvThread3, 0);
    t = new Thread("four");
    t->Fork(cvThread4, 0);

} // problem exist! here

//----------------------------------------------------------------------
// CvTest4 Condition delete queue that is not empty
//----------------------------------------------------------------------
void
CvTest4(){
    DEBUG('t', "Condition variable broadcast");

    cvLock1 = new Lock("cvLock4");
    cv1 = new Condition("cv4");

    Thread *t = new Thread("eight");
    t->Fork(cvThread8, 0);
    t = new Thread("nine");
    t->Fork(cvThread9, 0);
    t = new Thread("ten");
    t->Fork(cvThread10, 0);

}


void
MailTest(){
    DEBUG('t', "Mailbox Text");

    mb = new Mailbox("mailbox");

    Thread *t = new Thread("one");
    t->Fork(mailThread1, 0);
    t = new Thread("two");
    t->Fork(mailThread2, 0);
    t = new Thread("three");
    t->Fork(mailThread3, 0);
    t = new Thread("four");
    t->Fork(mailThread4, 0);

}

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

void
PriLock(){
    DEBUG('t', "PriorityLock Text");
    priLock = new Lock("priLock");
    Thread *t = new Thread("one");
    t->setPriority(10);
    t->Fork(priLockThread1, 0);
    t = new Thread("two");
    t->setPriority(50);
    t->Fork(priLockThread2, 0);
    t = new Thread("three");
    t->setPriority(25);
    t->Fork(priLockThread3, 0);
}

void
PriSema(){
    DEBUG('t', "PrioritySemaphore Text");
    priSemaphore = new Semaphore("priSemaphore", 0);
    Thread *t = new Thread("one");
    t->setPriority(50);
    t->Fork(priSemaThread1, 0);
    currentThread->Yield();
    t = new Thread("two");
    t->setPriority(25);
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

void
PriCV(){
    DEBUG('t', "Priority Condition Variable Text");
    priCV = new Condition("priCV");
    priCVLock = new Lock("priCVLock");
    Thread *t = new Thread("one");
    t->setPriority(50);
    t->Fork(priCVThread1, 0);
    currentThread->Yield();
    t = new Thread("two");
    t->setPriority(25);
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
//Ruogu modified on 10.28
//Whale Test
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
//end of Whale Test

//start of join test section

void
Joiner(Thread *joinee)
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

//----------------------------------------------------------------------
// ForkerThread1 common join test
// when yieldTimes = 10 joinee is only destroy after join() is called
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

  // fork off the two threads and let them do their business
  joiner->Fork((VoidFunctionPtr) Joiner, (int) joiner);
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
  joiner->Fork((VoidFunctionPtr) Joiner, (int) joinee);
  joinee->Fork((VoidFunctionPtr) Joinee, 0);

  joinee->Finish();

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
  joiner->Fork((VoidFunctionPtr) Joiner, (int) joinee);
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
  joiner->Join();



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
  joiner->Fork((VoidFunctionPtr) Joiner, (int) joinee);
  joinee->Fork((VoidFunctionPtr) Joinee, 0);
  joinee->Join();



  // this thread is done and can go on its merry way
  printf("Forked off the joiner and joiner threads.\n");
}

//end of join test section

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
    case 2:
        LockTest1();
	break;
    case 3:
        LockTest3();
        break;
    case 4:
        LockTest4();
        break;
    case 5:
        LockTest5();
        break;
    case 6:
        CvTest0();
        break;
    case 7:
        CvTest1();
        break;
    case 8:
        CvTest2();
        break;
    case 9:
        CvTest3();
        break;
    case 10:
        CvTest4();
        break;
    case 11:
        MailTest();
        break;
    case 12:
        ForkerThread();
    break;
    case 13:
        yieldTimes=5;
        ForkerThread();
    break;
    case 14:
        ForkerThread2();
    break;
    case 15:
        ForkerThread3();
    break;
    case 16:
        ForkerThread4();
    break;
    case 17:
        ForkerThread5();
    break;
    case 18:
        ForkerThread6();
    break;
    // case 19:
    //     PriTest();
    //     break;
    // case 20:
    //     PriLock();
    //     break;
    // case 21:
    //     PriSema();
    //     break;
    // case 22:
    //     PriCV();
    //     break; 
    // case 23:
    //     WhaleMateTest();
    //     break;
    default:
	printf("No test specified.\n");
	break;
    }
}
