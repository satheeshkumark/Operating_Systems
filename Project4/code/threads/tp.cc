#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "unistd.h"
#include "math.h"

//Creating Exam Sheet form
struct ExamSheet
{
	char* name;
	int age;
	int id;
	
	bool pain;
	bool nausea;
	bool hear_alien_voices;

	float temperature;
	int BP;
	
	int xray;
	int shot;
	
	char* image;
};

List* ExamSheetQueue = new List;			//Queue of forms with WRN
int patientWaitingCount = 0;				//MV: Number of patients in queue for wrn
int wrnState=0;	
int wrnNurseState=0;							//MV: To know the state of wrn: 0-> Free, 1-> Busy
char *wrnTask=" ";							//Task to be performed: giveForm or getForm
int decision;

int ExamRoomMV[]={0,0,0,0,0,0};   			//0 for free and 1 for occupied
int DoctorMV[]={0,0,0,0};					//0 for free and 1 for occupied
int nurseTask2MV[]={1,1,1,1,1,1};			//0 for free and 1 for occupied
int XRayRoomMV[]={0,0,0};					//0 for free and 1 for occupied

int nurseWaitingCount = 0;
int ExamRoomCount = 0;
int patientAssignedRoomGlobal=0;
float patientTemperature;
int patientBP;
int doctorExamRoomGlobal;
int doctorCount=0;
int nurseCount=0;
int doctorWaitCount[]={0,0,0,0,0,0};
char * imageGlobal;
int XRayCount[]={0,0,0};
int waitInXRayLineGlobal[6];
int XRayRoomGlobal[6];
int SupplyCabinetCount=0;
int SupplyCabinetState=0;

struct ExamSheet wrnExamSheet;			//For form exchange betn WRN and Patient
struct ExamSheet myExamSheet;			//Betn WRN and Patient
struct ExamSheet ExamSheetPatient;		//Betn WRN and Nurse
struct ExamSheet WallPocketExamSheet[6];	//Nurse hangs on Exam Room Wall
struct ExamSheet patientExamSheetGlobal; //Betn Doctor-Patient
struct ExamSheet XRayExamSheetGlobal;

Lock* waitingRoomLock=new Lock("waitingRoomLock");
Lock* wrnLock=new Lock("wrnLock");
//Lock* wrnLock=new Lock("wrnLock");
Lock* ExamRoomLock=new Lock("ExamRoomLock");
Lock* registeredPatientLock=new Lock("registeredPatientLock");
//Lock* nurseLock=new Lock("nurseLock");
Lock* RoomLock[6];			//Array of Locks
Lock* doctorRoomLock=new Lock("doctorRoomLock");
Lock* doctorNurseLock=new Lock("doctorNurseLock");
Lock* DoctorAvailableLock=new Lock("DoctorAvailableLock");
Lock* Task2Lock=new Lock("Task2Lock");
Lock* XRayLock=new Lock("XRayLock");
Lock* XRayRoomLock[3];
Lock* waitingForXRayLock[3];
Lock* XRayWaitingRoomLock[3];
Lock* SupplyCabinetLock=new Lock("SupplyCabinetLock");

Condition* patientWaitingCV=new Condition("patientWaitingCV");
Condition* wrnWaitingCV=new Condition("wrnWaitingCV");
Condition* nurseWrnWaitingCV=new Condition("nurseWrnWaitingCV");
Condition* nurseWaitingCV=new Condition("nurseWaitingCV");
Condition* nurseRegisteredPatientCV=new Condition("nurseRegisteredPatientCV");
//Condition* nursePatientCV=new Condition("nursePatientCV");
Condition* RoomLockCV[6];	//Array of CV's
Condition* doctorWaitingCV=new Condition("doctorWaitingCV");
Condition* doctorNurseCV=new Condition("doctorNurseCV");
Condition* nurseWaitingForDoctorCV=new Condition("nurseWaitingForDoctorCV");
Condition* nurseDoctorCV=new Condition("nurseDoctorCV");
Condition* task2CV=new Condition("task2CV");
Condition* XRayRoomCV[3];
Condition* XRayRoomNurseCV[3];
Condition* waitingForXRayCV[3];
Condition* XRayWaitingRoomCV[3];
Condition* SupplyCabinetCV=new Condition("SupplyCabinetCV");

