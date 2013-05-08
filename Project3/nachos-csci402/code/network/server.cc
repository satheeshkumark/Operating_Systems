#include <map>
#include <sstream>
#include <string.h>

#include "system.h"
#include "network.h"
#include "server.h"
#include "post.h"
#include "interrupt.h"


using std::map;
using std::string;

/* Map declarations */

map<string, int > LockMap;
map<string, int > CVMap;
map<string, int > MVMap;
map<string, int >::iterator i;

/* declarations of the map searching and insertion functions */ 

int searchMap(char type, char name[MAX_CHAR]);
int insertMap(char type, char name[MAX_CHAR]);

/* global variables to track assignment of newly created locks,cvs and mvs */

int nextLockID=0;
int nextCVID=0;
int nextMVID=0;

/* Server structure variables for locks,cvs and mvs */

struct serverLockStruct serverLock[MAX_LOCK];
struct serverCVStruct serverCV[MAX_CV];
struct serverMVStruct serverMV[MAX_MV];

/*
Send function 
	- create the server side packet that is to be sent to the client
	- assign the data part to the data that need to be sent
	- create packet and mail headers and assign the values such as client machine id and mailbox id as required
	- send the server side packet to the client using postOffice->Send() method
*/
	

void send(char *type,bool status,int ID,int machineID,int mailBoxID){
	serverPacket packetToSend;
	PacketHeader pFromSToC;
	MailHeader mFromSToC;
	
	/* assigning values and sets status to the data packet to be sent to client */
	
	packetToSend.value=ID;
	packetToSend.status=status;
	
	int len = sizeof(packetToSend);
	char *data = new char[len+1];
	data[len]='\0';
	memcpy((void *)data,(void *)&packetToSend,len);
	
	/* assigning values to the packet header and the mail headers*/
	
	pFromSToC.to = machineID;
	mFromSToC.to = mailBoxID;
	mFromSToC.from=0;
	mFromSToC.length=len+1;
	
	/* Sending message using postOffice Send method */
	
	if(!(postOffice->Send(pFromSToC,mFromSToC,data))){
		printf("\nCOULDNT SEND THE DATA");
		interrupt->Halt();
	}
}

/*
searchMap() function
	- gets the type of the entity(lock or cv or mv) and the name of the entity as the input 
	- searches the map with the name 
	- if found returns the corresponding value which is usually the id of the entity
	- if not found in the map, returns -1
*/	

int searchMap(char type,char *name){

	/*gets the type of the entity(lock or cv or mv) and the name of the entity as the input */

	if(name==NULL)
		return -1;
		
	int result;
	
	switch(type){
		case 'l':
			i = LockMap.find(name);
			/*	searches the map with the lock name */ 
			if(i!=LockMap.end()){
				result = i->second;
				/* sets the return value to be lockid */
			}
			else{
				result = -1;
				/* name not found -- sets the return value to be -1 */
			}
			break;
		
		case 'c':
			i = CVMap.find(name);
			/*	searches the map with the cv name */ 
			if(i!=CVMap.end()){
				result = i->second;
				/* sets the return value to be cvid */
			}
			else{
				result = -1;
				/* name not found -- sets the return value to be -1 */
			}
			break;
			
		case 'm':
			i = MVMap.find(name);
			/*	searches the map with the mv name */ 
			if(i!=MVMap.end()){
				result = i->second;
				/* sets the return value to be mvid */				
			}
			else{
				result = -1;
				/* name not found -- sets the return value to be -1 */	
			}
			break;
		default:
			printf("\nInvalid entity type");
			result =  -1;
			break;
	}
	return result;
}

/*
insertMap() function
	- gets the type of the entity(lock or cv or mv) and the name of the entity as the input 
	- create a new unique id of the entity, enter it in the map and returns the id
*/	

