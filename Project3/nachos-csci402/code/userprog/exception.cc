// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synch.h"
#include "math.h"
#include "machine.h"
#include "addrspace.h"
#include "table.h"
#include <stdio.h>
#include <iostream>

using namespace std;

int copyin(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes from the current thread's virtual address vaddr.
    // Return the number of bytes so read, or -1 if an error occors.
    // Errors can generally mean a bad virtual address was passed in.
    bool result;
    int n=0;			// The number of bytes copied in
    int *paddr = new int;

    while ( n >= 0 && n < len) {
      result = machine->ReadMem( vaddr, 1, paddr );
      while(!result) // FALL 09 CHANGES
	  {
			result = machine->ReadMem( vaddr, 1, paddr ); // FALL 09 CHANGES: TO HANDLE PAGE FAULT IN THE ReadMem SYS CALL
	  }	
      buf[n++] = *paddr;
     
      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    delete paddr;
    return len;
}

int copyout(unsigned int vaddr, int len, char *buf) {
    // Copy len bytes to the current thread's virtual address vaddr.
    // Return the number of bytes so written, or -1 if an error
    // occors.  Errors can generally mean a bad virtual address was
    // passed in.
    bool result;
    int n=0;			// The number of bytes copied in

    while ( n >= 0 && n < len) {
      // Note that we check every byte's address
      result = machine->WriteMem( vaddr, 1, (int)(buf[n++]) );

      if ( !result ) {
	//translation failed
	return -1;
      }

      vaddr++;
    }

    return n;
}

void Create_Syscall(unsigned int vaddr, int len) {
    // Create the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  No
    // way to return errors, though...
    char *buf = new char[len+1];	// Kernel buffer to put the name in

    if (!buf) return;

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Create\n");
	delete buf;
	return;
    }	
	buf[len]='\0';
	fileSystem->Create(buf,0);
	delete[] buf;
	
	return;
}

int Open_Syscall(unsigned int vaddr, int len) {
    // Open the file with the name in the user buffer pointed to by
    // vaddr.  The file name is at most MAXFILENAME chars long.  If
    // the file is opened successfully, it is put in the address
    // space's file table and an id returned that can find the file
    // later.  If there are any errors, -1 is returned.
    char *buf = new char[len+1];	// Kernel buffer to put the name in
    OpenFile *f;			// The new open file
    int id;				// The openfile id

    if (!buf) {
	printf("%s","Can't allocate kernel buffer in Open\n");
	return -1;
    }

    if( copyin(vaddr,len,buf) == -1 ) {
	printf("%s","Bad pointer passed to Open\n");
	delete[] buf;
	return -1;
    }

    buf[len]='\0';

    f = fileSystem->Open(buf);
    delete[] buf;

    if ( f ) {
	if ((id = currentThread->space->fileTable.Put(f)) == -1 )
	    delete f;
	return id;
    }
    else
	return -1;
}

void Write_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one. For disk files, the file is looked
    // up in the current address space's open file table and used as
    // the target of the write.
    
    char *buf;		// Kernel buffer for output
    OpenFile *f;	// Open file for output

    if ( id == ConsoleInput) return;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer for write!\n");
	return;
    } else {
        if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("%s","Bad pointer passed to to write: data not written\n");
	    delete[] buf;
	    return;
	}
    }

    if ( id == ConsoleOutput) {
      for (int ii=0; ii<len; ii++) {
	printf("%c",buf[ii]);
      }

    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    f->Write(buf, len);
	} else {
	    printf("%s","Bad OpenFileId passed to Write\n");
	    len = -1;
	}
    }

    delete[] buf;
}

int Read_Syscall(unsigned int vaddr, int len, int id) {
    // Write the buffer to the given disk file.  If ConsoleOutput is
    // the fileID, data goes to the synchronized console instead.  If
    // a Write arrives for the synchronized Console, and no such
    // console exists, create one.    We reuse len as the number of bytes
    // read, which is an unnessecary savings of space.
    char *buf;		// Kernel buffer for input
    OpenFile *f;	// Open file for output

    if ( id == ConsoleOutput) return -1;
    
    if ( !(buf = new char[len]) ) {
	printf("%s","Error allocating kernel buffer in Read\n");
	return -1;
    }

    if ( id == ConsoleInput) {
      //Reading from the keyboard
      scanf("%s", buf);

      if ( copyout(vaddr, len, buf) == -1 ) {
	printf("%s","Bad pointer passed to Read: data not copied\n");
      }
    } else {
	if ( (f = (OpenFile *) currentThread->space->fileTable.Get(id)) ) {
	    len = f->Read(buf, len);
	    if ( len > 0 ) {
	        //Read something from the file. Put into user's address space
  	        if ( copyout(vaddr, len, buf) == -1 ) {
		    printf("%s","Bad pointer passed to Read: data not copied\n");
		}
	    }
	} else {
	    printf("%s","Bad OpenFileId passed to Read\n");
	    len = -1;
	}
    }

    delete[] buf;
    return len;
}

void Close_Syscall(int fd) {
    // Close the file associated with id fd.  No error reporting.
    OpenFile *f = (OpenFile *) currentThread->space->fileTable.Remove(fd);

    if ( f ) {
      delete f;
    } else {
      printf("%s","Tried to close an unopen file\n");
    }
}

int numThreads=0;
int numProcesses=0;
int flag=0;


// Yield System Call

void Yield_Syscall() 
{
	currentThread->Yield();
}