//#---------------------------Waiting Room Nurse------------------------------#
void waitingRoomNurse(int index)
{
	struct ExamSheet patientExamSheet; 
	while(1)
	{
	
		//printf("WRN entered the clinic\n");
		DEBUG('p', "WRN %d entered the clinic\n", index);
	
		waitingRoomLock->Acquire();	
		
		//printf("WRN %d acquired the waitingRoomlock", index);
		DEBUG('p', "WRN %d acquired the waitingRoomlock for PI\n", index);
		
		//printf("patientWaitingCount: %d\n",patientWaitingCount);
		if(patientWaitingCount>0)	//Patients are in line
		{
			//Waking the patient
			patientWaitingCV->Signal(waitingRoomLock);
			DEBUG('p',"WRN Signalled Patient on patientWaitingCV using waitingRoomLock for PI\n");
			patientWaitingCount--;
			//wrnState="busy";
			wrnState=1;
		}
		else
		{
			//wrnState="free";
			wrnState=0;
		}
		
		wrnLock->Acquire();
		
		//printf("WRN %d acquired the wrnlock\n", index);
		DEBUG('p', "WRN %d acquired the wrnlock for PI\n", index);
		
		waitingRoomLock->Release();
		DEBUG('p',"WRN Released waitingRoomLock\n");
		
		//#--------------------WRN<-->Patient Interaction---------------------------------#
		
		wrnWaitingCV->Wait(wrnLock);
		DEBUG('p',"WRN out of Wait on wrnWaitingCV using wrnLock for PI pehli bar.\n");

		//Decide if task is either to GETFORM or GIVEFORM
		
		if(decision == 1)			// GETFORM
		{
			//Make a new exam sheet
			//wrnExamSheet;
			
			printf("Giving form to the patient\n");
			DEBUG('p', " WRN Giving form to the patient\n");
			
			//Signal the Patient to take the Empty Form and Wait
			wrnWaitingCV->Signal(wrnLock);
			DEBUG('p',"WRN Signalled on wrnWaitingCV using wrnLock for GETFORM for PI\n");
			wrnWaitingCV->Wait(wrnLock);
			DEBUG('p',"WRN out of wait on wrnWaitingCV using wrnLock for GETFORM for PI\n");
			
		}
		else if(decision == 2)		//GIVEFORM
		{
			//Signal the Patient to give the Filled Form and Wait
			wrnWaitingCV->Signal(wrnLock);
			DEBUG('p',"WRN Signalled on wrnWaitingCV using wrnLock for GIVEFORM for PI\n");
			wrnWaitingCV->Wait(wrnLock);
			DEBUG('p',"WRN out of wait on wrnWaitingCV using wrnLock for GIVEFORM for PI\n");
			
			//Take the copy from patient and keep to itself
			patientExamSheet=myExamSheet;
			
			printf("Take the form from the Patient %d\n",patientExamSheet.id);
			DEBUG('p', " WRN:Take the form from the Patient %d\n", patientExamSheet.id);
			
			//Appending the sheet to the ExamSheetQueue
			ExamSheetQueue->Append(&patientExamSheet);
			printf("Appending the form %d to the ExamSheetQueue\n",patientExamSheet.id);
			DEBUG('p', " WRN is Appending the form %d to the ExamSheetQueue\n",patientExamSheet.id);
		}
		
		/*if(patientWaitingCount==0)
		{
			wrnLock->Release();
			//printf("Name %s\n",ExamSheetQueue->name);
			exit(1);
		}*/
		
		wrnLock->Release();
		DEBUG('p',"WRN released wrnLock after appending the form\n");
		
		//===============Patient Task interaction is over========================//
		
		//Delay
		/*int r = 200;
		for(int i=0;i<r;i++)
		{
			currentThread->Yield();
		}*/
		

		//=================Nurse-WRN Interaction Begins==========================//
		
		//Again acquire waitingRoomLock to interact with Nurse
		waitingRoomLock->Acquire();
		DEBUG('p',"WRN acquired waitingRoomLock for NI\n");
		if(nurseWaitingCount>0)
		{
			nurseWaitingCV->Signal(waitingRoomLock);
			DEBUG('p',"WRN Signalled Nurse on nurseWaitingCV using waitingRoomLock for NI\n");
			nurseWaitingCount--;
			wrnNurseState=1;				// WRN is BUSY
		}
		else
		{
			wrnNurseState=0;				// WRN is FREE
			DEBUG('p',"No Nurse is waiting so WRN set to free\n");
		}
		
		wrnLock->Acquire();
		DEBUG('p',"WRN acquired wrnLock for NI\n");
		waitingRoomLock->Release();
		DEBUG('p',"WRN released waitingRoomLock for NI\n");
		
		//Waiting to see if Nurse Signals for an empty ExamRoom
		nurseWrnWaitingCV->Wait(wrnLock);
		DEBUG('p',"WRN out of on nurseWrnWaitingCV using wrnLock for NI 1st time.\n");
		if(!ExamSheetQueue->IsEmpty())
		{
			ExamSheetPatient=*((ExamSheet*)ExamSheetQueue->Remove());
			
			//Giving first form to the nurse--//
			printf("WRN is giving Patient %d's form to the nurse\n",ExamSheetPatient.id);
			nurseWrnWaitingCV->Signal(wrnLock);
			DEBUG('p',"WRN Signalled Nurse on nurseWrnWaitingCV using wrnLock for NI\n");
			nurseWrnWaitingCV->Wait(wrnLock);
			DEBUG('p',"WRN out of wait on nurseWrnWaitingCV using wrnLock for NI 2nd time\n");
		}
		else
		{
			//ExamSheetPatient=NULL;
			ExamSheetPatient.id=0;
			nurseWrnWaitingCV->Signal(wrnLock);
			DEBUG('p',"WRN Signalled Nurse on nurseWrnWaitingCV using wrnLock for NI id is 0\n");
			nurseWrnWaitingCV->Wait(wrnLock);
			DEBUG('p',"WRN out of wait on nurseWrnWaitingCV using wrnLock for NI if id is 0\n");
		}
		
		wrnLock->Release();
		DEBUG('p',"WRN released wrnLock after talkin with nurse\n");
		
		//=====================Nurse-WRN Interaction ends==================================//
		
	}//End of while
}

