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
#include "stdio.h"

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
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
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
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) 
{
	name = debugName;
	available = true;
	currOwner = NULL;
	waitQueue = new List;
}
Lock::~Lock() {}
void Lock::Acquire() 
{
	//Disable interrupts
	IntStatus oldLevel=interrupt->SetLevel(IntOff);
	
	DEBUG('l',"In lock Acquire\n");
	
	DEBUG('l',"Current Thread: %d\n", currentThread);
	DEBUG('l',"Current Owner: %d\n", currOwner);
	//If I'm the lock owner
	if(currentThread==currOwner)
	{
		printf("ERROR at Acquire: You already have the lock\n");
		interrupt->SetLevel(oldLevel);
		return;
	}
	//printf("ERROR at Acquire: not lock owner");
	DEBUG('l',"I m not the Lock Owner\n");
	
	//If lock is available
	if(available==true)
	{
		currOwner=currentThread;
		available=false;
		DEBUG('l',"Lock available\n");
	}
	//Else lock is'nt available
	else
	{
		waitQueue->Append(currentThread);
		currentThread->Sleep();
		DEBUG('l',"Lock not available\n");
	}
	
	//Restore interrupts
	interrupt->SetLevel(oldLevel);
}

void Lock::Release() 
{
	//Disable interrupts
	IntStatus oldLevel=interrupt->SetLevel(IntOff);
	
	DEBUG('l',"In release\n");
	
	Thread *thread;
		
	//If I'm not the lock owner
	if(currentThread!=currOwner)
	{
		printf("ERROR at Release: You dont have the lock to release\n");
		interrupt->SetLevel(oldLevel);
		return;
	}
	
	//If a thread is waiting
	if(!waitQueue->IsEmpty())
	{
		thread = (Thread *)waitQueue->Remove();
		if (thread != NULL)	   
		{
			scheduler->ReadyToRun(thread);
			currOwner=thread;
		}
	}
	else
	{
		available=true;
		currOwner=NULL;
	}
	
	//printf("Lock released by %d\n", currentThread);
	DEBUG('l', "Lock released by %d \n", currentThread);
	
	//Restore interrupts
	interrupt->SetLevel(oldLevel);
}

Condition::Condition(char* debugName)
{
	name = debugName;
	waitingLock=NULL;
	cvWaitQueue=new List;
}
Condition::~Condition() { }

void Condition::Wait(Lock* conditionLock)
{
	DEBUG('c',"In Wait\n");
	//printf("Current Thread: %d\n", currentThread);
	
	//Disable intterupts
    IntStatus oldLevel=interrupt->SetLevel(IntOff);
    
	if(conditionLock==NULL)
	{
		printf("ERROR at Wait: Your CV Lock points to NULL\n");
		interrupt->SetLevel(oldLevel);
		return;
	}
	
	if(waitingLock==NULL)
	{
		waitingLock=conditionLock;	//First thread
	}
	
	if(waitingLock!=conditionLock)
	{
		printf("ERROR at Wait: Locks dont match \n");
		interrupt->SetLevel(oldLevel);
		return;
	}
    
	conditionLock->Release();
	DEBUG('c',"Lock %d released\n",conditionLock);
    cvWaitQueue->Append(currentThread);
	DEBUG('c',"Appended in CVWaitQueue %d \n",currentThread);
    currentThread->Sleep();
	DEBUG('c',"Current Thead in sleep %d \n",currentThread);
    conditionLock->Acquire();
	DEBUG('c',"Lock %d acquired\n",conditionLock);
	
	//Restore interrupts
    interrupt->SetLevel(oldLevel);
}
void Condition::Signal(Lock* conditionLock)
{
	//Disable interrupts
    IntStatus oldLevel=interrupt->SetLevel(IntOff);
	
	DEBUG('c', "In Signal\n");
    
	//No thread waiting
	if(cvWaitQueue->IsEmpty())
	{
		interrupt->SetLevel(oldLevel);
		return;
	}
	
	if(waitingLock!=conditionLock)
	{
		printf("ERROR at Signal: Locks don't match\n");
		interrupt->SetLevel(oldLevel);
		return;
	}
	
	//Wakeup 1 thread from wait queue, remove from condition wait queue,put them in ready queue
	Thread* thread;
	thread=(Thread*)cvWaitQueue->Remove();
	scheduler->ReadyToRun(thread);
	
	DEBUG('c', "Thread %d ready to run\n",thread);
	
	//Wait Queue Empty
    if(cvWaitQueue->IsEmpty())
		waitingLock=NULL;
		
	//Restore Interrupts
    interrupt->SetLevel(oldLevel);
}
void Condition::Broadcast(Lock* conditionLock)
{
    while(!cvWaitQueue->IsEmpty())
	{
		Signal(conditionLock);
	}
}