int insertMap(char type,char *name){
	/* gets the type of the entity(lock or cv or mv) and the name of the entity as the input */
	if(name==NULL){
		return -1;
	}
	int result;
	switch(type){
		case 'l':
			if(nextLockID>=MAX_LOCK){ /* checks the limit of the maximum allowable lock ids */
				printf("\nSERVER : MAXIMUM LOCK LIMIT REACHED");
				result = -1;
			}
			else{
				/* create anew lock id and enter it in the map */
				LockMap[name] = nextLockID;
				result =  nextLockID++;
			}
			break;
		
		case 'c':
			printf("\nSERVER : MAXIMUM CV LIMIT REACHED");
			if(nextCVID>=MAX_CV){
				result = -1;
			}
			else{
				/* create a new cv id and enter it in the map */
				CVMap[name] = nextCVID;
				result =  nextCVID++;
			}
			break;
		case 'm':
			if(nextMVID>=MAX_MV){
				printf("\nSERVER : MAXIMUM MV LIMIT REACHED");
				result = -1;
			}
			else{
				/* create a new mv id and enter it in the map */
				MVMap[name] = nextMVID;
				result = nextMVID++;
			}
			break;				
		default:
			printf("\nInvalid entity type");
			break;
	}
	return result;
	
}

/*
createMV() functions
	- gets the following details as input:
		- unique name of the mv
		- client machine id and mailbox ids which has requested
		- initial values to be set to the monitor variables
	- searches the map to find whether the name already has an entry in the map
	- if found, reply the client with the exiting mvid
	- if not found, create a new mvid,insert in the map and send the new mvid
*/

void createMV(char *name,int initialValue,int machineID,int mailBoxID){
	
	/*searches the map to find whether the name already has an entry in the map*/
	
	int mvID = searchMap('m',name);
	
	
	if(mvID != -1){
		/*if the mvid is found in the map, reply the client with the exiting mvid*/
		send("CREATEMV",true,mvID,machineID,mailBoxID);
		return;
	}
	
	/* if the mvid is not found int the map, create a new mvid,insert in the map and send the new mvid */
	mvID = insertMap('m',name);
	
	if(mvID == -1){
		send("CREATEMV",false,mvID,machineID,mailBoxID);
		return;
	}
	
	/* update the Server mv table */
	
	serverMV[mvID].name = new char[sizeof(name)+1];
	strcpy(serverMV[mvID].name,name);
	serverMV[mvID].mvID = mvID;
	serverMV[mvID].value = initialValue;
	serverMV[mvID].count = 0;
	serverMV[mvID].valid = true;
	
	printf("\n Creating new MV%d with the initial value %d",serverMV[mvID].mvID,initialValue);
	
	send("CREATEMV",true,mvID,machineID,mailBoxID);
}

/*
setMV() function
	- gets the following values as input
		- id of the monitor variable for which the value is to be set
		- value of the monitor varable that is needed to be set
		- machine id of the requested client
		- mail box id of the requested client
	- checks the validity of the monitor variable
	- if valid, sets the value of the mvid to the value the function has received and returns server packet with status set to true
	- else returns server packet with status set to false
	
*/
void setMV(int mvID,int value,int machineID,int mailBoxID){
	
	if(mvID<0){
		printf("\nSETMV : NEGATIVE MV INDEX");
		send("SETMV",false,1,machineID,mailBoxID);
		return;
	}
	if(mvID>MAX_MV){
		printf("\nSETMV : MV INDEX OUT OF RANGE");
		send("SETMV",false,1,machineID,mailBoxID);
		return;
	}
	
	/*checks the validity of the monitor variable whether it is alive or not*/
	
	if(serverMV[mvID].valid == false){
		printf("\nTHE MONITOR %d IS NOT VALID",serverMV[mvID].mvID);
		/*send server side packet to the client with the status set to false*/	
		send("SETMV",false,1,machineID,mailBoxID);
		return;
	}
	
	/* sets the value of the monitor variable by updating the server mv structure */
	
	serverMV[mvID].value = value;
	printf("\nSETTING THE VALUE OF THE MV%d to %d",serverMV[mvID].mvID,value);
	
	/*send server side packet to the client with the status set to true*/	
	send("SETMV",true,serverMV[mvID].value,machineID,mailBoxID);
	return;
}

/*
getMV() function
	- gets the following values as input
		- id of the monitor variable for which the value is to be set
		- machine id of the requested client
		- mail box id of the requested client
	- checks the validity of the monitor variable
	- if valid, send the server packet to the client with the status set to true and the value of the mv retrieved from the servermv table
	- else returns server packet with status set to false	
*/