void Patient(int index)
{

	int patientExamRoom; 				// To notify which ExamRoom he is escorted
	struct ExamSheet patientExamSheet;
	int patientXRayRoom;
	int waitInXRayLine;
	
	printf("Patient %d entered the clinic\n", index);
	DEBUG('p', "Patient %d entered the clinic\n", index);
	
	//--------------------Entering the queue to get the form----------------------------
	waitingRoomLock->Acquire();
	
	DEBUG('p', "Patient %d acquired the waitingRoomlock for GETFORM\n", index);
	
	if(wrnState==1)		//WRN busy. Wait in line
	{
		patientWaitingCount++;
		patientWaitingCV->Wait(waitingRoomLock);
		DEBUG('p',"GETFORM: Patient %d out of Wait and Patient Waiting Count: %d in \n",index, patientWaitingCount);
	}
	else
	{
		//wrnState="busy";
		wrnState=1;
	}
	
	waitingRoomLock->Release();
	DEBUG('p',"Patient %d released waitingRoomLock for GETFORM\n", index);
	
	//#---------------------------WRN<--> Patient Interaction--------------------------------#
	
	//---------------------------------GETFORM----------------------------------------------
	//Acquire Lock to get the form
	wrnLock->Acquire();
	
	DEBUG('p', "Patient %d acquired the wrnlock for GETFORM\n", index);
	DEBUG('p', "Patient %d recieving the empty form\n", index);
	
	//Get the form from WRN
	//wrnTask="getForm";
	decision = 1;
	DEBUG('p',"Patient %d set decision 1\n", index);
	//Patient Signal's WRN that he needs an empty form and goes to wait
	wrnWaitingCV->Signal(wrnLock);
	DEBUG('p',"Patient %d Signalled on wrnWaitingCV using wrnLock for GETFORM\n", index);
	wrnWaitingCV->Wait(wrnLock);
	DEBUG('p',"Patient %d out of wait on wrnWaitingCV using wrnLock for GETFORM\n", index);
	
	//Patient takes empty form from WRN
	patientExamSheet=wrnExamSheet;
	
	printf("Patient %d recieved the empty form\n", index);
	DEBUG('p', "Patient %d recieved the empty form\n", index);
	
	wrnWaitingCV->Signal(wrnLock);
	DEBUG('p',"Patient %d again Signalled on wrnWaitingCV using wrnLock for GETFORM\n", index);

	wrnLock->Release();
	DEBUG('p',"Patient %d released wrnLock\n", index);
	//Done with getForm
	
	//----------------------------GIVEFORM-----------------------------------------
	//--------------------------------Entering the queue to submit the form------------------
	waitingRoomLock->Acquire();
	DEBUG('p',"Patient %d acquired waitingRoomLock for GIVEFORM\n", index);
	
	if(wrnState==1)		//WRN busy. Wait in line
	{
		patientWaitingCount++;
		patientWaitingCV->Wait(waitingRoomLock);
		DEBUG('p',"GIVEFORM: Patient %d out of wait and Patient Waiting Count: %d\n",index, patientWaitingCount);
	}
	else
	{
		//wrnState="busy";
		wrnState=1;
	}
	
	waitingRoomLock->Release();
	DEBUG('p',"Patient %d released waitingRoomLock for GIVEFORM\n", index);
	
	/*------------Lock to give the form---------------*/
	wrnLock->Acquire();
	DEBUG('p',"Patient %d acquired wrnLock for GIVEFORM\n", index);
	//Give the form to WRN
	//wrnTask="giveForm";
	decision = 2;
	DEBUG('p',"Patient %d set decision 2\n", index);
	//Filling the ExamRoomSheet
	patientExamSheet.name="Sagar";
	patientExamSheet.age=50;
	patientExamSheet.id=index;
	patientExamSheet.pain=true;
	patientExamSheet.nausea=false;
	patientExamSheet.hear_alien_voices=true;
	
	myExamSheet=patientExamSheet;
	
	//Patient Signals WRN that he wants to submit filled form and Waits
	wrnWaitingCV->Signal(wrnLock);
	DEBUG('p',"Patient %d Signalled on wrnWaitingCV using wrnLock for GIVEFORM\n", index);
	wrnWaitingCV->Wait(wrnLock);
	DEBUG('p',"Patient %d Out of waiting queue after submitting form.\n",index);
	
	
	printf("Patient %d submits the Filled Form to WRN.\n", index);
	DEBUG('p', "Patient %d submits the Filled Form to WRN.\n", index);
	
	wrnWaitingCV->Signal(wrnLock);
	DEBUG('p',"Patient %d :Signalled WRN after submitting.\n", index);
	
	wrnLock->Release();
	DEBUG('p',"Patient %d released wrnLock for GIVEFORM\n", index);
	//Done with giveForm and waiting for the Nurse
	//#---------------------------Done with WRN-Patient Interaction----------------------------#
	
	//#---------------------------------Nurse-Patient-Interaction------------------------------#
	//Patient going with nurse to Exam Room 
	registeredPatientLock->Acquire();
	DEBUG('p',"Patient %d acquired registeredPatientLock\n", index);
	nurseRegisteredPatientCV->Wait(registeredPatientLock);
	DEBUG('p',"Patient %d out of wait on nurseRegisteredPatientCV using registeredPatientLock\n", index);
	
	patientExamRoom=patientAssignedRoomGlobal;
	printf("Patient %d going with nurse at Room %d\n",index, patientExamRoom);
	
	nurseRegisteredPatientCV->Signal(registeredPatientLock);
	DEBUG('p',"Patient %d Signalled on nurseRegisteredPatientCV using registeredPatientLock\n", index);

	RoomLock[patientExamRoom]->Acquire();
	DEBUG('p',"Patient %d acquired RoomLock[%d]\n", index,patientExamRoom);
	
	registeredPatientLock->Release();
	DEBUG('p',"Patient %d released registeredPatientLock\n", index);

	//Performing symptoms check with Nurse
	//Waiting for nurse to come in the Exam Room and check my symptoms 
	RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
	DEBUG('p',"Patient %d out of wait on RoomLockCV[%d] using RoomLock[%d]\n", index, patientExamRoom, patientExamRoom);
	
	//Patient Signals Nurse to check the Symptoms and goes to Wait
	RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
	DEBUG('p',"Patient %d Signalled on RoomLockCV[%d] using RoomLock[%d]\n", index, patientExamRoom,patientExamRoom);
	
	RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
	DEBUG('p',"Patient %d out of wait on RoomLockCV[%d] using RoomLock[%d] Signalled by Nurse\n", index, patientExamRoom, patientExamRoom);
	printf("Nurse has completed Patient %d Checking Symptoms\n",index );
	
	//Patient is waiting for Doctor's Visit
	
	RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
	DEBUG('p',"Patient %d out of Wait on RoomLock %d Signalled by Doctor\n", index, patientExamRoom);
	
	printf("Patient %d giving examsheet to Doctor\n", index);
	
	patientExamSheetGlobal=patientExamSheet;
	
	RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
	DEBUG('p',"Patient %d Signalled Doctor after giving the ExamSheet\n", index);
	
	printf("Doctor is examining Patient %d\n", index);
	
	RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
	DEBUG('p',"Patient %d out of wait After doctor Completes examining\n", index);
	patientExamSheet=patientExamSheetGlobal;
	
	printf("Patient %d done with doctors 1st visit. Xray %d. Shot %d.\n", index, patientExamSheet.xray, patientExamSheet.shot);
	//--------------------------Patient-Xray Begins------------------------------------//
	//Waiting for Nurse to come
	RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
	
	//Only XRay
	if(patientExamSheet.xray==1 && patientExamSheet.shot==0)
	{
	    //XRayLock->Acquire();
		patientXRayRoom=XRayRoomGlobal[patientExamRoom];
		waitInXRayLine=waitInXRayLineGlobal[patientExamRoom];
		//printf("Patient %d going to Xray Room %d\n", index, patientXRayRoom);
		//XRayLock->Release();
		if(waitInXRayLine==0)					//Not to Wait
		{
			printf("Patient %d going to Xray Room %d\n", index, patientXRayRoom);
			RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
			DEBUG('x',"Patient %d Waiting for Nurse to go and talk with XT\n",index);
			RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
			DEBUG('x',"Patient %d Waiting for Nurse to go and talk with XT\n",index);
			
			//Nurse passed him necessary info
			//Now talking with XRay Technician

			XRayRoomLock[patientXRayRoom]->Acquire();
			
			//Signalling the XRayTechnician
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
			
		}
		else									//Need to Wait
		{
			printf("Patient %d going to Xray Room %d\n", index, patientXRayRoom);
			RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
			
			//Patient done with Nurse to escort to Xray Room
			
			//Waiting in Line for XRay
			waitingForXRayLock[patientXRayRoom]->Acquire();
			//RoomLock[patientExamRoom]->Release();
		
			XRayCount[patientXRayRoom]++;
			waitingForXRayCV[patientXRayRoom]->Wait(waitingForXRayLock[patientXRayRoom]);
			
			//Interacting with XRay Technician
			waitingForXRayLock[patientXRayRoom]->Release();
			XRayRoomLock[patientXRayRoom]->Acquire();
			
			XRayExamSheetGlobal=patientExamSheet;
			
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
		}
			
			
		printf("Patient %d is going onto the Table for XRay Test\n",index);
			
		if(patientExamSheet.xray==1)
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
			
			patientExamSheet.image=imageGlobal;
			printf("Patient %d is done with XRay Image: %s\n",index,patientExamSheet.image);
			
			XRayRoomLock[patientXRayRoom]->Release();
				
			printf("Patient %d done with XRay\n",index);
			
			/*Waiting For Nurse to come
			XRayWaitingRoomLock[patientExamRoom]->Acquire();
			
			XRayWaitingRoomCV->Wait(XRayWaitingRoomLock[patientExamRoom]);
				
			//Going with Nurse back to his Exam Room
				
			XRayWaitingRoomLock[patientExamRoom]->Release();*/
		}
		else if(patientExamSheet.xray==2)
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
			
			printf("Patient %d moving after 1st Image\n",index);
				
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
				
			patientExamSheet.image=imageGlobal;
			printf("Patient %d is done with XRay Image\n",index);
				
			XRayRoomLock[patientXRayRoom]->Release();
				
			/*XRayWaitingRoomLock[patientXRayRoom]->Acquire();
				
			//Waiting For Nurse to come
			XRayWaitingRoomCV->Wait(XRayWaitingRoomLock[patientXRayRoom]);
				
			//Going with Nurse back to his Exam Room
				
			XRayWaitingRoomLock[patientXRayRoom]->Release();*/
		}
		else
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
			
			printf("Patient %d moving after 1st Image\n",index);
				
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
				
			printf("Patient %d moving after 2nd Image\n",index);
				
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
				
			patientExamSheet.image=imageGlobal;
			printf("Patient %d is done with XRay Image\n",index);
				
			XRayRoomLock[patientXRayRoom]->Release();
				
			/*XRayWaitingRoomLock[patientXRayRoom]->Acquire();
				
			//Waiting For Nurse to come
			XRayWaitingRoomCV->Wait(XRayWaitingRoomLock[patientXRayRoom]);
				
			//Going with Nurse back to his Exam Room
				
			XRayWaitingRoomLock[patientXRayRoom]->Release();*/
		}
	}//Only XRay If done
	
	//Only Shot
	else if(patientExamSheet.xray==0 && patientExamSheet.shot==1)
	{
		//Letting Nurse go to get the medicine for shot
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
		
		printf("Patient %d is ready to take Shot\n",index);
		
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
		
	}
	
	//Both XRay and Shot done
	else if(patientExamSheet.xray==1 && patientExamSheet.shot==1)
	{
		//1st Shot
		//Letting Nurse go to get the medicine for shot
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
		
		printf("Patient %d is ready to take Shot\n",index);
		
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
		
		//2nd XRay
		//XRayLock->Acquire();
		patientXRayRoom=XRayRoomGlobal[patientExamRoom];
		waitInXRayLine=waitInXRayLineGlobal[patientExamRoom];
		//printf("Patient %d going to Xray Room %d\n", index, patientXRayRoom);
		//XRayLock->Release();
		if(waitInXRayLine==0)					//Not to Wait
		{
			printf("Patient %d going to Xray Room %d\n", index, patientXRayRoom);
			RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
			DEBUG('x',"Patient %d Waiting for Nurse to go and talk with XT\n",index);
			RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
			DEBUG('x',"Patient %d Waiting for Nurse to go and talk with XT\n",index);
			
			//Nurse passed him necessary info
			//Now talking with XRay Technician

			XRayRoomLock[patientXRayRoom]->Acquire();
			
			//Signalling the XRayTechnician
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
			
		}
		else									//Need to Wait
		{
			printf("Patient %d going to Xray Room %d\n", index, patientXRayRoom);
			RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
			
			//Patient done with Nurse to escort to Xray Room
			
			//Waiting in Line for XRay
			waitingForXRayLock[patientXRayRoom]->Acquire();
			//RoomLock[patientExamRoom]->Release();
		
			XRayCount[patientXRayRoom]++;
			waitingForXRayCV[patientXRayRoom]->Wait(waitingForXRayLock[patientXRayRoom]);
			
			//Interacting with XRay Technician
			waitingForXRayLock[patientXRayRoom]->Release();
			XRayRoomLock[patientXRayRoom]->Acquire();
			
			XRayExamSheetGlobal=patientExamSheet;
			
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
		}
			
			
		printf("Patient %d is going onto the Table for XRay Test\n",index);
			
		if(patientExamSheet.xray==1)
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
			
			patientExamSheet.image=imageGlobal;
			printf("Patient %d is done with XRay Image: %s\n",index,patientExamSheet.image);
			
			XRayRoomLock[patientXRayRoom]->Release();
				
			printf("Patient %d done with XRay\n",index);
			
			/*Waiting For Nurse to come
			XRayWaitingRoomLock[patientExamRoom]->Acquire();
			
			XRayWaitingRoomCV->Wait(XRayWaitingRoomLock[patientExamRoom]);
				
			//Going with Nurse back to his Exam Room
				
			XRayWaitingRoomLock[patientExamRoom]->Release();*/
		}
		else if(patientExamSheet.xray==2)
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
			
			printf("Patient %d moving after 1st Image\n",index);
				
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
				
			patientExamSheet.image=imageGlobal;
			printf("Patient %d is done with XRay Image\n",index);
				
			XRayRoomLock[patientXRayRoom]->Release();
				
			/*XRayWaitingRoomLock[patientXRayRoom]->Acquire();
				
			//Waiting For Nurse to come
			XRayWaitingRoomCV->Wait(XRayWaitingRoomLock[patientXRayRoom]);
				
			//Going with Nurse back to his Exam Room
				
			XRayWaitingRoomLock[patientXRayRoom]->Release();*/
		}
		else
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
			
			printf("Patient %d moving after 1st Image\n",index);
				
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
				
			printf("Patient %d moving after 2nd Image\n",index);
				
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
				
			patientExamSheet.image=imageGlobal;
			printf("Patient %d is done with XRay Image\n",index);
				
			XRayRoomLock[patientXRayRoom]->Release();
				
			/*XRayWaitingRoomLock[patientXRayRoom]->Acquire();
				
			//Waiting For Nurse to come
			XRayWaitingRoomCV->Wait(XRayWaitingRoomLock[patientXRayRoom]);
				
			//Going with Nurse back to his Exam Room
				
			XRayWaitingRoomLock[patientXRayRoom]->Release();*/
		}
	}
	
	
	//Temporary until another tasks start, freeing the Exam Room
	
	RoomLock[patientExamRoom]->Release();
			
	ExamRoomLock->Acquire();
	ExamRoomMV[patientExamRoom]=0;
	DEBUG('p',"Patient %d freed Room# %d\n",index, patientExamRoom);
	ExamRoomLock->Release();
	
	
}

