// system.h 
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "synch.h"

#ifndef SYSTEM_H
#define SYSTEM_H
#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "stats.h"
#include "timer.h"

#define MAX_PROCESS	10
#define MAX_CV 1000
#define MAX_LOCK 1000
#define MAX_THREADS 1000
#define MAX_CHAR 50
#define MAX_LENGTH 200

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
										// called before anything else
					

extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers

//Lock table declaration
typedef struct LockTypeStruct{
	Lock *lock;
	AddrSpace *addrSpace;
	bool isToBeDeleted;
	bool isDeleted;
	int count;
};

//Condition Variable table declaration
typedef struct CVTypeStruct{
	Condition *cv;
	AddrSpace *addrSpace;
	bool isToBeDeleted;
	bool isDeleted;
	int count;		
};

//Process Table declaration
typedef struct processTableStruct
{
	int processID;
	int threadCount;
	AddrSpace *addrSpace;
};

//Table variables for the three structures
extern struct processTableStruct processTable[MAX_PROCESS];
extern struct LockTypeStruct UserLock[MAX_LOCK];
extern struct CVTypeStruct UserCV[MAX_CV];


extern Lock *KernelLock;
extern Lock *CVLock;
extern Lock *PageTableLock;
extern Lock *ProcessTableLock;
extern Lock *printLock;

extern int nextLockIndex;
extern int nextCVIndex;
extern int nextProcessID;
extern int processCount;

//Bitmap Store
extern BitMap *memoryBitmap;
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB 
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H