void getMV(int mvID,int machineID,int mailBoxID){
	
	if(mvID<0){
		printf("\nSETMV : NEGATIVE MV INDEX");
		send("SETMV",false,1,machineID,mailBoxID);
		return;
	}
	if(mvID>MAX_MV){
		printf("\nSETMV : MV INDEX OUT OF RANGE");
		send("SETMV",false,1,machineID,mailBoxID);
		return;
	}
	
	/* checks the validity of the monitor variable */
	
	if(serverMV[mvID].valid == false){
		printf("\nTHE MONITOR %d IS NOT VALID",serverMV[mvID].mvID);
		/* if mvid is not valid,returns server packet with status set to false	*/
		send("SETMV",false,1,machineID,mailBoxID);
		return;
	}
	
	printf("\nGETTING THE VALUE OF THE MONITOR VARIABLE %d",serverMV[mvID].mvID);
	
	/*if valid, send the server packet to the client with the status set to true and the value of the mv retrieved from the servermv table*/
	send("GETMV",true,serverMV[mvID].value,machineID,mailBoxID);
	return;
}

/*
createLock() functions
	- gets the following details as input:
		- unique name of the lock
		- client machine id and mailbox ids which has requested
	- searches the map to find whether the name already has an entry in the map
	- if found, reply the client with the exiting lockid
	- if not found, create a new lockid,insert in the map and send the new mvid
*/


void createLock(char *name,int machineID,int mailBoxID){
	int lockID = searchMap('l',name);
	
	if(lockID != -1){				/*Halts the program if the index received is -1*/
		send("CREATELOCK",true,lockID,machineID,mailBoxID);
		return;
	}
	
	lockID = insertMap('l',name);
	
	if(lockID == -1){				/*Halts the program if the index received is -1*/
		send("CREATELOCK",false,lockID,machineID,mailBoxID);
		return;
	}
	
	printf("\n Creating new lock, ID: %d",lockID);
	serverLock[lockID].name = new char[sizeof(name)+1];
	strcpy(serverLock[lockID].name,name);	/*Updating the serverlock structure*/		
	serverLock[lockID].lockID = lockID;
	serverLock[lockID].count = 0;
	serverLock[lockID].waitQueue = new List;
	serverLock[lockID].available = true;
	serverLock[lockID].valid = true;
	serverLock[lockID].isToBeDeleted = false;
	serverLock[lockID].isDeleted = false;
	
	send("CREATELOCK",true,lockID,machineID,mailBoxID);
}

/*

acquireLock()
	- gets the following values as input
		- lock index
		- machine id of the client
		- mailbox id of the client
	- checks for the validity of the lockid
	- if the lockid is invalid, send reply packet to the client with status set to false
	- if the lockid is valid and available, 
		- set the available flag to false
		- set the details of the owner
	- if the lockid is valid and not available, make the client to waait in wait queue 
*/

void acquireLock(int index,int machineID,int mailBoxID){
	serverPacket packetToSend;
	client *waitingClient = NULL;
	
	if(index<0 || index > MAX_LOCK){								/*Checks if the lock index is valid or not*/
		printf("\nLOCK ACQUIRE : INVALID LOCK INDEX");
		send("ACQUIRELOCK", false,-1, machineID, mailBoxID);
		return;
	} 
			
	if(serverLock[index].valid==false){								/*Checks if the lock we are trying to acquire is valid*/
		printf("\nLOCK ACQUIRE : LOCK YOU ARE TRYING TO ACQUIRE IS INVALID");
		send("ACQUIRELOCK", false,2, machineID, mailBoxID);
		return;
	}
																	/*Check if the client who is trying to acquire the lock already has the lock*/
	if(serverLock[index].owner.machineID==machineID && serverLock[index].owner.mailBoxID==mailBoxID){
		printf("\nLOCK ACQUIRE : CLIENT(%d,%d) ALREADY HAVE THE LOCK",machineID,mailBoxID);
		send("ACQUIRELOCK", false,3, machineID, mailBoxID);
		return;
	}
	
	serverLock[index].count++;
	
	if(serverLock[index].available==true){							/*Client is made the lock owner*/
		serverLock[index].available=false;
		serverLock[index].owner.machineID=machineID;
		serverLock[index].owner.mailBoxID=mailBoxID;
		printf("\nLOCK ACQUIRE : CLIENT(%d,%d) MADE THE LOCK OWNER",machineID,mailBoxID);		
		send("ACQUIRELOCK", true,index, machineID, mailBoxID);
	}
	else{															/*Clien is added to the wait queue*/
		waitingClient = new client;
		waitingClient->machineID=machineID;
		waitingClient->mailBoxID=mailBoxID;
		serverLock[index].waitQueue->Append((void *)waitingClient);
		printf("\nLOCK ACQUIRE : LOCK ISNT FREE,CLIENT(%d,%d) APPENDED TO LOCK WAITQUEUE",machineID,mailBoxID);
	}
	
}

