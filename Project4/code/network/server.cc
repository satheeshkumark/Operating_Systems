#include <map>
#include <sstream>

#include "string.h"
#include "system.h"
#include "network.h"
#include "server.h"
#include "post.h"
#include "interrupt.h"
#include "bitmap.h"


using std::map;
using std::string;
using namespace std;

/* Map declarations */

map<string, int > LockMap;
map<string, int > CVMap;
map<string, int > MVMap;
map<string, int >::iterator i;


/* global variables to track assignment of newly created locks,cvs and mvs */

int nextLockID;
int nextCVID;
int nextMVID;
int globalRequestIndex=0;

/* declarations of the map searching and insertion functions */ 

int searchMap(char type, char name[MAX_CHAR]);
int insertMap(char type, char name[MAX_CHAR]);



/* Server structure variables for locks,cvs and mvs */

struct serverLockStruct serverLock[6000];
struct serverCVStruct serverCV[6000];
struct serverMVStruct serverMV[6000];
struct queueStruct pendingQueue[6000];

BitMap *requestBitmap;
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
	memcpy((void *)data,(void *)&packetToSend,len);
	data[len]='\0';
	
	/* assigning values to the packet header and the mail headers*/
	
	pFromSToC.to = machineID;
	mFromSToC.to = mailBoxID;
	pFromSToC.from = myMachineID;
	mFromSToC.from=0;
	mFromSToC.length=len;
	
	
	/* Sending message using postOffice Send method */
	
	if(!(postOffice->Send(pFromSToC,mFromSToC,data))){
	
		printf("\nPROCESS ALREADY ENDED");
	}
	else
		printf("\n%s SEND : SENT ID %d TO CLIENT(%d,%d)",type,packetToSend.value,pFromSToC.to,mFromSToC.to);
	
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
	int iEntityId = searchMap(type, name);
	if ( iEntityId != -1 ) {
		return iEntityId;
	}
	switch(type){
		case 'l':
			if(nextLockID+1>=((myMachineID+1)*MAX_LOCK)){ /* checks the limit of the maximum allowable lock ids */
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
			if(nextCVID+1>= ((myMachineID+1)*MAX_CV)){
				printf("\nSERVER : MAXIMUM CV LIMIT REACHED");
				result = -1;
			}
			else{
				/* create a new cv id and enter it in the map */
				CVMap[name] = nextCVID;
				result =  nextCVID++;
			}
			break;
		case 'm':
			if(nextMVID+1>=((myMachineID+1)*MAX_MV)){
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
	
	printf("\n Creating new MV%d - %s with the initial value %d for Client(%d,%d)",serverMV[mvID].mvID,serverMV[mvID].name,initialValue,machineID,mailBoxID);
	
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
	if(mvID>((myMachineID+1)*MAX_MV)){
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
	printf("\nSETTING THE VALUE OF THE MV%d - %s to %d",serverMV[mvID].mvID,serverMV[mvID].name,value);
	
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
		printf("\nGETMV : NEGATIVE MV INDEX");
		send("GETMV",false,1,machineID,mailBoxID);
		return;
	}
	if(mvID>((myMachineID+1)*1000)){
		printf("\nGETMV : MV INDEX OUT OF RANGE");
		send("GETMV",false,1,machineID,mailBoxID);
		return;
	}
	
	/* checks the validity of the monitor variable */
	
	if(serverMV[mvID].valid == false){
		printf("\nTHE MONITOR %d - %s IS NOT VALID",serverMV[mvID].mvID,serverMV[mvID].name);
		/* if mvid is not valid,returns server packet with status set to false	*/
		send("GETMV",false,1,machineID,mailBoxID);
		return;
	}
	
	printf("\nGETTING THE VALUE OF THE MV%d - %s, VALUE : %d FOR CLIENT(%d,%d)",serverMV[mvID].mvID,serverMV[mvID].name,serverMV[mvID].value,machineID,mailBoxID);
	
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
		printf("\nCREATELOCK : LOCK ALREADY CREATED, RETURNING LOCKID%d - %s",lockID,name);
		send("CREATELOCK",true,lockID,machineID,mailBoxID);
		return;
	}
	
	lockID = insertMap('l',name);
	
	if(lockID == -1){				/*Halts the program if the index received is -1*/
		send("CREATELOCK",false,lockID,machineID,mailBoxID);
		return;
	}
	
	
	serverLock[lockID].name = new char[sizeof(name)+1];
	strcpy(serverLock[lockID].name,name);	/*Updating the serverlock structure*/		
	serverLock[lockID].lockID = lockID;
	serverLock[lockID].count = 0;
	serverLock[lockID].waitQueue = new List;
	serverLock[lockID].available = true;
	serverLock[lockID].valid = true;
	serverLock[lockID].isToBeDeleted = false;
	serverLock[lockID].isDeleted = false;
	
	printf("\n Creating new lock, LOCKID%d  - %s",lockID,serverLock[lockID].name);
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
	
	if(index<0 || index > ((myMachineID+1)*MAX_LOCK)){								/*Checks if the lock index is valid or not*/
		printf("\nLOCK ACQUIRE : INVALID LOCK INDEX");
		send("ACQUIRELOCK", false,index, machineID, mailBoxID);
		return;
	} 
			
	if(serverLock[index].valid==false){								/*Checks if the lock we are trying to acquire is valid*/
		printf("\nLOCK ACQUIRE : LOCKID%d YOU ARE TRYING TO ACQUIRE IS INVALID",index);
		send("ACQUIRELOCK", false,index, machineID, mailBoxID);
		return;
	}
																	/*Check if the client who is trying to acquire the lock already has the lock*/
	if(serverLock[index].owner.machineID==machineID && serverLock[index].owner.mailBoxID==mailBoxID){
		printf("\nLOCK ACQUIRE : CLIENT(%d,%d) ALREADY HAVE THE LOCKID%d - %s",machineID,mailBoxID,index,serverLock[index].name);
		send("ACQUIRELOCK", false,index, machineID, mailBoxID);
		return;
	}
	
	if(serverLock[index].available==true){							/*Client is made the lock owner*/
		serverLock[index].available=false;
		serverLock[index].owner.machineID=machineID;
		serverLock[index].owner.mailBoxID=mailBoxID;
		printf("\nLOCK ACQUIRE : CLIENT(%d,%d) MADE THE LOCKID%d - %s OWNER",serverLock[index].owner.machineID,serverLock[index].owner.mailBoxID,index,serverLock[index].name);		
		send("ACQUIRELOCK", true,index, machineID, mailBoxID);
	}
	else{															/*Clien is added to the wait queue*/\
		if(!(serverLock[index].owner.machineID==machineID && serverLock[index].owner.mailBoxID==mailBoxID)){
			waitingClient = new client;
			waitingClient->machineID=machineID;
			waitingClient->mailBoxID=mailBoxID;
			serverLock[index].waitQueue->Append((void *)waitingClient);
			printf("\nLOCK ACQUIRE : LOCKID%d - %s ISNT FREE,CLIENT(%d,%d) APPENDED TO LOCK WAITQUEUE",index,serverLock[index].name,machineID,mailBoxID);
		}
		else{
			printf("\nLOCK ACQUIRE : CLIENT(%d,%d) ALREADY HAVE THE LOCKID%d - %s",machineID,mailBoxID,index,serverLock[index].name);
			send("ACQUIRELOCK", false,index, machineID, mailBoxID);
		}
	}
	
	serverLock[index].count++;
	
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
	client newOwner;
	
	if(index<0 || index > ((myMachineID+1)*MAX_LOCK)){								/*Checks if the lock index is valid or not*/
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
		printf("\nLOCK RELEASE : CLIENT(%d,%d) RELEASED THE LOCKID%d - %s, COUNT : %d",machineID,mailBoxID,index,serverLock[index].name,serverLock[index].count);
		if(syscallType==SC_ReleaseLock)
			send("RELEASELOCK", true,index, machineID, mailBoxID);
		else
			printf("\nRELEASELOCK : LOCK RELEASED FOR CLIENT(%d,%d) WAIT REQUEST", machineID, mailBoxID);
		if(serverLock[index].waitQueue->IsEmpty()){
			if(extraCredit == 1)
			{
				//--------------------------------------EXTRACREDIT-----------------------------------------------
				printf("\nEC==>SERVER (%d) MADE THE LOCK AVAILABLE SINCE THE LOCK(%d) OWNER CLIENT(%d,%d) IS DEAD",myMachineID,index,serverLock[index].owner.machineID, serverLock[index].owner.mailBoxID);
			}			
			serverLock[index].available = true;
			serverLock[index].owner.machineID=-1;
			serverLock[index].owner.mailBoxID=-1;
		}
		else{														/*Client is made the lock owner*/
			newOwner = *((client*)serverLock[index].waitQueue->Remove());
			if(extraCredit == 1)
			{
				//--------------------------------------EXTRACREDIT-----------------------------------------------
				printf("\nEC==>SERVER (%d) MADE THE CLIENT(%d,%d) AS THE NEW LOCK OWNER SINCE THE CURRENT LOCK(%d) OWNER CLIENT(%d,%d) IS DEAD",myMachineID,newOwner.machineID,newOwner.mailBoxID,index,serverLock[index].owner.machineID, serverLock[index].owner.mailBoxID);
			}
			
			serverLock[index].owner = newOwner;
			
			printf("\nLOCK RELEASE : CLIENT(%d,%d) IS NOW MADE THE LOCKID%d - %s OWNER",serverLock[index].owner.machineID,serverLock[index].owner.mailBoxID,index,serverLock[index].name);
			send("RELEASELOCK", true,index, serverLock[index].owner.machineID, serverLock[index].owner.mailBoxID);
		}		
	}	
	else{	
		printf("\nLOCK RELEASE : CLIENT(%d,%d) IS NOT THE LOCK OWNER, CURRENT OWNER(%d,%d)",machineID,mailBoxID, serverLock[index].owner.machineID, serverLock[index].owner.mailBoxID);
		send("RELEASELOCK", false,2, machineID, mailBoxID);	
	}	
		
	serverLock[index].count--;
	
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
	
	
	if(index < 0 || index > ((myMachineID+1)*MAX_LOCK)){								/*Checks if the lock index is valid or not*/
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
		printf("\nLOCKID%d - %s IS DESTROYED",index,serverLock[index].name);
		delete []serverLock[index].name;
		delete []serverLock[index].waitQueue;
		serverLock[index].valid = 0;	
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
		printf("\nCREATECV : CV ALREADY CREATED, RETURNING CVID%d - %s",cvID,name);
		send("CREATECV",true,cvID,machineID,mailBoxID);
		return;
	}
	
	cvID = insertMap('c',name);
	
	if(cvID == -1){
		send("CREATECV",false,cvID,machineID,mailBoxID);
		return;
	}
	
	printf("\nCreating new cv, CVID%d - %s",cvID,name);
	
	serverCV[cvID].name = new char[sizeof(name)+1];
	strcpy(serverCV[cvID].name,name);
	serverCV[cvID].lockID = -1;
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
	
	clientPacket packetToSend;
	client *waitingClient = NULL;
	PacketHeader pFromSToS;
	MailHeader mFromSToS;
	
	int len = sizeof(packetToSend);
	
	char *data=new char[len+1];
	
	/*check for the validity of the lock*/
	
	/*if(index1<0 || index1 > ((myMachineID+1)*MAX_LOCK)){
		printf("\nWAITCV: INVALID LOCK INDEX : LOCKID%d",index1);
		send("WAITCV", false,1, machineID, mailBoxID);
		return;
	} */
	
	if(index2<0 || index2 > ((myMachineID+1)*MAX_CV)){
		printf("\nWAITCV: INVALID CV INDEX : CVID%d",index2);
		send("WAITCV", false,1, machineID, mailBoxID);
		return;
	} 

	/*check for the validity of the cv*/
	
	/*if(serverLock[index1].valid==false || serverCV[index2].valid==false ){
		printf("\nLOCKID%d WITH CVID%d IS INVALID",index1,index2);
		send("WAITCV", false,1, machineID, mailBoxID);
		return;
	}*/
	
	if(serverCV[index2].valid==false ){
		printf("\nLOCKID%d WITH CVID%d IS INVALID",index1,index2);
		send("WAITCV", false,1, machineID, mailBoxID);
		return;
	}
	if(serverCV[index2].lockID == -1) {
		serverCV[index2].lockID = index1;
	}
		
	if(serverCV[index2].lockID  != index1) {
		printf(" \nWAITCV: LOCKID%d -%s,CVID%d - %s DOESN'T MATCH FOR LOCKID%d - %s IN CVID%d - %s",index1,serverLock[index1].name,index2,serverCV[index2].name,serverCV[index2].lockID,serverLock[serverCV[index2].lockID].name,index2,serverCV[index2].name);
		send("WAITCV", false,1, machineID, mailBoxID);
		return;
	}
	
	/* release the lock */
	if(SERVERS == 1){
			releaseLock(index1,machineID, mailBoxID, 1);
	}
	else{
		packetToSend.value = SC_WaitCV;
		packetToSend.index1 = index1;
		packetToSend.syscall = SC_ReleaseLock;
		packetToSend.serverArg = 0;
		
		pFromSToS.from = machineID;
		mFromSToS.from = mailBoxID;
		pFromSToS.to = index1/1000;
		mFromSToS.to = 0;
		mFromSToS.length = sizeof(packetToSend);
		
		memcpy((void *)data,(void *)&packetToSend,len);
		postOffice->Send(pFromSToS,mFromSToS,data);
	}
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
	
void signalCV(int index1,int index2,int machineID, int mailBoxID,int syscallType) {
	
	clientPacket packetToSend;
	client signalClient;
	PacketHeader pFromSToS;
	MailHeader mFromSToS;
	
	int len = sizeof(packetToSend);
	
	char *data=new char[len+1];
	
	data[len]='\0';
	
	/* check for the validity of the lock */
	
	/*if(index1<0 || index1 > ((myMachineID+1)*MAX_LOCK)){
		printf("\nSIGNALCV: INVALID LOCK INDEX");
		send("SIGNALCV", false,1, machineID, mailBoxID);
		return;
	} */
	
	/*check for the validity of the cv*/
	
	if(index2<0 || index2 > ((myMachineID+1)*MAX_CV)){
		printf("\nSIGNALCV: INVALID CV INDEX");
		send("SIGNALCV", false,1, machineID, mailBoxID);
		return;
	} 

	if(SERVERS == 1){
		if(serverLock[index1].owner.machineID!=machineID || serverLock[index1].owner.mailBoxID!=mailBoxID){
		printf("\nSIGNALCV : CLIENT(%d,%d) DOESNT HAVE THE LOCK",machineID,mailBoxID);
		send("SIGNALCV", false,1, machineID, mailBoxID);
		}
	}
	/*if(serverLock[index1].valid==false || serverCV[index2].valid==false ){
		printf("\nLOCK%d WITH CV %d IS INVALID",index1,index2);
		send("SIGNALCV", false,1, machineID, mailBoxID);
		return;
	}*/
	
	if(serverCV[index2].valid==false ){
		printf("\nLOCK%d WITH CV %d IS INVALID",index1,index2);
		send("SIGNALCV", false,1, machineID, mailBoxID);
		return;
	}
	
	if(serverCV[index2].lockID  != index1) {
		printf(" \nSIGNALCV: LOCKID%d -%s,CVID%d - %s DOESN'T MATCH FOR LOCKID%d - %s IN CVID%d - %s",index1,serverLock[index1].name,index2,serverCV[index2].name,serverCV[index2].lockID,serverLock[serverCV[index2].lockID].name,index2,serverCV[index2].name);
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
	printf("\nSIGNAL : CLIENT(%d,%d) IS SIGNALLED FROM WAIT QUEUE",signalClient.machineID,signalClient.mailBoxID);
	if(serverCV[index2].waitQueue->IsEmpty()) {
		serverCV[index2].lockID = -1;
	}
	
	serverCV[index2].count--;
	//printf("\nSIGNAL : WAITING CLIENT(%d,%d) IS WOKEN UP",signalClient.machineID, signalClient.mailBoxID);
	// Acquire lock - Acquire Lock will signal to the process, once lock has been acquired.
	
	/*acquire the lock and send reply message to the client with status set to true*/
	
	if(SERVERS == 1){
		acquireLock(index1, signalClient.machineID, signalClient.mailBoxID);
	}
	else{
		packetToSend.index1 = index1;
		packetToSend.syscall = SC_AcquireLock;
		packetToSend.serverArg = 0;
		
		pFromSToS.from = signalClient.machineID;
		mFromSToS.from = signalClient.mailBoxID;
		pFromSToS.to = index1/1000;
		mFromSToS.to = 0;
		mFromSToS.length = sizeof(packetToSend);
		
		memcpy((void *)data,(void *)&packetToSend,len);
		postOffice->Send(pFromSToS,mFromSToS,data);
	}
	
	if(syscallType != SC_BroadcastCV)
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
	
	/*if(index1<0 || index1 > ((myMachineID+1)*MAX_LOCK)){
		printf("\nBROADCASTCV: INVALID LOCK INDEX");
		send("BROADCASTCV", false,1, machineID, mailBoxID);
		return;
	} */
	
	if(index2<0 || index2 > ((myMachineID+1)*MAX_CV)){
		printf("\nBROADCASTCV: INVALID CV INDEX");
		send("BROADCASTCV", false,1, machineID, mailBoxID);
		return;
	} 
	
	/*check for the validity of the cv */

	/*if(serverLock[index1].valid==false && serverCV[index2].valid==false ){
		printf(" \nBROADCASTCV : LOCKID%d - %s,CVID%d - %s DOESN'T MATCH FOR LOCKID%d - %s IN CVID%d - %s",index1,serverLock[index1].name,index2,serverCV[index2].name,serverCV[index2].lockID,serverLock[serverCV[index2].lockID].name,index2,serverCV[index2].name);
		send("BROADCASTCV", false,1, machineID, mailBoxID);
		return;
	}*/
	if(serverCV[index2].valid==false){
		printf(" \nBROADCASTCV : LOCKID%d - %s,CVID%d - %s DOESN'T MATCH FOR LOCKID%d - %s IN CVID%d - %s",index1,serverLock[index1].name,index2,serverCV[index2].name,serverCV[index2].lockID,serverLock[serverCV[index2].lockID].name,index2,serverCV[index2].name);
		send("BROADCASTCV", false,1, machineID, mailBoxID);
		return;
	}
	/* If the cv has some clients,signal to every client waiting in the cv */
	
	while(!serverCV[index2].waitQueue->IsEmpty()) {
	
		signalCV(index1,index2,machineID,mailBoxID,SC_BroadcastCV);
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
	
	
	if(index < 0 || index > ((myMachineID+1)*MAX_CV)){
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
	
	if(index < 0 || index > ((myMachineID+1)*MAX_MV)){
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


void EndSimulation(){
	int j;
	client waitingClient;
	for(j=(myMachineID*1000);j<nextCVID;j++){
		if(serverCV[j].valid){
			while(!serverCV[j].waitQueue->IsEmpty()){
				waitingClient = *((client*)serverCV[j].waitQueue->Remove());
				send("ENDSIM",true,-99,waitingClient.machineID,waitingClient.mailBoxID);
			}
		}
	}
	for(j=(myMachineID*1000);j<nextLockID;j++){
		if(serverLock[j].valid){
			while(!serverLock[j].waitQueue->IsEmpty()){
				waitingClient = *((client*)serverLock[j].waitQueue->Remove());
				send("ENDSIM",true,-99,waitingClient.machineID,waitingClient.mailBoxID);
			}
		}
	}
	return;
}

int searchMyOwn(int syscall,char name[],int index){
	int ID = -1;
	if(syscall == SC_CreateLock)
		ID = searchMap('l',name);
	if(syscall == SC_CreateCV)
		ID = searchMap('c',name);
	if(syscall == SC_CreateMV)
		ID = searchMap('m',name);
	if(syscall==SC_GetMV || syscall == SC_SetMV){
		if(serverMV[index].valid){
			ID = searchMap('m',serverMV[index].name);
		}
		else{
			ID = -1;
		}
	}
	if(syscall==SC_AcquireLock || syscall == SC_ReleaseLock){
		if(serverLock[index].valid)
			ID = searchMap('l',serverLock[index].name);
		else
			ID = -1;
	}
	if(syscall==SC_WaitCV || syscall == SC_SignalCV || syscall == SC_BroadcastCV){
		if(serverCV[index].valid)
			ID = searchMap('c',serverCV[index].name);
		else
			ID = -1;
	}
	return ID;
}

void switchCase(int syscall,char name[],int index1,int index2,int machineID,int mailBoxID,int value){
	switch(syscall){
		case SC_CreateLock:
			printf("\nREQUEST :  CREATE LOCK FROM CLIENT(%d,%d), NAME : %s",machineID,mailBoxID,name);
			createLock(name,machineID,mailBoxID);
			break;
		case SC_CreateCV:
			printf("\nREQUEST :  CREATECV FROM CLIENT(%d,%d) NAME : %s",machineID,mailBoxID,name);
			createCV(name,machineID,mailBoxID);
			break;
		case SC_CreateMV:
			printf("\nREQUEST :  CREATE MV FROM CLIENT(%d,%d), NAME : %s",machineID,mailBoxID,name);
			createMV(name,value,machineID,mailBoxID);
			break;
		case SC_AcquireLock:
				printf("\nREQUEST :  ACQUIRE LOCK FROM CLIENT(%d,%d)",machineID,mailBoxID);
				acquireLock(index1,machineID,mailBoxID);
				break;
		case SC_ReleaseLock:
			printf("\nREQUEST :  RELEASE LOCK FROM CLIENT(%d,%d)",machineID,mailBoxID);
			releaseLock(index1,machineID,mailBoxID,value);
			break;
		case SC_SetMV:
			printf("\nREQUEST :  SET MV FROM CLIENT(%d,%d)",machineID,mailBoxID);
			setMV(index1,value,machineID,mailBoxID);
			break;
		case SC_GetMV:
				printf("\nREQUEST :  GET MV%d FROM CLIENT(%d,%d)",index1,machineID,mailBoxID);
				getMV(index1,machineID,mailBoxID);
				break;
		case SC_WaitCV:
				printf("\nREQUEST :  WAIT FROM CLIENT(%d,%d)",machineID,mailBoxID);
				waitCV(index1,index2,machineID,mailBoxID);
				break;
		case SC_SignalCV:
			printf("\nREQUEST :  SIGNAL FROM CLIENT(%d,%d)",machineID,mailBoxID);
			signalCV(index1,index2,machineID,mailBoxID,value);
			break;
		case SC_BroadcastCV:
			printf("\nREQUEST :  BROADCAST CV FROM CLIENT(%d,%d)",machineID,mailBoxID);
			broadcastCV(index1,index2,machineID,mailBoxID);
			break;
		case SC_EndSimulation:
			printf("\nREQUEST :  END SIMULATION FROM CLIENT(%d,%d)",machineID,mailBoxID);
			EndSimulation();
			break;
	}
}

void serverToServer(clientPacket packetReceived,int serverID){			/*This function will be called by the clientServer thread when the current server doesn't have the system call in its map*/
	PacketHeader pFromSToS,pFromSToC;									/*Creating Machine id objects*/
	MailHeader mFromSToS,mFromSToC;										/*Creating Mailbox id objects*/	
	clientPacket packetToSend;											/*Creating client packet object*/		
	serverPacket packetToSendToClient;									/*Creating server packet object*/
	
	//printf("\n My machine ID : %d",myMachineID);
	//packetReceived.print();
	int len=sizeof(packetReceived);
	int clientID,j;
	int index;
	int ID;
	int holdingServer = -1;
	client *waitingClient = NULL;
	
	bool flag=false;
	char *data=new char[len+1];
	
	data[len]='\0';
	
	ID = -1;
	
	if(packetReceived.syscall == SC_WaitCV || packetReceived.syscall == SC_SignalCV || packetReceived.syscall == SC_BroadcastCV)
		ID = searchMyOwn(packetReceived.syscall,packetReceived.name1,packetReceived.index2);				/*Searching in the current server map*/
	else if(packetReceived.syscall != SC_EndSimulation)														
		ID = searchMyOwn(packetReceived.syscall,packetReceived.name1,packetReceived.index1);				/*Searching in the current server map*/
			
	clientID = packetReceived.clientID;
	pFromSToC.to = clientID/100;																			/*Assigning the machine id of the client to send the packet to the client directly*/
	mFromSToC.to = clientID%100;																			/*Assigning the mailbox id of the client to send the packet to the client directly*/
	pFromSToC.from = myMachineID;
	mFromSToC.from = 0;
	mFromSToC.length = sizeof(packetReceived);
	if(ID!=-1){																								/*The program comes here if the current server has got the client's syscall in its map*/
		flag = 1;
		switchCase(packetReceived.syscall,packetReceived.name1,packetReceived.index1,packetReceived.index2,pFromSToC.to,mFromSToC.to,packetReceived.value);
	}
	else{
		index = -1;																							/*If the syscall is not in the curr server's map, it will add the client packet to its pending queue structure*/
		for(j=0;j<1000;j++){
			if(pendingQueue[j].received){
				if(pendingQueue[j].syscall==packetReceived.syscall && !strcmp(pendingQueue[j].name,packetReceived.name1)){
					index = j;																				/*Checks if the syscall is already in its pendingQueue structure*/
					break;
				}
			}
		}
		waitingClient = new client;
		waitingClient->machineID = pFromSToC.to;
		waitingClient->mailBoxID = mFromSToC.to;
		if(index !=-1){
			if(packetReceived.syscall==SC_CreateLock || packetReceived.syscall==SC_CreateCV || packetReceived.syscall == SC_CreateMV){			
				if(serverID < myMachineID){
					if(pendingQueue[index].doYouHave[serverID] == 0){										/*If current server's machine id > requested server's machine id*/
						printf("\n Replying yes to Server %d since that server already said NO",serverID);
						flag = 1;
						pendingQueue[index].requestWaitQueue->Append((void *)waitingClient);				/*If the syscall is already present in current server's pending queue, it will append the current syscall request to the index of its already present syscall using a queue data structure*/
						// Send YES - 1
					}
					else if(pendingQueue[index].doYouHave[serverID] == 1){									/*Sets the flag to 1 if the requested server's doYouHave has the value 1*/
						flag = 0;
						printf("\nReplying no to Server %d since my priority is low for request(%d,%s)",serverID,packetReceived.syscall,packetReceived.name1);
						//Send NO - 0
					}
					else{																					/*Sets the flag to 0 if the requested server's doYouHave doesn't have the value either 0 or 1 */
						flag = 0;
						printf("\nReplying NO to Server %d since my priority is low for request(%d,%s)",serverID,packetReceived.syscall,packetReceived.name1);
						//Send NO - 0
					}
				}
				else{ //My machine's priority is more													/*If current server's machine id < requested server's machine id*/
					if(pendingQueue[index].doYouHave[serverID] == 0){
						flag = 1;
						printf("\n Replying yes to Server %d since my priority is higher",serverID);
						pendingQueue[index].requestWaitQueue->Append((void *)waitingClient);
						// Send  YES - 1
					}
					else if(pendingQueue[index].doYouHave[serverID] == 1){								/*Sets the flag to 0 if the requested server's doYouHave has the value 1*/
						flag = 0;
						printf("\n Replying no to Server %d since that server already said yes",serverID);
					}
					else{																				/*Sets the flag to 1 if the requested server's doYouHave doesn't have the value either 0 or 1 */
						flag = 1;
						printf("\n Replying YES to Server %d since that server didnt reply anything to me yet and my priority is higher",serverID);
						pendingQueue[index].requestWaitQueue->Append((void *)waitingClient);
					}
				}
			}
		}
		else{																							/*Enters here if the current server doesn't have the requested syscall in its pendingQueue*/		
			flag = 0;
			printf("\nReplying no to Server %d since I dont have any pending request(%d,%s)",serverID,packetReceived.syscall,packetReceived.name1);
			//Send NO - 0
		}
	}
	
	packetToSend.clientID = clientID;																	/*Assigning the requred fields to a new packet which is to be sent*/	
	packetToSend.syscall = packetReceived.syscall;
	packetToSend.status = flag;
	packetToSend.index1 = packetReceived.index1;
	packetToSend.index2 = packetReceived.index2;
	packetToSend.serverArg = 2;																			/*Assigning the serverArg value to 2 which indicates that the reply is for the doYouHave message*/
	strcpy(packetToSend.name1,packetReceived.name1);
	packetToSend.value = packetReceived.value;
	
	//packetToSend.print();
	
	mFromSToS.to = 0;
	mFromSToS.length = sizeof(packetReceived);
	pFromSToS.to = serverID;
	pFromSToS.from = myMachineID;
	
	printf("\n Server %d sending %d to Server %d's doYouHave request",myMachineID,flag,serverID); 
	memcpy((void *)data,(void *)&packetToSend,len);
	postOffice->Send(pFromSToS,mFromSToS,data);															/*Sendint the reply*/
	return;
}

void clientToServer(){																					/*This function is forked initially by the server*/
																										
	PacketHeader pFromCToS,pFromSToS;																	/*Creating Machine id objects*/
	MailHeader mFromCToS,mFromSToS;                                                                     /*Creating Mailbox id objects*/	
	clientPacket packetReceived;                                                                        /*Creating client packet object*/
	clientPacket packetToSend;                                                                          /*Creating server packet object*/
	
	int len=sizeof(packetReceived);
	int index;
	int clientID,j;
	int count=0;
	int terminatingCount = 0;
	int ID =-1;
	int holdingServer = -1;
	
	client *waitingClient = NULL;
	client *processingClient = NULL;
	
	char *data=new char[len+1];
	
	data[len]='\0';
	
	while(1){
	
		/* server listens to client request infinitely */
	
		printf("\nSERVER : WAITING FOR CLIENT REQUEST");
		
		/* receives request from the client and copies it into character array in memory */
		
		postOffice->Receive(0,&pFromCToS,&mFromCToS,data);											/*Receives the reply*/
		memcpy((void *)&packetReceived,(void *)data,len);
		
		if(packetReceived.serverArg == 0){ // request From client									/*ServerArg value is 0, indicating that the request if from the client*/
			printf("\nREQUEST : %d FROM CLIENT(%d,%d), NAME : %s",packetReceived.syscall,pFromCToS.from,mFromCToS.from,packetReceived.name1);
			
			if(SERVERS == 1){																		/*Used for test case*/
				switchCase(packetReceived.syscall,packetReceived.name1,packetReceived.index1,packetReceived.index2,pFromCToS.from,mFromCToS.from,packetReceived.value);
				continue;
			}
			
			mFromSToS.from = 0;																		/*Assignint the required fields to send the packet*/
			mFromSToS.to = 0;
			pFromSToS.from = myMachineID;
			mFromSToS.length = mFromCToS.length;
			
			//packetReceived.print();
			
			packetToSend.clientID = (pFromCToS.from*100)+mFromCToS.from;							/*This is done for appending both the machine and the mailbox id into a single value, for the server to identify the credentials of the client*/
			packetToSend.index1 = packetReceived.index1;
			packetToSend.index2 = packetReceived.index2;
			packetToSend.value = packetReceived.value;
			packetToSend.syscall = packetReceived.syscall;
			strcpy(packetToSend.name1,packetReceived.name1);
			packetToSend.serverArg = 1;
			
			ID = -1;
			if(packetReceived.syscall == SC_WaitCV || packetReceived.syscall == SC_SignalCV || packetReceived.syscall == SC_BroadcastCV)
				ID = searchMyOwn(packetReceived.syscall,packetReceived.name1,packetReceived.index2); /*Checking the current server's map*/
			else if(packetReceived.syscall != SC_EndSimulation)
				ID = searchMyOwn(packetReceived.syscall,packetReceived.name1,packetReceived.index1);
			else{
				ID = 0;
			}		
						
			if(ID == -1){
				
				if(packetReceived.syscall==SC_CreateLock || packetReceived.syscall==SC_CreateCV || packetReceived.syscall == SC_CreateMV){
					index = -1;
					for(j=0;j<1000;j++){
						if(pendingQueue[j].received){
							if(pendingQueue[j].syscall==packetReceived.syscall && !strcmp(pendingQueue[j].name,packetReceived.name1)){
								index = j;
								break;
							}	
						}
					}
					if(index == -1){
						index = requestBitmap->Find();
						//printf("\n Index of pending Queue(%d,%s): %d",packetReceived.syscall,packetReceived.name1,index);
						pendingQueue[index].requestWaitQueue = new List;
						for(j=0;j<SERVERS;j++)
							pendingQueue[index].doYouHave[j] = 3;
						if(pendingQueue[index].requestWaitQueue->IsEmpty())
							printf("\n List is initially empty");
						
					}
					
					waitingClient = new client;
					waitingClient->machineID = pFromCToS.from;
					waitingClient->mailBoxID = mFromCToS.from;
					
					pendingQueue[index].syscall = packetReceived.syscall;
					strcpy(pendingQueue[index].name,packetReceived.name1);
					pendingQueue[index].value = packetReceived.value;
					pendingQueue[index].received = true;
					pendingQueue[index].requestWaitQueue->Append((void *)waitingClient);
					pendingQueue[index].doYouHave[myMachineID] = 0;
					memcpy((void *)data,(void *)&packetToSend,len);
					
					for(j=0;j<SERVERS;j++){
						if(j!=myMachineID){ 										// Current Server shouldnt send to its own mailbox
							printf("\nSending doyouhave to servers");
							pFromSToS.to = j;
							postOffice->Send(pFromSToS,mFromSToS,data);
						}
					}
				}
				else if(packetReceived.syscall==SC_AcquireLock || packetReceived.syscall==SC_ReleaseLock || packetReceived.syscall==SC_GetMV || packetReceived.syscall==SC_SetMV)
				{																	// Handling non create requests
					packetToSend.index1 = packetReceived.index1;
					packetToSend.syscall = packetReceived.syscall;
					packetToSend.serverArg = 0;
					
					pFromSToS.from = pFromCToS.from;
					mFromSToS.from = mFromCToS.from;
					pFromSToS.to = packetReceived.index1/1000;
					mFromSToS.to = 0;
					mFromSToS.length = sizeof(packetToSend);
					
					memcpy((void *)data,(void *)&packetToSend,len);
					postOffice->Send(pFromSToS,mFromSToS,data);
				}
				else{
					packetToSend.index1 = packetReceived.index1;
					packetToSend.syscall = packetReceived.syscall;
					packetToSend.serverArg = 0;
					
					pFromSToS.from = pFromCToS.from;
					mFromSToS.from = mFromCToS.from;
					pFromSToS.to = packetReceived.index2/1000;
					mFromSToS.to = 0;
					mFromSToS.length = sizeof(packetToSend);
					
					memcpy((void *)data,(void *)&packetToSend,len);
					postOffice->Send(pFromSToS,mFromSToS,data);
				}
			}
			else{																	// Processing the request since the resource is available in the current server
				switchCase(packetReceived.syscall,packetReceived.name1,packetReceived.index1,packetReceived.index2,pFromCToS.from,mFromCToS.from,packetReceived.value);
			}
		}
		else if(packetReceived.serverArg == 1){ 									// doYouHave request From server
			printf("\nInside doyouhave request server message : From Server : %d",pFromCToS.from);
			serverToServer(packetReceived,pFromCToS.from);
		}
		else if(packetReceived.serverArg ==2){ 										// doYouHave reply from server
			index = -1;
			for(j=0;j<1000;j++){
				if(pendingQueue[j].received){										// Identify the corresponding entry in the pending queue
					if(pendingQueue[j].syscall==packetReceived.syscall && !strcmp(pendingQueue[j].name,packetReceived.name1)){
						index = j;
						break;
					}	
				}
				else
					index = -1;
			}
			if(index != -1){
				pendingQueue[index].doYouHave[pFromCToS.from] = packetReceived.status;
				count = 0;
				terminatingCount = 0;
				for(j=0;j<SERVERS;j++){												
					if(pendingQueue[index].doYouHave[j]==0){ 						// Identifying the number of servers which said NO
						count++;
					}
					if(pendingQueue[index].doYouHave[j]!=3)							// Identifying the number of servers which replied
						terminatingCount++;
				}
				printf("\n So far Doyouhave msgs from %d servers have been received",count);
				if(count==SERVERS){

					while(!(pendingQueue[index].requestWaitQueue->IsEmpty())){		// Process the request if all the other servers said No for the DoYouHave request
						processingClient = ((client*)pendingQueue[index].requestWaitQueue->Remove());
						switchCase(packetReceived.syscall,packetReceived.name1,packetReceived.index1,packetReceived.index2,processingClient->machineID,processingClient->mailBoxID,packetReceived.value);
					}
				}
				if(terminatingCount==SERVERS){										// Remove the entry in the pending queue if all the servers replied
					//requestBitmap->Clear(index);
					pendingQueue[index].syscall = -1;
					pendingQueue[index].name[0] = '\0';
					pendingQueue[index].received = false;
					//delete pendingQueue[index].requestWaitQueue;
				}
			}
			else
				printf("\nPending Queue(%d,%s): %d is no longer valid",packetReceived.syscall,packetReceived.name1,index);
		}
		else{													// Never occurs
			printf("\nINVALID SERVER ARGUMENT");
		}
	}
	
}


void pingingThread()
{
	PacketHeader pFromSTOC;
	MailHeader mFromSTOC;
	serverPacket pingMsg;
	
	int j,k;
	int machineID;
	int mailBoxID;
	int len=sizeof(pingMsg);
	
	bool success;
	
	char* data = new char[len+1];
	
	
	
	time_t startTime = time(NULL);							// Retrieving the system time twice
	time_t currTime = time(NULL);
	time_t delay=currTime - startTime;
	
	delay=delay/1000;
	
	while(1)
	{
		//sleep(10);
		currentThread->Yield();
		currTime = time(NULL);
		
		if(currTime-startTime > delay/100)				//Checking if the delay is high enough to run the thread
		{
			
			for(j=0;j<nextLockID;j++)
			{	
				if(serverLock[j].valid==1 && serverLock[j].available==false)  // Checking whether the lock entry is valid and it is occupied
				{
					memcpy((void*)data, (void*)&pingMsg,len);
					data[len]='\0';
					
					machineID = serverLock[j].owner.machineID;
					mailBoxID = serverLock[j].owner.mailBoxID;
					
					pFromSTOC.to=machineID;
					mFromSTOC.to=20;
					mFromSTOC.from=2;
					mFromSTOC.length=len;
					
					success = postOffice->Send(pFromSTOC,mFromSTOC,data);		// Sending a ping message to check whether the lock owner is alive
					
					if(!success)												// IF not, release the lock and handover the ownership to the waiting client
					{
						printf("CLIENT(%d,%d) IS DEAD WITH LOCK(%d)\n",machineID,mailBoxID,j);
						releaseLock(j,machineID,mailBoxID,SC_ReleaseLock);
						for(k=0;k<3;k++)
							currentThread->Yield();
					}
					
				}
			}
			
			startTime = currTime;
		}
		
	}
}
	
/* 
Server()
	- gets the client request packet
	- decodes the client packet and handles the requested system call
	- redirects the request to particular system call functions and replies to the requested client
*/

void Server(){
	
	nextLockID=myMachineID*1000;
	nextCVID=myMachineID*1000;
	nextMVID=myMachineID*1000;

	Thread *newThread = new Thread("clientToServer");
	newThread->Fork((VoidFunctionPtr)clientToServer,0);
	requestBitmap = new BitMap(5000);
	
	if(extraCredit == 1)
	{
		printf("\nFORKING PINGING THREAD");
		Thread *pingingT = new Thread("pingThread");
		pingingT->Fork((VoidFunctionPtr)pingingThread,0);
	}
}