#ifdef NETWORK
void send(char *syscallType,clientPacket packetToSend){		/*Function that performs PostOffice->Send, which takes the data from client to server*/
	PacketHeader pFromCToS;									/*Creating an object of type PacketHeader, which has the information about the data in the packet*/
	MailHeader mFromCToS;									/*Creating an object of type MailHeader, which has the mail ID*/
	
	int len=sizeof(packetToSend);
	char* data=new char[len+1];								/*Creating a char array to hold the packet information*/
	memcpy((void*)data,(void *)&packetToSend,len);
	data[len]='\0';
		
	pFromCToS.to=0;											/*Setting the machine ID and the MailBox ID for the client and server*/
	mFromCToS.to=0;
	mFromCToS.from=0;
	mFromCToS.length=len+1;	
	
	bool s=postOffice->Send(pFromCToS,mFromCToS,data);		/*Performing the "send", which transfers the packet from client to server*/
	if(!s){
		printf("\n%s : COULDNT SEND DATA",syscallType);	
		interrupt->Halt();
	}
}

int receive(char *syscallType){								/*Function that performs PostOffice->Receive, which receives the data from server and gives it to client*/
	PacketHeader pFromSToC;									/*Creating an object of type PacketHeader, which has the information about the data in the packet*/
	MailHeader mFromSToC;									/*Creating an object of type MailHeader, which has the mail ID*/
	serverPacket packetToReceive;							
	
	int len=sizeof(packetToReceive);
	char *data=new char[len+1];

	postOffice->Receive(0,&pFromSToC,&mFromSToC,data);		/*Performing the "receive" function, that receives the data from server to client*/
	
	memcpy((void *)&packetToReceive,(void*)data,len);		/*Performing copy function into a packet structure*/
	
	if(!packetToReceive.status)
		return -1;
	return packetToReceive.value;
} 

serverPacket receiveAckPacket(char *syscallType){			/*Function that performs PostOffice->Receive , whcih is mainly used for handling Monitor Variables*/
	PacketHeader pFromSToC;									/*Creating an object of type PacketHeader, which has the information about the data in the packet*/		
	MailHeader mFromSToC;									/*Creating an object of type MailHeader, which has the mail ID*/
	serverPacket packetToReceive;
	
	int len=sizeof(packetToReceive);
	char *data=new char[len+1];

	postOffice->Receive(0,&pFromSToC,&mFromSToC,data); 		/*Performing the "receive" function, that receives the data from server to client*/
	memcpy((void *)&packetToReceive,(void*)data,len);		/*Performing copy function into a packet structure*/
	
	return packetToReceive;
}


#endif

// Create Lock System Call

int CreateLock_Syscall(int vaddr, int len) 
{
	int lockID;
	int localFlag;
	
	printLock->Acquire();
	localFlag=flag;
	printLock->Release();

	KernelLock->Acquire();
		
	if (len < 0 || len > MAX_CHAR)
	{
		printf("\nCREATELOCK : INVALID LOCK LENGTH");		
		KernelLock->Release();
		interrupt->Halt();
		return -1;
	}
	
	if(vaddr<0 || (vaddr+len)>currentThread->space->getNumPages()*PageSize){
		printf("\nCREATELOCK : INVALID LOCK VIRTUAL ADDRESS");		
		KernelLock->Release();
		interrupt->Halt();
		return -1;
	}
	
	char *buf;
	buf=new char[len+1];
	buf[len]='\0';
	copyin(vaddr,len,buf);
	
	#ifdef NETWORK
		
		clientPacket packetToSend;						/*Creating a packet, which initializes itself with the constructor*/
		strncpy(packetToSend.name1,buf,len);			/*Copying the required data into the packet*/	
		packetToSend.syscall=SC_CreateLock;				/*Assigning the syscall number*/
		
		send("CREATELOCK",packetToSend);				/*calls the send function which will perform postoffice->send*/
		lockID=receive("CREATELOCK");					/*Receives the index and stores it in a variable*/
		
		if(lockID==-1){									/*Halts the program if the index received is -1*/
			printf("\nCREATELOCK : ERROR CREATING LOCK");
			interrupt->Halt();
		}
		else											/*The lock is created if the program comes to this else part*/
			printf("\nCREATELOCK : LOCK %d CREATED",lockID);
	#else	
		

		if(nextLockIndex >= MAX_LOCK) 
		{
			printf("\nCREATELOCK : LOCK LIMIT REACHED. NO MORE LOCKS FOR YOU");
			KernelLock->Release();
			interrupt->Halt(); 
			return -1;
		}

		UserLock[nextLockIndex].lock = new Lock(buf);
		UserLock[nextLockIndex].addrSpace = currentThread->space;
		UserLock[nextLockIndex].isToBeDeleted = false;
		UserLock[nextLockIndex].isDeleted = false;
		UserLock[nextLockIndex].count = 0;
		lockID = nextLockIndex++;
		if(localFlag==0)
			printf("\nCREATELOCK : LOCK %d CREATED\n",lockID);
		for(int i=0;i<len;i++){
			printf("%c",buf[i]);
		}
	#endif
	KernelLock->Release();
	return lockID;
}

// Lock: Acquire Lock System Call