/*
releaseLock()
	- gets the following values as input
		- lock index
		- machine id of the client
		- mailbox id of the client
	- checks for the validity of the lockid
	- if the lockid is invalid, send reply packet to the client with status set to false
	- if the lockid is valid and the requested client is the lock owner, 
		- set the available flag to true
		- set the lock owner machine and mail box id to be -1
	- if the lockid is valid and the requested client is not the lock owner, reply with status set to false 
*/

void releaseLock(int index,int machineID,int mailBoxID,int syscallType){
	serverPacket packetToSend;
	
	if(index<0 || index > MAX_LOCK){								/*Checks if the lock index is valid or not*/
		printf("\nLOCK RELEASE : INVALID LOCK INDEX");
		send("RELEASELOCK", false,-1, machineID, mailBoxID);
		return;
	}
	
	if(serverLock[index].valid==false){								/*Checks if the lock we are trying to acquire is valid*/
		printf("\nLOCK RELEASE : LOCK YOU ARE TRYING TO ACQUIRE IS INVALID");
		send("RELEASELOCK", false,2, machineID, mailBoxID);
		return;
	}
																	/*Cleint released the lock*/
	if(serverLock[index].owner.machineID==machineID && serverLock[index].owner.mailBoxID==mailBoxID){
		printf("\nLOCK RELEASE : CLIENT(%d,%d) RELEASED THE LOCK, COUNT : %d",machineID,mailBoxID,serverLock[index].count);
		if(syscallType==SC_ReleaseLock)
		send("RELEASELOCK", true,index, machineID, mailBoxID);
		if(serverLock[index].waitQueue->IsEmpty()){
			serverLock[index].available = true;
			serverLock[index].owner.machineID=-1;
			serverLock[index].owner.mailBoxID=-1;
		}
		else{														/*Client is made the lock owner*/
			serverLock[index].owner = *((client*)serverLock[index].waitQueue->Remove());
			printf("\nLOCK RELEASE : CLIENT(%d,%d) IS NOW MADE THE LOCK OWNER",serverLock[index].owner.machineID,serverLock[index].owner.mailBoxID);
			send("RELEASELOCK", true,index, serverLock[index].owner.machineID, serverLock[index].owner.mailBoxID);
		}	
		serverLock[index].count--;	
	}	
	else{	
		printf("\nLOCK RELEASE : CLIENT(%d,%d) IS NOT THE LOCK OWNER",machineID,mailBoxID);
		send("RELEASELOCK", false,2, machineID, mailBoxID);	
	}	
		
	if(serverLock[index].isToBeDeleted && serverLock[index].count==0){
		delete serverLock[index].name;	
		delete serverLock[index].waitQueue;	
		serverLock[index].valid = false;	
	}		
}

/*
destroyLock()
	- gets the following values as input
		- lock index
		- machine id of the client
		- mailbox id of the client
	- checks for the validity of the lockid
	- if the lockid is invalid, send reply packet to the client with status set to false
	- if the lockid is valid, 
		- check for the count of the users waiting for the lock
		- if the count is 0,
			- set the tobedeleted flag to true,delete the lock, set the valid bit to false and set the isdeleted flag to true
		- if it is not zero, 
			- set the tobedeleted to be true and send reply packet with status set to false
	- if the lockid is valid and the requested client is not the lock owner, reply with status set to false 
*/