void Nurse(int index)
{
	int NurseExamRoom;					// To associate an Exam room with Nurse
	int EscortPatient;					
	struct ExamSheet nurseExamSheet;
	int DoctorFree;
	int NurseXRayRoom;
	
	while(1)
	{
	printf("Nurse %d entered the clinic\n", index);
	DEBUG('p', "Nurse %d entered the clinic\n", index);
	
	nurseExamSheet.id=0;
	
	//---Task 1 begins:Getting patient from the queue and escort to the exam room---//
	
	ExamRoomLock->Acquire();
	
	
	//Check which waiting room is free--//
	ExamRoomCount=0;
	for(int i=1;i<6;i++)
	{
		if(ExamRoomMV[i]==0)
		{
			ExamRoomMV[i]=1;
			NurseExamRoom=i;
			break;
		}
		else
		{
			ExamRoomCount++;
		}
	   
	}
	ExamRoomLock->Release();
	
	//---------------------------Condtion that the room is available-----------------------//
	if(ExamRoomCount<5)
	{
		//--------------------Entering the queue to talk with WRN----------------------------//
		waitingRoomLock->Acquire();
		DEBUG('p', "Nurse %d acquired the waitingRoomLock\n", index);
		DEBUG('p', "WRN State : %d\n", wrnNurseState);
		if(wrnNurseState==1)		//WRN busy. Wait in line
		{
			nurseWaitingCount++;
			DEBUG('p',"Nurse %d got into queue\n", index);
			nurseWaitingCV->Wait(waitingRoomLock);
			DEBUG('p', "Nurse %d out of Wait and Nurse Waiting Count: %d\n",index, nurseWaitingCount);
		}
		else
		{
			//wrnNurseState="busy";
			wrnNurseState=1;
			DEBUG('p',"Nurse %d sees no queue\n", index);
		}
	
		waitingRoomLock->Release();
		DEBUG('p',"Nurse %d released waitingRoomLock\n", index);
		
	
		//----------------------------WRN<-->Nurse Interaction--------------------------
	
		/*------------Lock to get the patient form----------------*/
		wrnLock->Acquire();
	
		DEBUG('p', "Nurse %d acquired the wrnLock\n", index);
		DEBUG('p', "Nurse %d talking with WRN\n", index);
	
		//Nurse Signals WRN that she has a ExamRoom Free a nd waits if there are any patient waiting to be treated
		nurseWrnWaitingCV->Signal(wrnLock);
		DEBUG('p',"Nurse %d Signalled on nurseWrnWaitingCV using wrnLock\n", index);
		nurseWrnWaitingCV->Wait(wrnLock);
		DEBUG('p',"Nurse %d out of Wait on nurseWrnWaitingCV using wrnLock\n", index);

		printf("Nurse %d is receiving FORM from WRN.\n", index);
		DEBUG('n', "Nurse %d is receiving FORM from WRN.\n", index);
		
		//----------------Escort any Waiting Patient from Waiting Room--------------------------------//
		if(ExamSheetPatient.id!=0)
		{
			EscortPatient=1;
			nurseExamSheet=ExamSheetPatient;
			printf("Nurse %d got Patient %d's form from WRN\n",index, nurseExamSheet.id);
			DEBUG('p',"Nurse %d got Patient %d's form from WRN\n",index, nurseExamSheet.id);
			
			nurseWrnWaitingCV->Signal(wrnLock);
			DEBUG('p',"Nurse %d Signalled on nurseWrnWaitingCV using wrnLock after receiving Patient form\n", index);
			
			//NURSE-Patient Interaction
			registeredPatientLock-> Acquire();
			DEBUG('p',"Nurse %d acquired registeredPatientLock\n", index);
	
			patientAssignedRoomGlobal=NurseExamRoom;
			
			nurseRegisteredPatientCV->Signal(registeredPatientLock);
			DEBUG('p',"Nurse %d Signalled on nurseRegisteredPatientCV using registeredPatientLock\n", index);
			printf("Nurse %d calling Patient %d\n",index, nurseExamSheet.id);
	
			nurseRegisteredPatientCV->Wait(registeredPatientLock);
			DEBUG('p',"Nurse %d out of Wait on nurseRegisteredPatientCV using registeredPatientLock\n", index);
			printf("Nurse %d taking Patient %d to Exam Room %d\n",index,nurseExamSheet.id, NurseExamRoom);

			registeredPatientLock-> Release();
			DEBUG('p',"Nurse %d registeredPatientLock\n", index);
			wrnLock->Release();
			DEBUG('p',"Nurse %d released wrnLock\n", index);
			
			//Performing Symptom Check with Patient
			RoomLock[NurseExamRoom]->Acquire();
			DEBUG('p',"Nurse %d acquired RoomLock[%d]\n", index,NurseExamRoom);
			
			//Nurse enters the Exam Room to talk with patient
			printf("Nurse %d entered Room %d\n", index, NurseExamRoom);
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
			DEBUG('p',"Nurse %d Signalled on RoomLockCV[%d] using RoomLock[%d]\n", index, NurseExamRoom, NurseExamRoom);
			
			RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);
			DEBUG('p',"Nurse %d out of Wait on RoomLockCV[%d] using RoomLock[%d]\n", index,NurseExamRoom, NurseExamRoom);
			
			patientTemperature=60.6;
			patientBP=90;
			
			nurseExamSheet.temperature=patientTemperature;
			nurseExamSheet.BP=patientBP;
			
			//WallPocketExamSheet[NurseExamRoom]=nurseExamSheet;
			//Displaying Wall pocket;
			printf("Wall Pocket %d has following form\n%s\n%d\n%d\n%f\n%d\n", NurseExamRoom, nurseExamSheet.name,nurseExamSheet.age,nurseExamSheet.id,nurseExamSheet.temperature,nurseExamSheet.BP);
			
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
			DEBUG('p',"Nurse %d Signalled on RoomLockCV[%d] using RoomLock[%d]\n", index, NurseExamRoom, NurseExamRoom);
			
			RoomLock[NurseExamRoom]->Release();
		}
		else
		{
			EscortPatient=0;
			ExamRoomLock->Acquire();
			ExamRoomMV[NurseExamRoom]=0;
			ExamRoomLock->Release();
			printf("Nurse %d : There is no Form\n", index);
			nurseWrnWaitingCV->Signal(wrnLock);
			wrnLock->Release();
		}
	}//End of if
	
	//-------------------------------Nurse-patient Interaction ends---------------------//
	//Delay
	int r = 50;
	for(int i=0;i<r;i++)
	{
		currentThread->Yield();
	}
	//#------------------------------Nurse-Doctor Interaction--------------------------#
	if(nurseExamSheet.id!=0)
	{
		DoctorAvailableLock->Acquire();
		DEBUG('p',"Nurse %d acquired DoctorAvailableLock\n", index);
	
		//Check which Doctor is free--//
		doctorCount=0;
		for(int i=1;i<4;i++)
		{
			if(DoctorMV[i]==0)
			{
				DoctorMV[i]=1;
				DEBUG('p',"Nurse %d will call Doctor %d,\n", index, i);
				//DoctorFree=i;
				break;
			}
			else
			{
				doctorCount++;
			}
		}
		
		DoctorAvailableLock->Release();
		DEBUG('p',"Nurse %d released DoctorAvailableLock\n", index);
		
		doctorRoomLock->Acquire();
		DEBUG('p',"Nurse %d acquired the doctor room lock\n",index);
		
		printf("Doctor Count: %d\n",doctorCount);
		if(doctorCount<3)		//Doctor is Available
		{

			 doctorWaitingCV->Signal(doctorRoomLock);
			 DEBUG('p',"Nurse %d signalled doctor on doctor waitingCV using doctor Room lock\n",index);
			 //doctorCount--;
			 DEBUG('p',"Nurse %d decremented the doctorCount to %d\n", index, doctorCount);
			 
			
			 doctorNurseLock->Acquire();
			 DEBUG('p',"Nurse %d acquires doctorNurselock\n",index);
			 
			 doctorRoomLock->Release();
			 DEBUG('p',"Nurse %d releases doctor Room lock\n",index);
			 
			 DEBUG('p',"doctorCount is %d\n",doctorCount);
			 printf("Nurse %d Calling Doctor to Exam Room %d\n ",index, NurseExamRoom);
			 doctorNurseCV->Wait(doctorNurseLock);
			 DEBUG('p',"Nurse %d out of wait on doctorWaitingCV\n",index);

			 doctorExamRoomGlobal=NurseExamRoom;
			 
			 doctorNurseCV->Signal(doctorNurseLock);
			 
			 doctorNurseLock->Release();
			 DEBUG('p',"Nurse %d releases doctor Nurse lock\n",index);
		}
		else					//Doctor is not available
		{
			//Waiting for some Doctor to come.
			nurseCount++;
			nurseWaitingForDoctorCV->Wait(doctorRoomLock);
			
			//doctorCount--;
			
			doctorNurseLock->Acquire();
			DEBUG('p',"Nurse %d acquires doctorNurselock\n",index);
			 
			doctorRoomLock->Release();
			DEBUG('p',"Nurse %d releases doctor Room lock\n",index);
			
			doctorExamRoomGlobal=NurseExamRoom;
			
			printf("Nurse %d Calling Doctor to Exam Room %d\n ",index, NurseExamRoom);
			DEBUG('p',"Nurse %d releases doctor Room lock\n",index);
			
			nurseDoctorCV->Signal(doctorNurseLock);
			nurseDoctorCV->Wait(doctorNurseLock);
			
			doctorNurseLock->Release();
			
		}

	}		
	//------------------------------Nurse-Doctor Interaction ends--------------------------//
	//End of Task 1		
	//-------------------------------Task-2 Begins:Escort patient to XRay room-------------//
	
	Task2Lock->Acquire();
	int wallPocketCount=0;

	//Check each wallpocket
	for(int i=1;i<6;i++)
	{
		if(WallPocketExamSheet[i].id!=0)
		{
			NurseExamRoom=i;
			nurseExamSheet=WallPocketExamSheet[NurseExamRoom];
			
			printf("Nurse %d got WallPocket %d\n", index, nurseExamSheet.id);
			
			printf("Nurse %d is Going to Exam Room %d\n", index, NurseExamRoom);
			
			//Removing ExamSheet from WallPocket
			WallPocketExamSheet[NurseExamRoom].id=0;

			break;
					
		}
		else
		{
			wallPocketCount++;
			 
		}
	}
	
	Task2Lock->Release();
	
	//If got the Wall Pocket Exam Sheet
	if(wallPocketCount<5)
	{
		RoomLock[NurseExamRoom]->Acquire();
			
		//RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
		//RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);
		
		//Checking the number of XRAY/Shot/Cashier
		//Only XRay
		if(nurseExamSheet.xray==1 && nurseExamSheet.shot==0)
		{
			//Taking to XRay Room
			printf("Nurse %d is in Exam Room %d for escort Patient %d to XRay\n", index, NurseExamRoom, nurseExamSheet.id);
			
			//Checking which XRay Room to go
			XRayLock->Acquire();
			int XRayRoomCount=0;
			for(int i=1;i<3;i++)
			{
				if(XRayRoomMV[i]==0)
				{
					XRayRoomMV[i]=1;
					NurseXRayRoom=i;
					break;
				}
				else
				{
					XRayRoomCount++;
				}
			}
			
			XRayLock->Release();
			
			//Got 1 of the XRay Room
			if(XRayRoomCount<2)
			{	
				printf("Nurse %d taking Patient %d to XRay Room %d\n",index, nurseExamSheet.id,NurseXRayRoom);
				XRayRoomGlobal[NurseExamRoom]=NurseXRayRoom;
				waitInXRayLineGlobal[NurseExamRoom]=0;
				
				RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
				RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);
				
				
				//Giving XRay Technician the Exam Sheet
				DEBUG('x',"Nurse XRay Room: %d\n",NurseXRayRoom);
				XRayRoomLock[NurseXRayRoom]->Acquire();
				
				XRayExamSheetGlobal=nurseExamSheet;
				printf("Nurse %d giving Patient %d's Exam Sheet To XRay Technician%d\n",index, nurseExamSheet.id, NurseXRayRoom);
				
				XRayRoomNurseCV[NurseXRayRoom]->Signal(XRayRoomLock[NurseXRayRoom]);
				//printf("Nurse %d gave Patient %d's Exam Sheet To XRay Technician %d\n",index, nurseExamSheet.id, NurseXRayRoom);
				
				XRayRoomNurseCV[NurseXRayRoom]->Wait(XRayRoomLock[NurseXRayRoom]);
				//printf("Nurse %d out of wait from XRay Technician %d\n",index, NurseXRayRoom);
				
				XRayRoomLock[NurseXRayRoom]->Release();
				
				//Signalling the patient to go talk with XRayTechnician
				RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
				
			}
			else
			{
				//NurseXRayRoom= min(XRayCount1,XRayCount2);
				NurseXRayRoom=1;
				
				printf("Nurse %d taking Patient %d to XRay Room %d\n",index, nurseExamSheet.id,NurseXRayRoom);
				XRayRoomGlobal[NurseExamRoom]=NurseXRayRoom;
				waitInXRayLineGlobal[NurseExamRoom]=1;
				
				RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
				RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);
			}
				
			RoomLock[NurseExamRoom]->Release();
				
		}
		
		//Only Shot
		else if(nurseExamSheet.xray==0 && nurseExamSheet.shot==1)
		{
			printf("Nurse %d is in Exam Room %d for giving Patient %d a Shot\n", index, NurseExamRoom, nurseExamSheet.id);
			
			//Signalling Patient that she is going to get medicine
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
			RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);
			
			//Nurse going to take medicine for shot
			printf("Nurse %d going to get medicine for Patient %d\n", index, nurseExamSheet.id);
			
			SupplyCabinetLock->Acquire();
			if(SupplyCabinetState==0)
			{
				SupplyCabinetState=1;
				printf("Nurse %d is getting medicine for Patient %d\n",index, nurseExamSheet.id);
				SupplyCabinetState=0;
				
				//Check if some other nurse is Waiting
				if(SupplyCabinetCount>0)
				{
					SupplyCabinetCount--;
					SupplyCabinetCV->Signal(SupplyCabinetLock);
				}
				
				SupplyCabinetLock->Release();
			}
			else
			{
				SupplyCabinetCount++;
				SupplyCabinetCV->Wait(SupplyCabinetLock);
				
				SupplyCabinetState=1;
				printf("Nurse %d is getting medicine for Patient %d\n",index, nurseExamSheet.id);
				SupplyCabinetState=0;
				
				//Check if some other nurse is Waiting
				if(SupplyCabinetCount>0)
				{
					SupplyCabinetCount--;
					SupplyCabinetCV->Signal(SupplyCabinetLock);
				}
				
				SupplyCabinetLock->Release();
			}
			//Signalling Patient that Shot is ready
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
			
			//Waiting for Patient to get ready for Shot
			RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);
			
			printf("Nurse %d giving Shot to Patient %d\n",index, nurseExamSheet.id);
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
			
			RoomLock[NurseExamRoom]->Release();
		}
		
		//Both XRAY && Shot
		else if(nurseExamSheet.xray==1 && nurseExamSheet.shot=1)
		{
			//1st Taking Shot
			printf("Nurse %d is in Exam Room %d for giving Patient %d a Shot\n", index, NurseExamRoom, nurseExamSheet.id);
			
			//Signalling Patient that she is going to get medicine
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
			RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);
			
			//Nurse going to take medicine for shot
			printf("Nurse %d going to get medicine for Patient %d\n", index, nurseExamSheet.id);
			
			SupplyCabinetLock->Acquire();
			if(SupplyCabinetState==0)
			{
				SupplyCabinetState=1;
				printf("Nurse %d is getting medicine for Patient %d\n",index, nurseExamSheet.id);
				SupplyCabinetState=0;
				
				//Check if some other nurse is Waiting
				if(SupplyCabinetCount>0)
				{
					SupplyCabinetCount--;
					SupplyCabinetCV->Signal(SupplyCabinetLock);
				}
				
				SupplyCabinetLock->Release();
			}
			else
			{
				SupplyCabinetCount++;
				SupplyCabinetCV->Wait(SupplyCabinetLock);
				
				SupplyCabinetState=1;
				printf("Nurse %d is getting medicine for Patient %d\n",index, nurseExamSheet.id);
				SupplyCabinetState=0;
				
				//Check if some other nurse is Waiting
				if(SupplyCabinetCount>0)
				{
					SupplyCabinetCount--;
					SupplyCabinetCV->Signal(SupplyCabinetLock);
				}
				
				SupplyCabinetLock->Release();
			}
			//Signalling Patient that Shot is ready
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
			
			//Waiting for Patient to get ready for Shot
			RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);
			
			printf("Nurse %d giving Shot to Patient %d\n",index, nurseExamSheet.id);
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
			
			//Done with giving Shot
			//RoomLock[NurseExamRoom]->Release();
			
			//2nd Go for XRay
			//Taking to XRay Room
			printf("Nurse %d is in Exam Room %d for escort Patient %d to XRay\n", index, NurseExamRoom, nurseExamSheet.id);
			
			//Checking which XRay Room to go
			XRayLock->Acquire();
			int XRayRoomCount=0;
			for(int i=1;i<3;i++)
			{
				if(XRayRoomMV[i]==0)
				{
					XRayRoomMV[i]=1;
					NurseXRayRoom=i;
					break;
				}
				else
				{
					XRayRoomCount++;
				}
			}
			
			XRayLock->Release();
			
			//Got 1 of the XRay Room
			if(XRayRoomCount<2)
			{	
				printf("Nurse %d taking Patient %d to XRay Room %d\n",index, nurseExamSheet.id,NurseXRayRoom);
				XRayRoomGlobal[NurseExamRoom]=NurseXRayRoom;
				waitInXRayLineGlobal[NurseExamRoom]=0;
				
				RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
				RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);
				
				
				//Giving XRay Technician the Exam Sheet
				DEBUG('x',"Nurse XRay Room: %d\n",NurseXRayRoom);
				XRayRoomLock[NurseXRayRoom]->Acquire();
				
				XRayExamSheetGlobal=nurseExamSheet;
				printf("Nurse %d giving Patient %d's Exam Sheet To XRay Technician%d\n",index, nurseExamSheet.id, NurseXRayRoom);
				
				XRayRoomNurseCV[NurseXRayRoom]->Signal(XRayRoomLock[NurseXRayRoom]);
				//printf("Nurse %d gave Patient %d's Exam Sheet To XRay Technician %d\n",index, nurseExamSheet.id, NurseXRayRoom);
				
				XRayRoomNurseCV[NurseXRayRoom]->Wait(XRayRoomLock[NurseXRayRoom]);
				//printf("Nurse %d out of wait from XRay Technician %d\n",index, NurseXRayRoom);
				
				XRayRoomLock[NurseXRayRoom]->Release();
				
				//Signalling the patient to go talk with XRayTechnician
				RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
				
			}
			else
			{
				//NurseXRayRoom= min(XRayCount1,XRayCount2);
				NurseXRayRoom=1;
				
				printf("Nurse %d taking Patient %d to XRay Room %d\n",index, nurseExamSheet.id,NurseXRayRoom);
				XRayRoomGlobal[NurseExamRoom]=NurseXRayRoom;
				waitInXRayLineGlobal[NurseExamRoom]=1;
				
				RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
				RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);
			}
				
			RoomLock[NurseExamRoom]->Release();
		}
		
		//Nothing To do. Patient Has to Leave
		else
		{
			//Nurse Letting patient go to Cashier
			RoomLock[NurseExamRoom]->Release();
		}
		*/	
	}
	//----------------------------------Task-2 End--------------------------------//
	
	}//End of While
}