void AcquireLock_Syscall(int lockID) 
{
	int localFlag;
	
	printLock->Acquire();								
	localFlag=flag;
	printLock->Release();
	
	#ifdef NETWORK
		clientPacket packetToSend;								/*Creating a packet, which initializes itself with the constructor*/
		                                                       	
		packetToSend.syscall = SC_AcquireLock;                  /*Assigning the syscall number*/
		packetToSend.index1 = lockID;                           
		                                                        
		send("AQUIRELOCK",packetToSend);                        /*calls the send function which will perform postoffice->send*/
		lockID=receive("ACQUIRELOCK");                          /*Receives the index and stores it in a variable*/
		                                                          
		if(lockID==-1){                                         /*Halts the program if the index received is -1*/
			printf("\nLOCK ACQUIRE : ERROR ACQUIRING LOCK");       
			interrupt->Halt();                                     
		}                                                          
		else                                                    /*The lock is acquired if the program comes to this else part*/
			printf("\nLOCK ACQUIRE : ACQUIRED LOCK %d",lockID);
		
	#else	
	
	KernelLock->Acquire();
	if(lockID >= nextLockIndex || lockID < 0) 
	{
		printf("\nLOCK ACQUIRE : INVALID LOCK NUMBER");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(currentThread->space != UserLock[lockID].addrSpace) 
	{
		printf("\nLOCK ACQUIRE: THREAD %d FROM PROCESS %d DO NOT HAVE ACCESS TO THIS ADDRESS SPACE",currentThread->threadID,currentThread->myProcessID);
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(UserLock[lockID].isDeleted || UserLock[lockID].isToBeDeleted) 
	{
		printf("\nLOCK AQCUIRE : THE LOCK YOU ARE TRYING TO ACQUIRE IS NO LONGER ALIVE");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
		
	UserLock[lockID].count++;
	KernelLock->Release();
	UserLock[lockID].lock->Acquire();
	if(localFlag==0)
		printf("\nLOCK ACQUIRE : THREAD %d FROM PROCESS %d ACQUIRED LOCK %d",currentThread->threadID,currentThread->myProcessID,lockID);
	#endif
}

// Lock: Release Lock System Call

void ReleaseLock_Syscall(int lockID) 
{
	int localFlag;
	
	printLock->Acquire();
	localFlag=flag;
	printLock->Release();
	
	#ifdef NETWORK
		clientPacket packetToSend;									/*Creating a packet, which initializes itself with the constructor*/						
		                                                            
		packetToSend.syscall = SC_ReleaseLock;                      /*Assigning the syscall number*/
		packetToSend.index1 = lockID;                               
		                                                            
		send("RELEASELOCK",packetToSend);                           /*calls the send function which will perform postoffice->send*/
		lockID=receive("RELEASELOCK");                              /*Receives the index and stores it in a variable*/
		                                                              
		if(lockID==-1){                                             /*Halts the program if the index received is -1*/
			printf("\nLOCK RELEASE : ERROR RELEASING LOCK");           
			interrupt->Halt();                                         
		}                                                              
		else                                                        /*The lock is released if the program comes to this else part*/
			printf("\nLOCK RELEASE : RELEASED LOCK %d",lockID);
		
	#else
	
	KernelLock->Acquire();
	if(lockID >= nextLockIndex || lockID < 0) 
	{
		printf("\nLOCK RELEASE : INVALID LOCK NUMBER");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(currentThread->space != UserLock[lockID].addrSpace) 
	{
		printf("\nLOCK RELEASE : THREAD %d FROM PROCESS %d YOU DO NOT HAVE ACCESS TO THIS ADDRESS SPACE",currentThread->threadID,currentThread->myProcessID);
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(UserLock[lockID].isDeleted) 
	{
		printf("\nLOCK RELEASE : THE LOCK YOU ARE TRYING TO RELEASE IS NO LONGER ALIVE");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	
	UserLock[lockID].lock->Release();
	UserLock[lockID].count--;
	if(UserLock[lockID].isToBeDeleted && UserLock[lockID].count == 0) 
	{
		UserLock[lockID].isDeleted = true;
		delete UserLock[lockID].lock;
	}
	if(localFlag==0)
		printf("\nLOCK RELEASE : THREAD %d FROM PROCESS %d RELEASED LOCK %d",currentThread->threadID,currentThread->myProcessID,lockID);
	KernelLock->Release();
	#endif
}

// Destroy Lock System Call

void DestroyLock_Syscall(int lockID) 
{
	int localFlag;
	
	printLock->Acquire();
	localFlag=flag;
	printLock->Release();
	
	
	#ifdef NETWORK
	
		clientPacket packetToSend;								/*Creating a packet, which initializes itself with the constructor*/
		                        
		packetToSend.syscall = SC_DestroyLock;                  /*Assigning the syscall number*/
		packetToSend.index1 = lockID; 
		send("DESTROYLOCK",packetToSend);                       /*calls the send function which will perform postoffice->send*/
		lockID=receive("DESTROYLOCK");                         	/*Receives the index and stores it in a variable*/ 
		                                                  
		if(lockID==-1){                                         /*Halts the program if the index received is -1*/  
			printf("\nDESTROYLOCK : ERROR DESTROYING LOCK");   
			interrupt->Halt();                                     
		}                                                      	/*The lock is destryed if the program comes to this else part*/    
		else                                                       
			printf("\nLOCK IS DESTROYED");                       
			return;

	#else
	KernelLock->Acquire();
	
	if(lockID > nextLockIndex || lockID < 0) 
	{
		printf("\nLOCK DESTROY : INVALID LOCK NUMBER");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(currentThread->space != UserLock[lockID].addrSpace) 
	{
		printf("\nLOCK DESTROY : THREAD %d FROM PROCESS %d YOU DO NOT HAVE ACCESS TO THIS ADDRESS SPACE",currentThread->threadID,currentThread->myProcessID);
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(UserLock[lockID].isDeleted) {
		printf("\nLOCK DESTROY : THE LOCK YOU ARE TRYING TO DESTROY IS NO LONGER ALIVE");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	
	UserLock[lockID].isToBeDeleted = true;
	
	if(UserLock[lockID].count == 0) 
	{
		UserLock[lockID].isDeleted = true;
		delete UserLock[lockID].lock;
	}
	if(localFlag==0)
		printf("\nLOCK DESTROY : LOCK %d DESTROYED",lockID);
	KernelLock->Release();
	#endif
}

// CreateCV System Call

int CreateCV_Syscall(int vaddr, int len) 
{
	int cvID;
	int localFlag;
	
	printLock->Acquire();
	localFlag=flag;
	printLock->Release();
	
	KernelLock->Acquire();
		
	if (len < 0 || len > MAX_CHAR)
	{
		printf("\nCREATECV : INVALID CV LENGTH");		
		KernelLock->Release();
		interrupt->Halt();
		return -1;
	}
	
	if(vaddr<0 || (vaddr+len)>currentThread->space->getNumPages()*PageSize){
		printf("\nCREATECV : INVALID CV VIRTUAL ADDRESS");		
		KernelLock->Release();
		interrupt->Halt();
		return -1;
	}
	
	char *buf;
	buf=new char[len+1];
	buf[len]='\0';
	copyin(vaddr,len,buf);
	
	#ifdef NETWORK
		clientPacket packetToSend;							/*Creating a packet, which initializes itself with the constructor*/
		strncpy(packetToSend.name1,buf,len);                
		packetToSend.syscall=SC_CreateCV;                   /*Assigning the syscall number*/
		send("CREATECV",packetToSend);                      /*calls the send function which will perform postoffice->send*/
		cvID=receive("CREATECV");                           /*Receives the index and stores it in a variable*/ 
		if(cvID==-1){                                       /*Halts the program if the index received is -1*/     
			printf("\nCREATECV : ERROR CREATING CV");        
			interrupt->Halt();
		}
		else												/*The CV is created if the program comes to this else part*/
			printf("\nCREATECV : CV %d CREATED",cvID);
	#else
						
		if(nextCVIndex >= MAX_CV) 
		{
			printf("\nCREATECV : LOCK LIMIT REACHED. NO MORE LOCKS FOR YOU");
			KernelLock->Release();
			interrupt->Halt(); 
			return -1;
		}

		UserCV[nextCVIndex].cv = new Condition(buf);
		UserCV[nextCVIndex].addrSpace = currentThread->space;
		UserCV[nextCVIndex].isToBeDeleted = false;
		UserCV[nextCVIndex].isDeleted = false;
		UserCV[nextCVIndex].count = 0;

		cvID = nextCVIndex++;
		if(localFlag==0)
			printf("\nCREATECV : CV %d CREATED",cvID);
	#endif
	KernelLock->Release();
	return cvID;
}

//Signal System Call

void SignalCV_Syscall(int lockID, int cvID) {

	int localFlag;
	
	printLock->Acquire();
	localFlag=flag;
	printLock->Release();
	
	#ifdef NETWORK
	
		clientPacket packetToSend;							/*Creating a packet, which initializes itself with the constructor*/
		packetToSend.syscall=SC_SignalCV;                   /*Assigning the syscall number*/
		packetToSend.index1=lockID;                         /*Assigning the lock's index*/
		packetToSend.index2=cvID;                           /*Assigning the CV's index*/
		send("SIGNALCV",packetToSend);                      /*calls the send function which will perform postoffice->send*/
		cvID = receive("SIGNALCV");                         /*Receives the index and stores it in a variable*/
		if(cvID == -1)                                      /*Halts the program if the index received is -1*/      
			{                                               
				printf("\nERROR OCCURED IN SIGNALCV");      
				interrupt->Halt();                         
			}
		else												/*The CV is signalled if the program comes to this else part*/
			{
				printf("\nCV SIGNAL IS SUCCESSFUL");
				return;
			}
	#else
	
	KernelLock->Acquire();
	
	if(lockID >= nextLockIndex || lockID < 0) {
		printf("\nCREATELOCK : INVALID LOCK NUMBER");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(cvID >= nextCVIndex || cvID < 0) {
		printf("\nSIGNALCV : INVALID CONDITION VARIABLE NUMBER");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(currentThread->space != UserLock[lockID].addrSpace) {
		printf("\nSIGNALCV : LOCK YOU ARE TRYING TO USE DOESNT HAVE ACCESS TO THIS ADDRESS SPACE");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(currentThread->space != UserCV[cvID].addrSpace) {
		printf("\nSIGNALCV : THE CONDITION VARIABLE YOU ARE TRYING TO USE DOESNT HAVE ACCESS TO THIS ADDRESS SPACE");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(UserCV[cvID].isDeleted) {
		printf("\nSIGNALCV : YOU ARE TRYING TO SIGNAL TO A CONDITION VARIABLE THAT IS NO LONGER ALIVE");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(UserLock[lockID].isDeleted || UserLock[lockID].isToBeDeleted) {
		printf("\nSIGNALCV : YOU ARE TRYING TO USE A LOCK THAT IS NO LONGER ALIVE");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}

	UserCV[cvID].cv->Signal(UserLock[lockID].lock);
	if(localFlag==0)
		printf("\nSIGNALCV:THREAD %d SIGNALLED CV %d",currentThread->threadID,cvID);
	UserCV[cvID].count--;

	// If isToBeDeleted Flag is set and if no one is waiting on the lock, then delete the CV
	
	if(UserCV[cvID].isToBeDeleted && UserCV[cvID].count == 0) {
		if(localFlag==0)
			printf("\nSIGNALCV : CV %d IS DELETED", cvID);
		UserCV[cvID].isDeleted = true;
		delete UserCV[cvID].cv;
	}
	KernelLock->Release();
	#endif
}

// Condition Variable: Wait System Call

void WaitCV_Syscall(int lockID, int cvID) 
{
	int localFlag;
	
	printLock->Acquire();
	localFlag=flag;
	printLock->Release();
	
	#ifdef NETWORK
	
		clientPacket packetToSend;						/*Creating a packet, which initializes itself with the constructor*/
		packetToSend.syscall=SC_WaitCV;                 /*Assigning the syscall number*/
		packetToSend.index1=lockID;                     /*Assigning the lock's index*/
		packetToSend.index2=cvID;                       /*Assigning the CV's index*/
		send("WAITCV",packetToSend);                    /*calls the send function which will perform postoffice->send*/
		cvID = receive("WAITCV");                       /*Receives the index and stores it in a variable*/
		if(cvID == -1)                                  /*Halts the program if the index received is -1*/      
			{                                           
				printf("\nERROR OCCURED IN WAITCV");    
				interrupt->Halt();                      
			}                                           
		else                                            /*The CV is put to wait if the program comes to this else part*/
			{
				printf("\nCV WAIT IS SUCCESSFUL");
				return;
			}
	#else
	KernelLock->Acquire();
	// Checking all the possible conditions
	if(lockID >= nextLockIndex || lockID < 0) 
	{
		printf("\nWAITCV : INVALID LOCK NUMBER");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(cvID >= nextCVIndex || cvID < 0) 
	{
		printf("\nWAITCV : INVALID CONDITION VARIABLE NUMBER");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(currentThread->space != UserLock[lockID].addrSpace) 
	{
		printf("\nWAITCV : LOCK YOU ARE TRYING TO USE DOESNT HAVE ACCESS TO THIS ADDRESS SPACE");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(currentThread->space != UserCV[cvID].addrSpace) 
	{
		printf("\nWAITCV : CV YOU ARE TRYING TO USE DOESNT HAVE ACCESS TO THIS ADDRESS SPACE");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(UserCV[cvID].isDeleted || UserCV[cvID].isToBeDeleted) 
	{
		printf("\nWAITCV : YOU ARE TRYING TO WAIT TO A CONDITION VARIABLE THAT IS NO LONGER ALIVE");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(UserLock[lockID].isDeleted || UserLock[lockID].isToBeDeleted) 
	{
		printf("\nWAITCV : YOU ARE TRYING TO USE A LOCK THAT IS NO LONGER ALIVE");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	UserCV[cvID].count++;
	KernelLock->Release();
	if(localFlag==0)
		printf("\nWAITCV: THREAD %d WAITING IN CV %d ",currentThread->threadID,cvID);
	UserCV[cvID].cv->Wait(UserLock[lockID].lock);
	#endif
}

// Condition Variable: Broadcast System Call

void BroadcastCV_Syscall(int lockID, int cvID)
 {
	int localFlag;
	
	printLock->Acquire();
	localFlag=flag;
	printLock->Release();
	#ifdef NETWORK

		clientPacket packetToSend;							/*Creating a packet, which initializes itself with the constructor*/
		packetToSend.syscall=SC_BroadcastCV;                /*Assigning the syscall number*/
		packetToSend.index1=lockID;                         /*Assigning the lock's index*/
		packetToSend.index2=cvID;                           /*Assigning the CV's index*/
		send("BROADCASTCV",packetToSend);                   /*calls the send function which will perform postoffice->send*/
		cvID = receive("BROADCASTCV");                      /*Receives the index and stores it in a variable*/
		if(cvID == -1)                                      /*Halts the program if the index received is -1*/      
			{                                               
				printf("\nERROR OCCURED IN BROADCASTCV");   
				interrupt->Halt();                          
			}                                               
		else                                                /*The CV is broadcasted if the program comes to this else part*/
			{
				printf("\nCV BROADCAST IS SUCCESSFUL");
				return;
			}
	#else
	KernelLock->Acquire();
	// Checking all the possible conditions
	if(lockID >= nextLockIndex || lockID < 0) 
	{
		printf("\nBROADCASTCV : INVALID LOCK NUMBER");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(cvID >= nextCVIndex || cvID < 0) 
	{
		printf("\nBROADCASTCV : INVALID CONDITION VARIABLE NUMBER");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(currentThread->space != UserLock[lockID].addrSpace) 
	{
		printf("\nBROADCASTCV : THE LOCK YOU ARE TRYING TO USE DOESNT HAVE ACCESS TO THIS ADDRESS SPACE");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(currentThread->space != UserCV[cvID].addrSpace) 
	{
		printf("\nBROADCASTCV : CV YOU ARE TRYING TO USE DOESNT HAVE ACCESS TO THIS ADDRESS SPACE");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(UserCV[cvID].isDeleted || UserCV[cvID].isToBeDeleted) 
	{
		printf("\nBROADCASTCV : YOU ARE TRYING TO SIGNAL TO A CV THAT IS NO LONGER ALIVE");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(UserLock[lockID].isDeleted) 
	{
		printf("\nBROADCASTCV : YOU ARE TRYING TO USE A LOCK THAT IS NO LONGER ALIVE");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}

	UserCV[cvID].cv->Broadcast(UserLock[lockID].lock);
	if(localFlag==0)
	printf("\nBROADCASTCV: THREAD %d BROADCASTED  CV %d ",currentThread->threadID,cvID);
	UserCV[cvID].count = 0;
	
	if(UserCV[cvID].isToBeDeleted) {
		if(localFlag==0)
		printf("\nBROADCASTCV : CV %d IS DELETED", cvID);
		UserCV[cvID].isDeleted = true;
		delete UserCV[cvID].cv;
	}
	KernelLock->Release();
#endif-
}

//DestroyCV System Call

void DestroyCV_Syscall(int cvID) 
{
	int localFlag;
	
	printLock->Acquire();
	localFlag=flag;
	printLock->Release();
	#ifdef NETWORK
	
		clientPacket packetToSend;								/*Creating a packet, which initializes itself with the constructor*/
		packetToSend.syscall = SC_DestroyCV;                    /*Assigning the syscall number*/
		packetToSend.index1 = cvID;                             /*Assigning the lock's index*/
		send("DESTROYCV",packetToSend);                         /*calls the send function which will perform postoffice->send*/
		cvID=receive("DESTROYCV");                              /*Receives the index and stores it in a variable*/
		if(cvID==-1){                                           /*Halts the program if the index received is -1*/      
			printf("\\nDESTROYCV : ERROR DESTROYING CV");       
			interrupt->Halt();                                  
		}                                                       
		else                                                    /*The CV is destroyed if the program comes to this else part*/
			printf("\nCV IS DESTROYED");                        
			return;                                             

	#else
	KernelLock->Acquire();	
	if(cvID >= nextCVIndex || cvID < 0) 
	{
		printf("\nDESTROYCV : INVALID CONDITION VARIABLE NUMBER");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(currentThread->space != UserCV[cvID].addrSpace)
	{
		printf("\nDESTROYCV : THE CV YOU ARE TRYING TO USE DOESNT HAVE ACCESS TO THIS ADDRESS SPACE");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}
	if(UserCV[cvID].isDeleted || UserCV[cvID].isToBeDeleted) 
	{
		printf("\nDESTROYCV : YOU ARE TRYING TO SIGNAL TO A CV THAT IS NO LONGER ALIVE");
		KernelLock->Release();
		interrupt->Halt();
		return;
	}

	UserCV[cvID].isToBeDeleted = true;
	if(UserCV[cvID].count == 0) 
	{
		if(localFlag==0)
			printf("\nDESTROYCV : CV %d IS DESTROYED",cvID);
		UserCV[cvID].isDeleted = true;
		delete UserCV[cvID].cv;
	}
	KernelLock->Release();
	#endif
}  

// Exec Thread System Call

void Exec_thread()
{
	currentThread->space->InitRegisters();
    currentThread->space->RestoreState();
	machine->Run();
}

int Exec_Syscall(int vaddr,int len) 
{
	int localFlag;
	
	printLock->Acquire();
	localFlag=flag;
	printLock->Release();
	
	numProcesses++;
	
	if(numProcesses == MAX_PROCESS)
	{
		printf("\nEXEC : SORRY. MAXIMUM PROCESSES ALREADY IN EXECUTION");
		interrupt->Halt();
	}
	
	char *buf;
	int i;
	buf=new char[len+1];
	buf[len]='\0';
	if ( copyin(vaddr,len,buf) == -1 ) {
	    printf("\nEXEC : BAD POINTER PASSED");
		return -1;
	}
	
	//Opening the process file and storing its pointer
	//OpenFile *executable = fileSystem->Open(buf);
    
	//Create new thread for execution of process
	Thread *newThread = new Thread("Exec Thread");
	
		
	//Creating new address space for this executable.
	processTable[currentThread->myProcessID].PageTableLock->Acquire();
	AddrSpace *newAddrSpace = new AddrSpace(buf);
	int pages=newAddrSpace->getNumPages();
	
	//Allocate the space created to this thread's space
	newThread->space = newAddrSpace;
	newThread->numPages = pages;
	processTable[currentThread->myProcessID].PageTableLock->Release();
	
	//Updating the Process table and related Data Structure
	ProcessTableLock->Acquire();
	int spaceID = nextProcessID++;
	newThread->threadID=processTable[spaceID].threadCount;
	processTable[spaceID].processID=spaceID;
	processTable[spaceID].threadCount++;
	newThread->myProcessID=spaceID;
	processTable[spaceID].addrSpace= newAddrSpace;
	processCount++;
	ProcessTableLock->Release();

	//Fork the thread for execution of process
	newThread->Fork((VoidFunctionPtr)Exec_thread,0);
	if(localFlag==0)
		printf("\nEXEC : PROCESS %d CREATED",newThread->myProcessID);
	return spaceID;	
}

// Fork Thread System Call

void kernelThread(int vaddr)
{	
	machine->WriteRegister(PCReg,vaddr);
	machine->WriteRegister(NextPCReg, vaddr+4);
	currentThread->space->RestoreState();
	machine->WriteRegister(StackReg,currentThread->myStackTop);
	machine->Run();
}

void Fork_Syscall(int vaddr)
{	
	int pages,stackStartLocation;
	int localFlag;
	
	printLock->Acquire();
	localFlag=flag;
	printLock->Release();
	
	numThreads++;
	if(numThreads == MAX_THREADS)
	{
		printf("\nFORK : MAXIMUM THREADS LIMIT REACHED");
		interrupt->Halt();
	}
	
	//Creating new thread
	Thread *newThread = new Thread("Fork Thread");
		
	//Acquire page table lock to update the Page Table 
	processTable[currentThread->myProcessID].PageTableLock->Acquire();
	AddrSpace *currentAddrSpace = currentThread->space;
	pages = currentAddrSpace->allocateStack();
	//printf("\nStack allocated for thread %d, pageCount: %d",newThread->threadID,pages);
	stackStartLocation = pages*PageSize-16;
	processTable[currentThread->myProcessID].PageTableLock->Release();
	
	//Acquire Process Table Lock to update the Process Table
	ProcessTableLock->Acquire();
	int pid = currentThread->myProcessID;	
	newThread->threadID = processTable[pid].threadCount;
	processTable[pid].threadCount++;
	ProcessTableLock->Release();
		
	newThread->myProcessID=pid;
	newThread->space = currentAddrSpace;
	newThread->myStackTop = stackStartLocation;
	newThread->numPages = pages;

	//Forking the Kernel Thread
	newThread->Fork((VoidFunctionPtr)kernelThread,vaddr);
	//if(localFlag==0)
		printf("\nFORK : THREAD FORKED FOR PROCESS %d,FORKED THREAD ID : %d",pid,newThread->threadID);

}

// Exit System Call

void Exit_Syscall(int vaddr)
{
	int localFlag;
	
	printLock->Acquire();
	localFlag=flag;
	printLock->Release();
	
	if(vaddr == 0)
	{
		int s=currentThread->myProcessID;
	
		//Deallocate the stack for this thread.
		processTable[currentThread->myProcessID].PageTableLock->Acquire();
			currentThread->space->clearMemory();
		processTable[currentThread->myProcessID].PageTableLock->Release();
		
		ProcessTableLock->Acquire();
		processTable[s].threadCount--;
		
		//Check if all the threads in the Process are finished.
		if(processTable[s].threadCount == 0)
		{
			delete processTable[s].addrSpace;
			processTable[s].processID=-1;
			processTable[s].addrSpace= NULL;
			
			processCount--;
		}
		
		//Check if all the Processes are finished.
		if(processCount==0)
		{
			if(localFlag==0)
				printf("\nEXIT : THREAD FINISHED EXECUTION -1");
			interrupt->Halt();
		}
		
		ProcessTableLock->Release();
		if(localFlag==0)
			printf("\nEXIT : THREAD FINISHED EXECUTION -2");
		currentThread->Finish();
	}
	else
	{
		printf("\nEXIT : USER PROGRAM RESULT : %d",vaddr);
		if(localFlag==0)
			printf("\nEXIT : THREAD FINISHED EXECUTION -3");
		currentThread->Finish();
	}
	
}

#ifdef NETWORK

int CreateMV_Syscall(int vaddr,int len,int initialValue){		/*System call for creating a monitor variable*/
	
	int mvID;
	int localFlag;
	KernelLock->Acquire();
	
	printLock->Acquire();
	localFlag=flag;
	printLock->Release();
	
	if (len < 0 || len > MAX_CHAR)
	{
		printf("\nCREATEMV : INVALID MV LENGTH");		
		KernelLock->Release();
		interrupt->Halt();
		return -1;
	}
	
	if(vaddr<0 || (vaddr+len)>currentThread->space->getNumPages()*PageSize){
		printf("\nCREATEMV : INVALID MV VIRTUAL ADDRESS");		
		KernelLock->Release();
		interrupt->Halt();
		return -1;
	}
	
	char *buf;													/*Creating a char array to store the name*/
	buf=new char[len+1];
	buf[len]='\0';
	copyin(vaddr,len,buf);										
	
	clientPacket packetToSend;									/*Creating a packet in the client side*/
	strncpy(packetToSend.name1,buf,len);						/*Assigning the required datea in the packet*/
	packetToSend.syscall=SC_CreateMV;							
	packetToSend.value = initialValue;
	send("CREATEMV",packetToSend);								/*calls the send function which will perform postoffice->send*/
	mvID=receive("CREATEMV");
	if(mvID==-1){												/*Halts the program if the index received is -1*/
		printf("\nCREATEMV : ERROR CREATING MV");
		interrupt->Halt();
	}
	printf("\nCREATEMV : MV %d CREATED",mvID);					/*The MV is created if the program comes to this else part*/
	KernelLock->Release();
	return mvID;
}

void SetMV_Syscall(int mvID,int value){							/*System call for setting a monitor variable*/
	int localFlag;
	
	printLock->Acquire();
	localFlag=flag;
	printLock->Release();
	clientPacket packetToSend;									/*Creating a packet in the client side*/
	packetToSend.syscall = SC_SetMV;                            /*Assigning the required datea in the packet*/
	packetToSend.index1 = mvID;                                 
	packetToSend.value = value;                                 
	send("SETMV",packetToSend);                                 /*calls the send function which will perform postoffice->send*/
	serverPacket receivedPacket = receiveAckPacket("SETMV");    /*Receives the index and stores it in a variable*/
	if(receivedPacket.status == true)                           
		return;                                                 
		                                                        
	printf("\nSETMV : ERROR SETTING MV");
	interrupt->Halt();                                          
}

int GetMV_Syscall(int mvID){									/*System call for getting a monitor variable*/
	int localFlag;
	
	printLock->Acquire();
	localFlag=flag;
	printLock->Release();
	clientPacket packetToSend;									/*Creating a packet in the client side*/
	packetToSend.syscall = SC_GetMV;                            /*Assigning the required datea in the packet*/
	packetToSend.index1 = mvID;                                 
	send("GETMV",packetToSend);                                 /*calls the send function which will perform postoffice->send*/
	serverPacket receivedPacket = receiveAckPacket("GETMV");    /*Receives the index and stores it in a variable*/
	if(receivedPacket.status == true)                           
		return receivedPacket.value;
	printf("\nSETMV : ERROR GETTING MV");
	interrupt->Halt();
}

void DestroyMV_Syscall(int mvID){
	clientPacket packetToSend;									/*Creating a packet in the client side*/
	packetToSend.syscall = SC_DestroyMV;                        /*Assigning the required datea in the packet*/
	packetToSend.index1 = mvID;                                 
	send("DESTROYMV",packetToSend);                             /*calls the send function which will perform postoffice->send*/
	mvID=receive("DESTROYMV");                                  /*Receives the index and stores it in a variable*/
	
	if(mvID==-1){
		printf("\nDESTROYMV : ERROR DESTROYING MV");
		interrupt->Halt();
	}
	else 
		printf("\nMV IS DESTROYED");
		return;
}

#endif

//Print Syscall - With no arguments


void Print_Syscall(int vaddr)
{	
	char *buf;
	buf=new char[MAX_LENGTH];
	buf[MAX_LENGTH]='\0';
	if(copyin(vaddr,MAX_LENGTH,buf)==-1)
	{
		printf("\n Invalid copyin");
		interrupt->Halt();
	}
	printf("\n%s",buf);
	delete buf;

}

//Print Syscall - With one argument

void Print1_Syscall(int vaddr, int arg1)
{	
	char *buf;
	buf=new char[MAX_LENGTH];
	buf[MAX_LENGTH]='\0';
	copyin(vaddr,MAX_LENGTH,buf);
	printf(buf,arg1);
	delete buf;

}

//Print Syscall - With two arguments

void Print2_Syscall(int vaddr, int arg1 ,int arg2)
{
	char *buf;
	buf=new char[MAX_LENGTH];
	buf[MAX_LENGTH]='\0';
	copyin(vaddr,MAX_LENGTH,buf);
	printf(buf,arg1,arg2);
	delete buf;

}

//Print Syscall - With three arguments

void Print3_Syscall(int vaddr, int arg1 ,int arg2, int arg3)
{
	
	char *buf;
	buf=new char[MAX_LENGTH];
	buf[MAX_LENGTH]='\0';
	copyin(vaddr,MAX_LENGTH,buf);
	printf(buf,arg1,arg2,arg3);
	delete buf;
}

void PrintTest_Syscall(int vaddr)
{
	
	char *buf;
	buf=new char[MAX_LENGTH];
	buf[MAX_LENGTH]='\0';
	copyin(vaddr,MAX_LENGTH,buf);
	printf("%s",buf);
	delete buf;
}

int scan() {
	int x;
	scanf("%d",&x);
	return x;
}

int RandomSearch() {
	return rand();
}

void Flag_Syscall(){
	printLock->Acquire();
	flag=1;
	printLock->Release();
}


void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2); // Which syscall?
    int rv=0; 	// the return value from a syscall

    if ( which == SyscallException ) {
	switch (type) {
	    default:
			DEBUG('a', "Unknown syscall - shutting down.\n");
	    
		case SC_Halt:
			DEBUG('a', "Shutdown, initiated by user program.\n");
			interrupt->Halt();
			break;
	    
		case SC_Create:
			DEBUG('a', "Create syscall.\n");
			Create_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;
	    
		case SC_Open:
			DEBUG('a', "Open syscall.\n");
			rv = Open_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;
	    
		case SC_Write:
			DEBUG('a', "Write syscall.\n");
			Write_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
			break;
	    
		case SC_Read:
			DEBUG('a', "Read syscall.\n");
			rv = Read_Syscall(machine->ReadRegister(4),
			      machine->ReadRegister(5),
			      machine->ReadRegister(6));
			break;
	    
		case SC_Close:
			DEBUG('a', "Close syscall.\n");
			Close_Syscall(machine->ReadRegister(4));
			break;
		
		case SC_Yield:
			DEBUG('a', "Yield syscall.\n");
			Yield_Syscall();
			break;
			
		case SC_Print:
			DEBUG('a', "Delete Lock syscall.\n");
			Print_Syscall(machine->ReadRegister(4));
			break;
			
		case SC_Print1:
			DEBUG('a', "Print1 Syscall.\n");
			Print1_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;
		
		case SC_Print2:
			DEBUG('a', "Print2 Syscall.\n");
			Print2_Syscall(machine->ReadRegister(4),machine->ReadRegister(5),machine->ReadRegister(6));
			break;
		
		case SC_Print3:
			DEBUG('a', "Print3 Syscall.\n");
			Print3_Syscall(machine->ReadRegister(4),machine->ReadRegister(5),machine->ReadRegister(6),machine->ReadRegister(7));
			break;
			
		case SC_CreateLock:
			DEBUG('a', "Create Lock syscall.\n");
			rv=CreateLock_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
			break;
			
		case SC_AcquireLock:
			DEBUG('a', "Acquire Lock syscall.\n");
			AcquireLock_Syscall(machine->ReadRegister(4));
			break;
			
		case SC_ReleaseLock:
			DEBUG('a', "Release Lock syscall.\n");
			ReleaseLock_Syscall(machine->ReadRegister(4));
			break;
			
		case SC_DestroyLock:
			DEBUG('a', "Destroy Lock syscall.\n");
			DestroyLock_Syscall(machine->ReadRegister(4));
			break;
			
		case SC_CreateCV:
			DEBUG('a', "Create CV syscall.\n");
			CreateCV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
			break;
			
		case SC_SignalCV:
			DEBUG('a', "Signal CV syscall.\n");
			SignalCV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
			break;
			
		case SC_WaitCV:
			DEBUG('a', "\nWait CV syscall");
			WaitCV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
			break;
			
				
		case SC_BroadcastCV:
			DEBUG('a', "\nBroadcast syscall");
			BroadcastCV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
			break;
			
		case SC_DestroyCV:
			DEBUG('a', "\nDestroy CV syscall");
			DestroyCV_Syscall(machine->ReadRegister(4));
			break;
		
		case SC_Exec:
			DEBUG('a', "\nExec syscall");
			Exec_Syscall(machine->ReadRegister(4), machine->ReadRegister(5));
			break;
			
		case SC_Fork:
			DEBUG('a', "\nFork syscall");
			Fork_Syscall(machine->ReadRegister(4));
			break;
			
		case SC_Exit:
			DEBUG('a', "\nExit syscall");
			Exit_Syscall(machine->ReadRegister(4));
			break;
			
		case SC_Scan:
			DEBUG('a', "Scan integer syscall\n");
			rv = scan();
			break;
			
		case SC_Rand:
			DEBUG('a', "Scan integer syscall\n");
			rv = RandomSearch();
			break;
			
		case SC_PrintTest:
			DEBUG('a', "Print Test.\n");
			PrintTest_Syscall(machine->ReadRegister(4));
			break;
			
		case SC_Flag:
			DEBUG('a', "Flagging print statements syscall\n");
			Flag_Syscall();
			break;
			
		#ifdef NETWORK
			
		case SC_CreateMV:
			DEBUG('a', "Create MV syscall.\n");
			rv=CreateMV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5),machine->ReadRegister(6));
			break;	
			
		case SC_SetMV:
			DEBUG('a', "Set MV syscall.\n");
			SetMV_Syscall(machine->ReadRegister(4),machine->ReadRegister(5));
			break;
		
		case SC_GetMV:
			DEBUG('a', "Get MV syscall.\n");
			rv = GetMV_Syscall(machine->ReadRegister(4));
			break;	
		
		case SC_DestroyMV:
			DEBUG('a', "Destroy MV syscall.\n");
			DestroyMV_Syscall(machine->ReadRegister(4));
			break;
		
		#endif
	}

	// Put in the return value and increment the PC
	machine->WriteRegister(2,rv);
	machine->WriteRegister(PrevPCReg,machine->ReadRegister(PCReg));
	machine->WriteRegister(PCReg,machine->ReadRegister(NextPCReg));
	machine->WriteRegister(NextPCReg,machine->ReadRegister(PCReg)+4);
	return;
    } 
	else if ( which == PageFaultException){
		currentThread->space->handlePageFault();
	}
	else {
      cout<<"Unexpected user mode exception - which:"<<which<<"  type:"<< type<<endl;
      interrupt->Halt();
    }
}