void destroyLock(int index,int machineID,int mailBoxID){
	
	
	if(index < 0 || index > MAX_LOCK){								/*Checks if the lock index is valid or not*/
		printf("\nINVALID LOCK NUMBER");
		send("DESTROYLOCK", false,1, machineID, mailBoxID);
		return;
	}
	
	if(serverLock[index].valid == 0){								/*Checks if the lock that the client is trying to delete has been deleted already*/
			printf("\nTHE LOCK YOU ARE TRYING TO DELETE IS INVALID");
	}
	
	serverLock[index].isToBeDeleted = true;
	
	if(serverLock[index].count== 0)									/*Destroying the lock*/
	{
		delete []serverLock[index].name;
		delete []serverLock[index].waitQueue;
		serverLock[index].valid = 0;	
		printf("\nLOCK IS DESTRYOED");
		send("DESTROYLOCK",true,index,machineID,mailBoxID);
	}
	else{															/*Showing the error message*/
		printf("\nDESTROY LOCK : LOCK CANNOT BE DESTROYED");
		send("DESTROYLOCK", false,1, machineID, mailBoxID);
	}	
}

/*
createCV() functions
	- gets the following details as input:
		- unique name of the cv
		- client machine id and mailbox ids which has requested
	- searches the map to find whether the name already has an entry in the map
	- if found, reply the client with the exiting cvid
	- if not found, create a new cvid,insert in the map and send the new cvid
*/

void createCV(char *name,int machineID,int mailBoxID){
	int cvID = searchMap('c',name);
	
	if(cvID != -1){
		send("CREATECV",true,cvID,machineID,mailBoxID);
		return;
	}
	
	cvID = insertMap('c',name);
	
	if(cvID == -1){
		send("CREATECV",false,cvID,machineID,mailBoxID);
		return;
	}
	
	printf("\nCreating new cv, ID: %d",cvID);
	
	serverCV[cvID].name = new char[sizeof(name)+1];
	strcpy(serverCV[cvID].name,name);
	serverCV[cvID].cvID = cvID;
	serverCV[cvID].count = 0;
	serverCV[cvID].waitQueue = new List;
	serverCV[cvID].available = true;
	serverCV[cvID].valid = true;
	serverCV[cvID].isToBeDeleted = false;
	serverCV[cvID].isDeleted = false;
	
	send("CREATECV",true,cvID,machineID,mailBoxID);
}


/*
createCV() function
	- gets the following details as input:
		- cv index where the client should wait
		- lock index which is used for waiting in the cv
		- client machine id and mailbox ids which has requested
	- check for the validity of the lock
		- if invalid,send reply with status set to false
		- if valid, check for the validity of the cv
	- check for the validity of the cv
		- check whether cv has lockid as -1
			- if true, assign the input lockid to the cv 
			- if not check whether the locked id assigned to cv and the input lockids are equal
			- if not equal set status to false and send reply message
	- realease the lock and add the client to the waiting client queue
*/

void waitCV(int index1,int index2,int machineID, int mailBoxID) {
	
	serverPacket packetToSend;
	client *waitingClient = NULL;
	
	/*check for the validity of the lock*/
	
	if(index1<0 || index1 > MAX_LOCK){
		printf("\nWAITCV: INVALID LOCK INDEX");
		send("WAITCV", false,1, machineID, mailBoxID);
		return;
	} 
	
	if(index2<0 || index2 > MAX_CV){
		printf("\nWAITCV: INVALID CV INDEX");
		send("WAITCV", false,1, machineID, mailBoxID);
		return;
	} 

	/*check for the validity of the cv*/
	
	if(serverLock[index1].valid==false && serverCV[index2].valid==false ){
		printf("\nLOCK%d WITH CV %d IS INVALID",index1,index2);
		send("WAITCV", false,1, machineID, mailBoxID);
		return;
	}
	
	if(serverCV[index2].lockID == -1) {
		serverCV[index2].lockID = index1;
	}
		
	if(serverCV[index2].cvID  != index1) {
		printf(" \nWAITCV: LOCK(%d) DOESN'T MATCH FOR CV(%d)", index1, index2);
		send("WAITCV", false,1, machineID, mailBoxID);
		return;
	}
	
	/* release the lock */
	releaseLock(index1,machineID, mailBoxID, 1);
	
	/* add the client to the waiting client queue*/
	
	serverCV[index2].count++;	
	waitingClient = new client;
	waitingClient->machineID=machineID;
	waitingClient->mailBoxID=mailBoxID;
	serverCV[index2].waitQueue->Append((void *)waitingClient);
}