void Doctor(int index)
{

	int doctorExamRoom; 
	struct  ExamSheet doctorExamSheet;
    while(1)
	{//-------------------------Doctor-Nurse Interaction--------------------------//
		
		//Setting him/her self free
		DoctorAvailableLock->Acquire();
			DoctorMV[index]=0;
		DoctorAvailableLock->Release();
		
		doctorRoomLock->Acquire();
		DEBUG('p',"Doctor %d acquired doctorRoomLock\n", index);
		
		
		
		if(nurseCount==0)					//Waiting for nurse
		{
			doctorWaitingCV->Wait(doctorRoomLock);
			DEBUG('p',"Doctor %d is out of doctorWaitingCV Wait\n", index);
			
			doctorNurseLock->Acquire();
			DEBUG('p',"Doctor %d acquired doctorNurseLock \n", index);
			
			doctorRoomLock->Release();
			DEBUG('p',"Doctor %d released doctorRoomLock \n", index);
	
			doctorNurseCV->Signal(doctorNurseLock);
			DEBUG('p',"Doctor %d signalled on doctorWaitingCV \n", index);
			
			doctorNurseCV->Wait(doctorNurseLock);
			
			doctorExamRoom=doctorExamRoomGlobal;
		
			//Going to exam room
	
			printf("Doctor %d is going to exam room %d\n", index, doctorExamRoom);
			doctorNurseLock->Release();
			DEBUG('p',"Doctor %d releases doctor Nurse lock\n",index);
		}
		else								//Signal the Nurse which is Waiting
		{
			nurseCount--;
			
			//Setting him/her self busy
			DoctorAvailableLock->Acquire();
				DoctorMV[index]=1;
			DoctorAvailableLock->Release();
			
			nurseWaitingForDoctorCV->Signal(doctorRoomLock);
			
			doctorNurseLock->Acquire();
			DEBUG('p',"Nurse %d acquires doctorNurselock\n",index);
			 
			doctorRoomLock->Release();
			DEBUG('p',"Nurse %d releases doctor Room lock\n",index);
			
			nurseDoctorCV->Wait(doctorNurseLock);
			doctorExamRoom = doctorExamRoomGlobal;
			
			printf("Doctor %d is going to exam room %d\n", index, doctorExamRoom);
			
			nurseDoctorCV->Signal(doctorNurseLock);
			
			doctorNurseLock->Release();
		}
			
		//-------------------Doctor-Nurse Interaction End--------------------------------//
		
		//-------------------Doctor-Patient Interaction End--------------------------------//
		
		RoomLock[doctorExamRoom]->Acquire();
		DEBUG('p',"Doctor %d acquired RoomLock %d\n",index, doctorExamRoom);
		
		RoomLockCV[doctorExamRoom]->Signal(RoomLock[doctorExamRoom]);
		printf("Doctor %d is in the Exam Room %d\n", index, doctorExamRoom);
		
		RoomLockCV[doctorExamRoom]->Wait(RoomLock[doctorExamRoom]);
		doctorExamSheet=patientExamSheetGlobal;
		
		printf("Doctor %d got Patient %d's form\n",index, doctorExamSheet.id);

		doctorExamSheet.xray=0;
		doctorExamSheet.shot=1;
		
		patientExamSheetGlobal=doctorExamSheet;
		
		RoomLockCV[doctorExamRoom]->Signal(RoomLock[doctorExamRoom]);
		
		printf("Doctor %d Tested and Done with Patient %d\n", index,doctorExamSheet.id);
		
		
		
		RoomLock[doctorExamRoom]->Release();
		
		printf("Doctor %d is free\n", index);
		
		//--------------------------Doctor<->patient Interaction done------------------------//
		
		
		//-------------------------Doctor putting on Wall Pocket----------------------------#
		Task2Lock->Acquire();
		
		//Put on Wallpocket
		WallPocketExamSheet[doctorExamRoom]=doctorExamSheet;
		printf("Doctor %d kept Exam sheet %d in WallPocket %d.\n", index, doctorExamSheet.id, doctorExamRoom);
		Task2Lock->Release();	
		
	}//End of while
	
}

