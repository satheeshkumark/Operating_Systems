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

#define MAX_PROCESS	50
#define MAX_CV 1000
#define MAX_MV 1000
#define MAX_LOCK 1000
#define MAX_THREADS 1000
#define MAX_CHAR 110
#define MAX_LENGTH 110
#define MAX_NAMELENGTH 10

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





/*------------------Project 3 - Network-------------------*/

typedef struct clientPacket{
	char name1[MAX_NAMELENGTH];
	
	int syscall;
	int index1;
	int index2;
	int value;
	int clientID;
	
	bool status;
	
	int serverArg;
	clientPacket(){
		syscall=-1;
		/*name1=(char *)malloc(MAX_CHAR*sizeof(char *));
		name2=(char *)malloc(MAX_CHAR*sizeof(char *));*/
		name1[0]='\0';
		index1=-1;
		index2=-1;
		value=-1000;
		status = false;
	}
	
	void print(){
		printf("\nSyscall: %d\tName1: %s\tClientID: %d\tServerArg: %d\tIndex1: %d\tIndex2: %d\tValue: %d",syscall,name1,clientID,serverArg,index1,index2,value);
	}
};

typedef struct serverPacket{
	bool status;
	int value;
	int index1;
	int index2;
	int syscall;
	int clientID;
	
	int serverArg;
	char name1[MAX_NAMELENGTH];
	
	void print(){
		printf("\nStatus: %d\tValue: %d",status,value);
	}
};

extern int SERVERS;
extern int nextServer;
extern int myMachineID;
extern int extraCredit;
/*------------------Project 3 - Network-------------------*/

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
	int mailBoxID;
	AddrSpace *addrSpace;
	Lock *PageTableLock;
	
};

class myTranslationEntry:public TranslationEntry{
	public:
		int pageType;
		int pageLocation;
		int processID;
		int timeStamp;
		AddrSpace *addrSpace;
};
	
extern myTranslationEntry *ipt;


//Table variables for the three structures
extern struct processTableStruct processTable[MAX_PROCESS];
extern struct LockTypeStruct UserLock[MAX_LOCK];
extern struct CVTypeStruct UserCV[MAX_CV];


extern Lock *KernelLock;
extern Lock *CVLock;
extern Lock *ProcessTableLock;
extern Lock *printLock;
extern Lock *memoryBitmapLock;
extern Lock *iptLock;

extern int nextLockIndex;
extern int nextCVIndex;
extern int nextProcessID;
extern int processCount;
extern int currentTLB;
extern int evictionOption;

extern OpenFile *swapFile;
extern BitMap *swapBitmap;
extern Lock *swapFileLock;
extern Lock *swapBitmapLock;


extern List *evictionQueue;

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