/*
signalCV() function
	- gets the following details as input:
		- cv index where the client should wait
		- lock index which is used for waiting in the cv
		- client machine id and mailbox ids which has requested
	- check for the validity of the lock
		- if invalid,send reply with status set to false
		- if valid, check for the validity of the cv
	- check for the validity of the cv
		- check whether cv has lockid as -1
			- if true, assign the input lockid to the cv 
			- if not check whether the locked id assigned to cv and the input lockids are equal
			- if not equal set status to false and send reply message
	- check whether the queue has any clients waiting on it
	- If so,acquire the lock and send reply message to the client with status set to true
	- else, send the reply message to the client with status set to false
*/	
	
void signalCV(int index1,int index2,int machineID, int mailBoxID) {
	
	serverPacket packetToSend;
	client signalClient;
	
	/* check for the validity of the lock */
	
	if(index1<0 || index1 > MAX_LOCK){
		printf("\nSIGNALCV: INVALID LOCK INDEX");
		send("SIGNALCV", false,1, machineID, mailBoxID);
		return;
	} 
	
	/*check for the validity of the cv*/
	
	if(index2<0 || index2 > MAX_CV){
		printf("\nSIGNALCV: INVALID CV INDEX");
		send("SIGNALCV", false,1, machineID, mailBoxID);
		return;
	} 

	if(serverLock[index1].valid==false && serverCV[index2].valid==false ){
		printf("\nLOCK%d WITH CV %d IS INVALID",index1,index2);
		send("SIGNALCV", false,1, machineID, mailBoxID);
		return;
	}
	
	if(serverLock[index1].owner.machineID!=machineID || serverLock[index1].owner.mailBoxID!=mailBoxID){
		printf("\nSIGNALCV : CLIENT(%d,%d) DOESNT HAVE THE LOCK",machineID,mailBoxID);
		send("SIGNALCV", false,1, machineID, mailBoxID);
	}
	
	if(serverCV[index2].lockID  != index1) {
		printf(" \nSIGNALCV: LOCK(%d) DOESN'T MATCH FOR CV(%d)", index1, index2);
		send("SIGNALCV", false,1, machineID, mailBoxID);
		return;
	}
	
	/*check whether the queue has any clients waiting on it*/
	
	if(serverCV[index2].waitQueue->IsEmpty()) {
		
		printf("\nQUEUE IS EMPTY");
		send("SIGNALCV", false,1, machineID, mailBoxID);
		return;
	}
	
	signalClient = *((client*)serverCV[index2].waitQueue->Remove());
	if(serverCV[index2].waitQueue->IsEmpty()) {
		serverCV[index2].lockID = -1;
	}
	
	serverCV[index2].count--;
	//printf("\nSIGNAL : WAITING CLIENT(%d,%d) IS WOKEN UP",signalClient.machineID, signalClient.mailBoxID);
	// Acquire lock - Acquire Lock will signal to the process, once lock has been acquired.
	
	/*acquire the lock and send reply message to the client with status set to true*/
	
	acquireLock(index1, signalClient.machineID, signalClient.mailBoxID);
	
	send( "SIGNALCV", true, index2, machineID, mailBoxID);	
}

/*
broadcastCV() function
	- gets the following details as input:
		- cv index where the client should wait
		- lock index which is used for waiting in the cv
		- client machine id and mailbox ids which has requested
	- check for the validity of the lock
		- if invalid,send reply with status set to false
		- if valid, check for the validity of the cv
	- check for the validity of the cv
		- check whether cv has lockid as -1
			- if true, assign the input lockid to the cv 
			- if not check whether the locked id assigned to cv and the input lockids are equal
			- if not equal set status to false and send reply message
	- check whether the queue has any clients waiting on it
	- If so,signal to every client waiting in the cv
	- else, send the reply message to the client with status set to false
*/