void XRayTechnician(int index)
{
	DEBUG('p',"XT %d entered Xray Room\n", index);
	struct ExamSheet XRayExamSheet;
	
	while(1)
	{
		waitingForXRayLock[index]->Acquire();
		DEBUG('x',"XT %d  acquired waitingForXRayLock\n", index);
		if(XRayCount[index]>0)			//Patient Waiting in Line
		{
			XRayCount[index]--;
		
			XRayRoomLock[index]->Acquire();
			DEBUG('x',"XT %d acquired XRayRoomLock\n", index);
			
			waitingForXRayCV[index]->Signal(waitingForXRayLock[index]);
			DEBUG('x',"XT %d Signalled on waitingForXRayCV\n", index);
			
			waitingForXRayLock[index]->Release();
			DEBUG('x',"XT %d released waitingForXRayLock\n", index);
			
			//Waiting for Patient to talk
			XRayRoomCV[index]->Wait(XRayRoomLock[index]);
			DEBUG('x',"XT %d OUT OF wait on XRayRoomCV using XRayRoomLock\n", index);
			
			//Take Examsheet From patient
			XRayExamSheet=XRayExamSheetGlobal;
			printf("XRay Technician %d got Exam Sheet %d from Patient\n",index, XRayExamSheet.id);
			printf("Xray Technician %d telling Patient %d to move onto the table\n",index, XRayExamSheet.id);
			
			XRayRoomCV[index]->Signal(XRayRoomLock[index]);
			DEBUG('x',"XT %d Signalled on XRayRoomCV using XRayRoomLock\n", index);
			
			XRayRoomCV[index]->Wait(XRayRoomLock[index]);
			//waitingForXRayCV[index]->Wait(waitingForXRayLock[index]);
			DEBUG('x',"XT %d Signalled on waitingForXRayCV\n", index);
	
			printf("XRayTechnician %d taking XRay of Patient %d\n",index, XRayExamSheet.id);
		
			imageGlobal="Fracture";
		
			XRayRoomCV[index]->Signal(XRayRoomLock[index]);
			printf("XRayTechnician: %d done with Xray of Patient %d\n",index, XRayExamSheet.id);
	
			XRayRoomLock[index]->Release();
			DEBUG('x',"XT %d Released XRayRoomLock\n", index);
		}
		else
		{
			XRayRoomLock[index]->Acquire();
			DEBUG('x',"XT %d acquired XRayRoomLock\n", index);
			
			waitingForXRayLock[index]->Release();
			DEBUG('x',"XT %d released waitingForXRayLock\n" , index);
			
			//Freeing Himself
			XRayLock->Acquire();
				XRayRoomMV[index]=0;
			XRayLock->Release();
		
			DEBUG('x',"XT %d is Free\n",index);
			//Waiting for Nurse to come
			XRayRoomNurseCV[index]->Wait(XRayRoomLock[index]);
			
			//XRayRoomNurseCV[1]->Wait(XRayRoomLock[1]);
			DEBUG('x',"XT %d Out of Wait\n" , index);
			printf("XRayTechnician %d is out of Wait\n",index);
			
			//Take Exam Sheet from Nurse
			XRayExamSheet=XRayExamSheetGlobal;
			printf("XRay Technician %d got Exam Sheet %d from Nurse\n",index, XRayExamSheet.id);
		
			XRayRoomNurseCV[index]->Signal(XRayRoomLock[index]);
			//XRayTechnician-Nurse Interaction ends
			
			//XRayTechnician-Patient Interaction starts
			XRayRoomCV[index]->Wait(XRayRoomLock[index]);
			printf("Xray Technician %d telling Patient %d to move onto the table\n",index, XRayExamSheet.id);
			
			XRayRoomCV[index]->Signal(XRayRoomLock[index]);
			XRayRoomCV[index]->Wait(XRayRoomLock[index]);
	
			printf("XRayTechnician %d taking XRay of Patient %d\n",index, XRayExamSheet.id);
		
			imageGlobal="Fracture";
		
			XRayRoomCV[index]->Signal(XRayRoomLock[index]);
			printf("XRayTechnician: %d done with Xray of Patient %d\n",index, XRayExamSheet.id);
	
			XRayRoomLock[index]->Release();
			DEBUG('x',"XT %d Released XRayRoomLock\n", index);
		}
		
	}//End of While
	
}		
		
			

