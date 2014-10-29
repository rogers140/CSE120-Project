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

//----------------------------------------------------------------------
// LockTest1
//----------------------------------------------------------------------

Lock *locktest1 = NULL;
Lock *locktest2 = NULL;
Lock *locktest3 = NULL;
Lock *locktest4 = NULL;
Lock *locktest5 = NULL;

Lock *cvLock1 =NULL;
Condition *cv1 = NULL;
int sharedState = 0;

Lock *cvLock2 =NULL;
Condition *cv2 = NULL;

Mailbox *mb = NULL;

Lock *priLock = NULL;

Semaphore *priSemaphore = NULL;

Condition *priCV= NULL;
Lock *priCVLock= NULL;

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
    cvLock2->Acquire();
    printf("C3:1\n");
    while(!sharedState){
        printf("C3:2\n");
        cv2->Wait(cvLock2);
        printf("C3:3\n");
    }
    ASSERT(sharedState);
    printf("C3:4\n");
    cvLock2->Release();
}

void
cvThread4(int param){
    printf("C4:0\n");
    cvLock2->Acquire();
    printf("C4:1\n");
    while(!sharedState){
        printf("C4:2\n");
        cv2->Wait(cvLock2);
        printf("C4:3\n");
    }
    ASSERT(sharedState);
    printf("C4:4\n");
    cvLock2->Release();
}

void
cvThread5(int param){
    printf("C5:0\n");
    cvLock2->Acquire();
    printf("C5:1\n");
    sharedState=1;
    printf("C5:2\n");
    // cv2->Broadcast(cvLock2);
    delete cv2;
    printf("C5:3\n");
    cvLock2->Release();
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

void
LockTest3()
{
    DEBUG('t', "Acquire  Lock Twice");

    locktest3 = new Lock("LockTest3");

    Thread *t = new Thread("one");
    t->Fork(LockThread3, 0);


}

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

void
CvTest1(){
    DEBUG('t', "Condition variable without holding a lock");

    cvLock1 = new Lock("cvLock1");
    cv1 = new Condition("cv1");

    Thread *t = new Thread("one");
    t->Fork(cvThread1, 0);
    t = new Thread("two");
    t->Fork(cvThread2, 0);

}

void
CvTest2(){
    DEBUG('t', "Condition variable broadcast");

    cvLock2 = new Lock("cvLock2");
    cv2 = new Condition("cv2");

    Thread *t = new Thread("three");
    t->Fork(cvThread3, 0);
    t = new Thread("four");
    t->Fork(cvThread4, 0);
    t = new Thread("five");
    t->Fork(cvThread5, 0);

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
        CvTest1();
        break;
    case 7:
        CvTest2();
        break;
    case 8:
        MailTest();
        break;
    case 9:
        PriTest();
        break;
    case 10:
        PriLock();
        break;
    case 11:
        PriSema();
        break;
    case 12:
        PriCV();
        break;
    case 13:
        WhaleMateTest();
        break;
    default:
	printf("No test specified.\n");
	break;
    }
}