void broadcastCV(int index1,int index2,int machineID, int mailBoxID) {
	
	serverPacket packetToSend;
	client signalClient;
	
	/*check for the validity of the lock */
	
	if(index1<0 || index1 > MAX_LOCK){
		printf("\nBROADCASTCV: INVALID LOCK INDEX");
		send("BROADCASTCV", false,1, machineID, mailBoxID);
		return;
	} 
	
	if(index2<0 || index2 > MAX_CV){
		printf("\nBROADCASTCV: INVALID CV INDEX");
		send("BROADCASTCV", false,1, machineID, mailBoxID);
		return;
	} 
	
	/*check for the validity of the cv */

	if(serverLock[index1].valid==false && serverCV[index2].valid==false ){
		printf("\nLOCK%d WITH CV %d IS INVALID",index1,index2);
		send("BROADCASTCV", false,1, machineID, mailBoxID);
		return;
	}
	
	/* If the cv has some clients,signal to every client waiting in the cv */
	
	while(!serverCV[index2].waitQueue->IsEmpty()) {
		signalCV(index1,index2,machineID,mailBoxID);
	}
		
		send( "BROADCASTCV", true, index2, machineID, mailBoxID);	
}

/*
destroyCV()
	- gets the following values as input
		- CV index
		- machine id of the client
		- mailbox id of the client
	- checks for the validity of the cvid
	- if the cvid is invalid, send reply packet to the client with status set to false
	- if the cvid is valid, 
		- check for the count of the users waiting in the cv
		- if the count is 0,
			- set the tobedeleted flag to true,delete the lock, set the valid bit to false and set the isdeleted flag to true
		- if it is not zero, 
			- set the tobedeleted to be true and send reply packet with status set to false
*/

void destroyCV(int index,int machineID,int mailBoxID){
	
	
	if(index < 0 || index > MAX_CV){
		printf("\nINVALID CV NUMBER");
		send("DESTROYCV", false,1, machineID, mailBoxID);
		return;
	}
	
	/* checks for the validity of the cvid */
	if(serverCV[index].valid == 0){
			printf("\nTHE CV YOU ARE TRYING TO DELETE IS INVALID");
			/*  if the mvid is invalid, send reply packet to the client with status set to false */
			send("DESTROYCV", false,1, machineID, mailBoxID);
			return;
	}
	
	serverCV[index].isToBeDeleted = true;	
	
	
	if(serverCV[index].count== 0)
	{
		/* No clients waiting on cv-- delete the cv entries in the cv table */
		delete []serverCV[index].name;
		delete []serverCV[index].waitQueue;
		serverCV[index].valid = 0;	
		printf("\nCV IS DESTROYED");
		send("DESTROYCV",true,index,machineID,mailBoxID);
	}
	else{
		/* Some clients still waiting on cv-- send the packet with status set to false */
		printf("\nDESTROY CV : CV CANNOT BE DESTROYED");
		send("DESTROYCV", false,1, machineID, mailBoxID);
	}
		
}

/*
destroyMV()
	- gets the following values as input
		- mv index
		- machine id of the client
		- mailbox id of the client
	- checks for the validity of the mvid
	- if the mvid is invalid, send reply packet to the client with status set to false
	- if the mvid is valid, 
		- set the tobedeleted flag to true,delete the lock, set the valid bit to false and set the isdeleted flag to true
*/

void destroyMV(int index,int machineID,int mailBoxID){
	
	
	if(index < 0 || index > MAX_MV){
		printf("\nINVALID MV NUMBER");
		send("DESTROYMV", false,1, machineID, mailBoxID);
		return;
	}
	
	/* checks for the validity of the mvid */
	
	if(serverMV[index].valid == 0){
			printf("\nTHE MV YOU ARE TRYING TO DELETE IS INVALID");
	}
	
	if(serverMV[index].count== 0)
	{
		/*if the mvid is valid, update the mv entry in the ms structure table and send reply packet to the client with status set to true */
		delete []serverMV[index].name;
		serverMV[index].valid = 0;	
		printf("\nMV IS DESTROYED");
		send("DESTROYMV",true,index,machineID,mailBoxID);
	}
	else{
		printf("\nDESTROY MV : MV CANNOT BE DESTROYED");
		/* if the mvid is invalid, send reply packet to the client with status set to false */
		send("DESTROYMV", false,1, machineID, mailBoxID);
	}
		
}

/* 
Server()
	- gets the client request packet
	- decodes the client packet and handles the requested system call
	- redirects the request to particular system call functions and replies to the requested client
*/