void DoctorSimulation()
{
	printf("\n\nEntering the simulation\n");
	DEBUG('ds',"Entering the simulation\n");
	
	//WallPocketExamSheet
	for(int i=1; i<6; i++)
	{
		WallPocketExamSheet[i].id=0;
		RoomLock[i] = new Lock(" ");
		RoomLockCV[i]=new Condition(" ");
	}
	
	for(int i=1; i<3;i++)
	{
		XRayRoomLock[i] = new Lock(" ");
		XRayRoomCV[i]=new Condition(" ");
		XRayRoomNurseCV[i]=new Condition(" ");
		waitingForXRayLock[i] = new Lock(" ");
		waitingForXRayCV[i]=new Condition(" ");
	}
	
	
	Thread *wrnT=new Thread(" ");
	wrnT->Fork(waitingRoomNurse,0);
	
	for(int i=1;i<7;i++)
	{
		Thread *t=new Thread(" ");
		t->Fork(Patient,i);
	}
	for(int i=1;i<4;i++)
	{
		Thread *dt=new Thread(" ");
		dt->Fork(Doctor,i);
	}
	for(int i=1;i<6;i++)
	{
		Thread *nt=new Thread(" ");
		nt->Fork(Nurse,i);
	}
	
	/*for(int i=11;i<21;i++)
	{
		Thread *t=new Thread(" ");
		t->Fork(Patient,i);
	}*/
	
	for(int i=1;i<3;i++)
	{
		Thread *xt=new Thread(" ");
		xt->Fork(XRayTechnician,i);
	}
	
	
}

