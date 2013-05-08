#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include "thread.h"
#include "list.h"

#define SC_CreateLock	16
#define SC_AcquireLock	17
#define SC_ReleaseLock	18
#define SC_DestroyLock	19	
#define SC_CreateCV		20
#define SC_SignalCV		21
#define SC_WaitCV		22
#define SC_BroadcastCV	23
#define SC_DestroyCV	24
#define SC_CreateMV		29
#define SC_GetMV		30
#define SC_SetMV		31
#define SC_DestroyMV	32
#define SC_EndSimulation 33

/* Client structure which maintains the data about the client machine */

struct client{
	int machineID;
	int mailBoxID;
};

/* Server lock structure shared across all client machines */

struct serverLockStruct{
	char *name;		// debug name of the lock - should be unique
	
	bool isToBeDeleted;	// set to true once the lock is to be destroyed
	bool isDeleted;		// set to true once the lock is destroyed
	bool valid;			// set to false when the lock is no more alive
	bool available;		// identifies whether the lock is available or taken by someone
	
	int count;		// number of clients waiting for the lock
	int lockID;		// uniquely identifies each locks shared across machines
	
	List *waitQueue;	// contains the actual identity of the waiting clients
	
	client owner;	// current owner of the lock
};

/* Server cv structure shared across all client machines */

struct serverCVStruct{
	char *name;		// debug name of the cv - should be unique
	
	bool isToBeDeleted;	// set to true once the cv is decided to be destroyed
	bool isDeleted;		// set to true once the cv is destroyed
	bool valid;			// set to false when the cv is no more alive
	bool available;		// identifies the availability of the cv
	
	int count;		// number of clients waiting in the cv
	int cvID;		// uniquely identifies each cv shared across machines
	int lockID;		// lock upon which the cv should be accessed
	
	List *waitQueue;	// contains the actual identity of the clients waiting in cv
	
	client owner;	// current owner of the lock
};

/* Server mv structure shared across all clients */ 

struct serverMVStruct{
	char *name;		// debug name of the mv - should be unique
	
	int mvID;		// uniquely identifies each monitor variable shared across client machines
	int value;		// contains the value of the monitor variable
	
	//int size;
	int count;	
	bool valid;		// set to false when the mv is destroyed	
};

struct queueStruct{
	int doYouHave[5];
	int doYouHaveReply[5];
	int syscall;
	int count;
	int value;
	int clientCount;
	int noOfReplies;
	int flag;
	
	char name[10];
	
	bool received;
	 
	List *requestWaitQueue;
	List *serverQueue;	
	List *clientQueue;
	
	queueStruct(){
		syscall=-1;
		value = -1;
		count = 0;
		name[0]='\0';
		received = false;
		clientCount = 0;
		requestWaitQueue = NULL;
		doYouHave[myMachineID] = 0;
		doYouHaveReply[myMachineID] = 1;
		noOfReplies = 0;
		flag = 0;
		//requestWaitQueue = new List;
	}	
};
