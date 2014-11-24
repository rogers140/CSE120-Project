// synch.cc
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"
#include <stdio.h>
#include "utility.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    DEBUG('c', "Semaphore P operation.\n");
    while (value == 0) { 			// semaphore not available
        queue->SortedInsert((void *)currentThread,currentThread->getPriority());	// so go to sleep
        currentThread->Sleep();
    }
    value--; 					// semaphore available,
    // consume its value

    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    DEBUG('c', "Semaphore V operation.\n");
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    thread = (Thread *)queue->SortedRemove(NULL);
    if (thread != NULL)	   // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {
     queue = new List;
     name = debugName;
     isHeld = 0;
     holder = NULL;
     //oldPriority = currentThread->getPriority();
}
Lock::~Lock() {
     ASSERT(holder==NULL);
     ASSERT(queue->IsEmpty());
     delete queue;
}
void Lock::Acquire() {
     oldPriority = currentThread->getPriority();
     if(holder!=NULL){
         ASSERT(holder != currentThread);
         //acquire same lock twice is not allowed
     }
     IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
     while(isHeld){
         queue->SortedInsert((void *)currentThread, currentThread->getPriority());
         holder->setPriority((-1) * (holder->getPriority() < currentThread->getPriority() ? 
                            holder->getPriority() : currentThread->getPriority()));
         //donate priority
         //printf("donating %s to priority: %d\n", holder->getName(), holder->getPriority());
         oldPriority = holder->getPriority();
         currentThread->Sleep();
     }

     isHeld = 1;
     holder = currentThread;
     oldPriority = holder->getPriority();
     (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts

}
void Lock::Release() {
     ASSERT(holder!=NULL&&(holder == currentThread));
     //release the lock that is not held is illegal
     Thread *thread;
     IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
     thread = (Thread *)queue->SortedRemove(NULL);
     if (thread != NULL)
         scheduler->ReadyToRun(thread);
     isHeld = 0;
     holder = NULL;
     currentThread->setPriority((-1) *oldPriority);// threads release lock gain its normal priority     
     (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}
bool Lock::isHeldByCurrentThread() {
    return (holder == currentThread);
}

Condition::Condition(char* debugName) { 
    name = debugName;
    cvQueue = new List;
}
Condition::~Condition() { 
    ASSERT(cvQueue->IsEmpty());
    delete cvQueue;
}
void Condition::Wait(Lock* conditionLock) {
    ASSERT(conditionLock->holder!=NULL&&(conditionLock->holder == currentThread));
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    //printf("wait priority:%d\n", currentThread->getPriority());

    cvQueue->SortedInsert((void *)currentThread, currentThread->getPriority());
    conditionLock->Release();
    currentThread->Sleep();
    (void)interrupt->SetLevel(oldLevel);
    conditionLock->Acquire();


}
void Condition::Signal(Lock* conditionLock) { 
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    Thread *thread;
    ASSERT(conditionLock->holder == currentThread);
    thread=(Thread*)cvQueue->SortedRemove(NULL);
    //printf("signaled priority:%s\n", thread->getPriority());
    if(thread!=NULL){
        scheduler->ReadyToRun(thread);
    }
    (void)interrupt->SetLevel(oldLevel);

}
void Condition::Broadcast(Lock* conditionLock) {
    IntStatus oldLevel = interrupt->SetLevel(IntOff); 
    Thread *thread;
    ASSERT(conditionLock->holder == currentThread);
    while(!cvQueue->IsEmpty()){
        thread=(Thread*)cvQueue->Remove();
        scheduler->ReadyToRun(thread);
    }
    (void)interrupt->SetLevel(oldLevel);

}
//mailbox
Mailbox::Mailbox(char* debugName){
    mail_lock = new Lock("mail_lock");
    sendGo = new Condition("send_cv");
    recvGo = new Condition("recv_cv");
    mailItem = 0;
    name = debugName;
    isEmpty = TRUE;
}
Mailbox::~Mailbox(){};
void Mailbox::Send(int message){
    mail_lock->Acquire();   
    while(!isEmpty){
        sendGo->Wait(mail_lock); 
    }
    mailItem = message;
    //printf("the mailitem is %d\n",mailItem);
    isEmpty= FALSE;
    recvGo->Signal(mail_lock);
    mail_lock->Release();
}
void Mailbox::Receive(int* message){
    mail_lock->Acquire();
    //printf("ok\n");
    while(isEmpty){
        recvGo->Wait(mail_lock);
    }
    //printf("start fetching %d\n",mailItem);
    //printf("message used to be %d\n", *message);
    *message = mailItem;
    //printf("%d\n", *message);
    isEmpty=TRUE;
    sendGo->Signal(mail_lock); 
    //printf("signaled\n");
    mail_lock->Release();
}
char* Mailbox::getName(){
    return name;
}
//Whale
Whale::Whale(char* debugName) {
    name = debugName;
    lock = new Lock("Whale_lock");
    maleNum = 0;
    femaleNum = 0;
    matcherNum = 0;
    maleCond = new Condition("male_condition");
    femaleCond = new Condition("female_condition");
    matcherCond = new Condition("macher_condition");
}
void Whale::Male() {
    lock->Acquire();
    ++maleNum;
    while(femaleNum == 0 || matcherNum == 0) {
        maleCond->Wait(lock);
    } 
    --femaleNum;
    --matcherNum;
    --maleNum;
    femaleCond->Signal(lock);
    matcherCond->Signal(lock);
    printf("Matched!\n");

    lock->Release();
}
void Whale::Female() {
    lock->Acquire();
    ++femaleNum;
    while(maleNum == 0 || matcherNum == 0) {
        femaleCond->Wait(lock);
    }   
    --maleNum;
    --matcherNum;
    --femaleNum;
    maleCond->Signal(lock);
    matcherCond->Signal(lock);
    printf("Matched!\n");
    lock->Release();

}
void Whale::Matchmaker() {
    lock->Acquire();
    ++matcherNum;
    while(femaleNum == 0 || maleNum == 0) {
        matcherCond->Wait(lock);
    }        
    --femaleNum;
    --maleNum;
    --matcherNum;
    femaleCond->Signal(lock);
    maleCond->Signal(lock);
    printf("Matched!\n");
    lock->Release();

}