void Server(){
	PacketHeader pFromCToS;
	MailHeader mFromCToS;
	clientPacket packetReceived;
	
	int len=sizeof(packetReceived);
	char *data=new char[len+1];
	data[len]='\0';
	
	while(1){
	
		/* server listens to client request infinitely */
	
		printf("\nSERVER : WAITING FOR CLIENT REQUEST");
		
		/* receives request from the client and copies it into character array in memory */
		
		postOffice->Receive(0,&pFromCToS,&mFromCToS,data);
		memcpy((void *)&packetReceived,(void *)data,len);
		
		//printf("\nSERVER : RECEIVED CLIENT REQUEST  : %s",packetReceived.name1);
		
		/* Based on the client request, performs that particular set of action */
		
		switch(packetReceived.syscall){
			case SC_CreateLock:
				printf("\nREQUEST :  CREATE LOCK FROM CLIENT(%d,%d)",pFromCToS.from,mFromCToS.from);
				createLock(packetReceived.name1,pFromCToS.from,mFromCToS.from);
				break;
			
			case SC_AcquireLock:
				printf("\nREQUEST :  ACQUIRE LOCK FROM CLIENT(%d,%d)",pFromCToS.from,mFromCToS.from);
				acquireLock(packetReceived.index1,pFromCToS.from,mFromCToS.from);
				break;
				
			case SC_ReleaseLock:
				printf("\nREQUEST :  RELEASE LOCK FROM CLIENT(%d,%d)",pFromCToS.from,mFromCToS.from);
				releaseLock(packetReceived.index1,pFromCToS.from,mFromCToS.from,SC_ReleaseLock);
				break;
			
			case SC_DestroyLock:
				printf("\nREQUEST :  DESTROYLOCK FROM CLIENT(%d,%d)",pFromCToS.from,mFromCToS.from);
				destroyLock(packetReceived.index1,pFromCToS.from,mFromCToS.from);
				break;	
				
			case SC_CreateCV:
				printf("\nREQUEST :  CREATECV FROM CLIENT(%d,%d)",pFromCToS.from,mFromCToS.from);
				createCV(packetReceived.name1,pFromCToS.from,mFromCToS.from);
				break;

			case SC_WaitCV:
				printf("\nREQUEST :  WAIT FROM CLIENT(%d,%d)",pFromCToS.from,mFromCToS.from);
				waitCV(packetReceived.index1,packetReceived.index2,pFromCToS.from,mFromCToS.from);
				break;
			
			case SC_SignalCV:
				printf("\nREQUEST :  SIGNAL FROM CLIENT(%d,%d)",pFromCToS.from,mFromCToS.from);
				signalCV(packetReceived.index1,packetReceived.index2,pFromCToS.from,mFromCToS.from);
				break;
			
			case SC_BroadcastCV:
				printf("\nREQUEST :  BROADCAST CV FROM CLIENT(%d,%d)",pFromCToS.from,mFromCToS.from);
				broadcastCV(packetReceived.index1,packetReceived.index2,pFromCToS.from,mFromCToS.from);
				break;
			
			case SC_DestroyCV:
				printf("\nREQUEST :  DESTROY CV FROM CLIENT(%d,%d)",pFromCToS.from,mFromCToS.from);
				destroyCV(packetReceived.index1,pFromCToS.from,mFromCToS.from);
				break;
			
			case SC_CreateMV:
				printf("\nREQUEST :  Create MV FROM CLIENT(%d,%d)",pFromCToS.from,mFromCToS.from);
				createMV(packetReceived.name1,packetReceived.value,pFromCToS.from,mFromCToS.from);
				break;
				
			case SC_SetMV:
				printf("\nREQUEST :  Set MV FROM CLIENT(%d,%d)",pFromCToS.from,mFromCToS.from);
				setMV(packetReceived.index1,packetReceived.value,pFromCToS.from,mFromCToS.from);
				break;
				
			case SC_GetMV:
				printf("\nREQUEST :  Get MV FROM CLIENT(%d,%d)",pFromCToS.from,mFromCToS.from);
				getMV(packetReceived.index1,pFromCToS.from,mFromCToS.from);
				break;
				
			case SC_DestroyMV:
				printf("\nREQUEST :  Destroy MV FROM CLIENT(%d,%d)",pFromCToS.from,mFromCToS.from);
				destroyMV(packetReceived.index1,pFromCToS.from,mFromCToS.from);
				break;
				
			default:
				printf("\nSERVER : INVALID SYSCALL %d",packetReceived.syscall);
		}
	}
}

	