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

	char* pain;
	char* nausea;
	char* hear_alien_voices;

	int temperature;
	int BP;

	int xray;
	int imageCount;
	int shot;

	char* image;

	int visit;
	int child;
};


//Data for Array of Structures for XRayWallPocket
struct ExamSheet XRayWallPocket1[200];
struct ExamSheet XRayWallPocket2[200];
int first1=0, first2=0;
int last1=-1,last2=-1;

//Queue for Doctor
int DoctorQueue[200];
int firstDoctor=0;
int lastDoctor=-1;

//Different Flags used for Test Cases
int flag1=0;
int flag2=0;
int flag3=0;
int flag4=0;
int flag5=0;
int flag6=0;
int flag7=0;

int patients;
int parents;
int nurses;
int doctors;
int xray;
int examrooms;
int input;
int child;
	
char *symptoms[] = {"true", "false"};
char *xrayResult[]= {"nothing", "break", "compound fracture"};
char *imageGlobal[3]; 

int ExamRoomMV[]={0,0,0,0,0,0};   			//0 for free and 1 for occupied
int DoctorMV[]={0,0,0,0};					//0 for free and 1 for occupied
int XRayRoomMV[]={0,0,0};					//0 for free and 1 for occupied
int XRayPatientEnter[]={0,0,0};
int PatientXRayEnter[]={0,0,0};

int n;
int parentEnter[100];
int childEnter[100];
int patientWaitingCount = 0;				
int wrnState=0;
int wrnNurseState=0;							
int decision;
int patientIDGlobal;
int nurseIDGlobal;
int doctorIDGlobal[6];
int nurseIDExamRoom;
int childDoctor[100];
int childRoom[100];
int childXRayRoom[100];

int nurseWaitingCount = 0;
int patientAssignedRoomGlobal=0;
int patientBP;
int doctorExamRoomGlobal[4];
int doctorCount=0;
int nurseCount=0;
int doctorWaitCount[]={0,0,0,0,0,0};
int XRayCount[]={0,0,0};
int waitInXRayLineGlobal[6];
int XRayRoomGlobal[6];
int patientNurseExamRoomGlobal[100];
int SupplyCabinetCount=0;
int SupplyCabinetState=0;
int patientWaitingForCashierCount=0;
int doctorIDNurseGlobal;
int cashierState=0;
int patientSize1=0;
int parent;

float feesGlobal;

struct ExamSheet emptyExamSheet;			//For Empty form exchange betn WRN and Patient
struct ExamSheet WrnFromPatientExamSheet;	//Patient Submitting to WRN
struct ExamSheet WrnToNurseExamSheet;		//WRN giving Pateint's Form to Nurse
struct ExamSheet WallPocketExamSheet[6];	//Nurse hangs on Exam Room Wall
struct ExamSheet patientExamSheetGlobal[6]; //Betn Doctor-Patient
struct ExamSheet NurseToPatientExamSheet[6];
struct ExamSheet XRayExamSheetGlobal[3];
struct ExamSheet cashierExamSheetGlobal;
struct ExamSheet ParentToChildExamSheet[100];

Lock* waitingRoomLock=new Lock("waitingRoomLock");
Lock* wrnLock=new Lock("wrnLock");
Lock* ExamRoomLock=new Lock("ExamRoomLock");
Lock* registeredPatientLock=new Lock("registeredPatientLock");
Lock* RoomLock[6];
Lock* doctorRoomLock=new Lock("doctorRoomLock");
Lock* doctorNurseLock[4];	
Lock* DoctorAvailableLock=new Lock("DoctorAvailableLock");
Lock* Task2Lock=new Lock("Task2Lock");
Lock* XRayLock=new Lock("XRayLock");
Lock* XRayRoomLock[3];
Lock* waitingForXRayLock[3];
Lock* XRayWaitingRoomLock[3];
Lock* SupplyCabinetLock=new Lock("SupplyCabinetLock");
Lock* Task3Lock=new Lock("Task3Lock");
Lock* nurseXRayPatientLock[100];
Lock* cashCounterLock=new Lock(" ");
Lock* cashierLock=new Lock(" ");
Lock* childPatientLock[100];

Condition* childPatientCV[100];
Condition* patientWaitingCV=new Condition("patientWaitingCV");
Condition* wrnWaitingCV=new Condition("wrnWaitingCV");
Condition* nurseWrnWaitingCV=new Condition("nurseWrnWaitingCV");
Condition* nurseWaitingCV=new Condition("nurseWaitingCV");
Condition* nurseRegisteredPatientCV=new Condition("nurseRegisteredPatientCV");
Condition* RoomLockCV[6];
Condition* doctorWaitingCV=new Condition("doctorWaitingCV");
Condition* doctorNurseCV[4];
Condition* nurseWaitingForDoctorCV=new Condition("nurseWaitingForDoctorCV");
Condition* task2CV=new Condition("task2CV");
Condition* XRayRoomCV[3];
Condition* XRayRoomNurseCV[3];
Condition* waitingForXRayCV[3];
Condition* XRayWaitingRoomCV[3];
Condition* SupplyCabinetCV=new Condition("SupplyCabinetCV");
Condition* nurseXRayPatientCV[100];
Condition* cashCounterCV=new Condition(" ");
Condition* cashierCV=new Condition(" ");



//====================================Waiting Room Nurse Thread====================================//
void waitingRoomNurse(int index)
{
	struct ExamSheet ExamSheetQueue[200];
	int first=0, last=-1;
	
	struct ExamSheet patientExamSheet;
	int patientSize;

	while(1)
	{

	if(patientSize<n)
	{
		waitingRoomLock->Acquire();

		if(patientWaitingCount>0)	//Patients are in line
		{
			//Waking the patient
			patientWaitingCV->Signal(waitingRoomLock);
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

		waitingRoomLock->Release();

		//#--------------------WRN<-->Patient Interaction---------------------------------#

		wrnWaitingCV->Wait(wrnLock);

		//Decide if task is either to GETFORM or GIVEFORM
		if(parent==0){
		if(decision == 1)			// GETFORM
		{
		
			
			int patientID= patientIDGlobal;
			
			if(flag2==1)
			printf("Waiting Room nurse gives a form to Adult Patient %d\n",patientID);

			//Signal the Patient to take the Empty Form and Wait
			wrnWaitingCV->Signal(wrnLock);
			wrnWaitingCV->Wait(wrnLock);
			
		}
		else if(decision == 2)		//GIVEFORM
		{
			//Signal the Patient to give the Filled Form and Wait
			wrnWaitingCV->Signal(wrnLock);
			wrnWaitingCV->Wait(wrnLock);

			//Take the copy from patient and keep to itself
			patientExamSheet=WrnFromPatientExamSheet;
			
			if(flag2==1)
			printf("Waiting Room nurse accepts the form from the Adult Patient with Name: Adult Patient %d and Age: %d\n",patientExamSheet.id, patientExamSheet.age);
		
			//Appending the sheet to the ExamSheetQueue
			if(flag2==1)
			printf("Waiting Room nurse creates an examination sheet for Adult Patient with Name: Adult Patient %d and Age: %d\n", patientExamSheet.id, patientExamSheet.age);
			last++;
			ExamSheetQueue[last]= patientExamSheet;
			if(flag2==1)
			printf("Waiting Room nurse tells the Adult Patient %d to wait in the waiting room for a nurse\n",patientExamSheet.id);
			
			//Checking 
			patientSize++;

		}
		}//End of If for Parent==0
		
		else{
		if(decision == 1)			// GETFORM
		{
			//Make a new exam sheet
			//emptyExamSheet;
			int patientID= patientIDGlobal;

			if(flag2==1 || flag1==1)
			printf("Waiting Room nurse gives a form to Parent of Child Patient %d\n",patientID);
			DEBUG('p', " WRN Giving form to the patient\n");

			//Signal the Patient to take the Empty Form and Wait
			wrnWaitingCV->Signal(wrnLock);
			wrnWaitingCV->Wait(wrnLock);

		}
		else if(decision == 2)		//GIVEFORM
		{
			//Signal the Patient to give the Filled Form and Wait
			wrnWaitingCV->Signal(wrnLock);
			wrnWaitingCV->Wait(wrnLock);

			//Take the copy from patient and keep to itself
			patientExamSheet=WrnFromPatientExamSheet;

			if(flag2==1 || flag1==1)
			printf("Waiting Room nurse accepts the form from the Parent with Name: Child Patient %d and Age: %d\n",patientExamSheet.id, patientExamSheet.age);
			
			//Appending the sheet to the ExamSheetQueue
			if(flag2==1 || flag1==1)
			printf("Waiting Room nurse creates an examination sheet for Child Patient with Name: Child Patient %d and Age: %d\n", patientExamSheet.id, patientExamSheet.age);
			last++;
			ExamSheetQueue[last]= patientExamSheet;
			
			if(flag2==1 || flag1==1)
			printf("Waiting Room nurse tells the Parent %d to wait in the waiting room for a nurse\n",patientExamSheet.id);
		
			patientSize++;

		}
		}//End of Else
		
		wrnLock->Release();
		
	}//End of IF for checking Exit criteria

	//===========================Patient Task interaction is over=============================//


	//=============================Nurse-WRN Interaction Begins===============================//

		//Again acquire waitingRoomLock to interact with Nurse
		waitingRoomLock->Acquire();
		
		if(nurseWaitingCount>0)
		{
			nurseWaitingCV->Signal(waitingRoomLock);
			nurseWaitingCount--;
			wrnNurseState=1;				// WRN is BUSY
		}
		else
		{
			wrnNurseState=0;				// WRN is FREE
		}

		wrnLock->Acquire();
		waitingRoomLock->Release();
		
		//Waiting to see if Nurse Signals for an empty ExamRoom
		nurseWrnWaitingCV->Wait(wrnLock);

		int nurseID=nurseIDGlobal;

		if(!(last<first))			//Queue is not Empty
		{
			//WrnToNurseExamSheet=*((ExamSheet*)ExamSheetQueue->Remove());
			WrnToNurseExamSheet=ExamSheetQueue[first];
			first++;

			//Giving first form to the nurse--//
			if(WrnToNurseExamSheet.child==0)
			{
				if(flag7==1)
				printf("Waiting Room nurse gives examination sheet of Adult Patient %d to Nurse %d\n",WrnToNurseExamSheet.id, nurseID);
			}
			else
			{	
				if(flag7==1)
				printf("Waiting Room nurse gives examination sheet of Child Patient %d to Nurse %d\n",WrnToNurseExamSheet.id, nurseID);
			}
			
			nurseWrnWaitingCV->Signal(wrnLock);
			nurseWrnWaitingCV->Wait(wrnLock);
		
		}
		else
		{
			//WrnToNurseExamSheet=NULL;
			WrnToNurseExamSheet.id=0;
			nurseWrnWaitingCV->Signal(wrnLock);
			
			nurseWrnWaitingCV->Wait(wrnLock);
		}

		wrnLock->Release();
		

		//=====================Nurse-WRN Interaction ends==================================//

	}//End of while
}//End of Waiting Room Nurse Thread


//==========================================Patient Thread==========================================//
void Patient(int index)
{

	int patientExamRoom; 				
	struct ExamSheet patientExamSheet;
	int nurseID;
	int doctorID;
	int patientXRayRoom;
	int waitInXRayLine;
	float fees;

	if(flag2==1)
	printf("Adult Patient %d has entered the Doctor's Office Waiting Room\n", index);
	
	//--------------------Entering the queue to get the form----------------------------
	waitingRoomLock->Acquire();

	if(wrnState==1)		//WRN busy. Wait in line
	{
		patientWaitingCount++;
		
		if(flag2==1 || flag5==1)
		printf("Adult Patient %d gets in the line of Waiting Room Nurse to get registration form\n",index);

		patientWaitingCV->Wait(waitingRoomLock);
	}
	else
	{
		//wrnState="busy";
		wrnState=1;
	}

	waitingRoomLock->Release();
	

	//---------------------------WRN <--> Patient Interaction-------------------------------------//

	//---------------------------------GETFORM----------------------------------------------------//
	
	//Acquire Lock to get the form
	wrnLock->Acquire();

	//Get the form from WRN
	//wrnTask="getForm";
	decision = 1;
	parent=0;		//Adult Patient
	patientIDGlobal = index;

	//Patient Signal's WRN that he needs an empty form and goes to wait
	wrnWaitingCV->Signal(wrnLock);
	wrnWaitingCV->Wait(wrnLock);
	
	//Patient takes empty form from WRN
	patientExamSheet=emptyExamSheet;

	patientExamSheet.visit=0;			//Visit 1
	patientExamSheet.child=0;			//Adult Patient
	patientExamSheet.xray=0;
	patientExamSheet.shot=0;

	if(flag2==1 || flag5==1)
	printf("Adult Patient %d gets the form from the Waiting Room Nurse\n", index);

	wrnWaitingCV->Signal(wrnLock);

	wrnLock->Release();
	
	//Done with getForm

	//----------------------------------------GIVEFORM-----------------------------------------//
	
	//--------------------------------Entering the queue to submit the form--------------------//
	waitingRoomLock->Acquire();

	if(wrnState==1)		//WRN busy. Wait in line
	{
		patientWaitingCount++;
		
		if(flag2==1 || flag5==1)
		printf("Adult Patient %d gets in the line of Waiting Room Nurse to submit registration form\n",index);
		
		patientWaitingCV->Wait(waitingRoomLock);
	}
	else
	{
		//wrnState="busy";
		wrnState=1;
	}

	waitingRoomLock->Release();
	

	//Acquire Lock to give the form
	wrnLock->Acquire();
	
	//Give the form to WRN
	//wrnTask="giveForm";
	decision = 2;
	parent=0;	
	
	//Filling the ExamRoomSheet
	patientExamSheet.age=(20 + rand()%50);
	patientExamSheet.id=index;

	WrnFromPatientExamSheet=patientExamSheet;

	//Patient Signals WRN that he wants to submit filled form and Waits
	wrnWaitingCV->Signal(wrnLock);
	
	wrnWaitingCV->Wait(wrnLock);

	if(flag2==1 || flag5==1)
	printf("Adult Patient %d submits the filled form from the Waiting Room Nurse\n", index);

	wrnWaitingCV->Signal(wrnLock);

	wrnLock->Release();
	
	//Done with giveForm and waiting for the Nurse
	
	//---------------------------Done with WRN-Patient Interaction----------------------------//


	//--------------------------------Nurse-Patient-Interaction-------------------------------//
	
	//Patient going with nurse to Exam Room
	registeredPatientLock->Acquire();

	nurseRegisteredPatientCV->Wait(registeredPatientLock);

	patientExamRoom=patientAssignedRoomGlobal;
	nurseID=nurseIDExamRoom;

	if( flag4==1 || flag5==1)
	printf("Adult Patient %d is following Nurse %d to Examination Room %d\n",index, nurseID, patientExamRoom);

	nurseRegisteredPatientCV->Signal(registeredPatientLock);

	RoomLock[patientExamRoom]->Acquire();

	registeredPatientLock->Release();

	if( flag4==1 || flag5==1)
	printf("Adult Patient %d arrived at Examination Room %d\n",index, patientExamRoom);

	//Performing symptoms check with Nurse
	//Waiting for nurse to come in the Exam Room and check my symptoms
	
	RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

	//Patient Signals Nurse to test Temperature and BP and goes to Wait
	RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);

	RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

	patientExamSheet=NurseToPatientExamSheet[patientExamRoom];

	patientExamSheet.pain=symptoms[rand()%2];
	patientExamSheet.nausea=symptoms[rand()%2];
	patientExamSheet.hear_alien_voices=symptoms[rand()%2];

	if(flag4==1 || flag5==1)
	printf("Adult Patient %d says, \"My Symptoms are Pain: %s, Nausea: %s, Hear Alien Voices: %s \"\n",index, patientExamSheet.pain, patientExamSheet.nausea, patientExamSheet.hear_alien_voices);

	RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);

	//--------------------------Patient done with Symptoms Check-------------------------------//

	//------------------------Patient is waiting for Doctor's Visit----------------------------//

	RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

	patientExamSheetGlobal[patientExamRoom]=patientExamSheet;

	RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
	RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
	
	RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);

	patientExamSheet=patientExamSheetGlobal[patientExamRoom];
	doctorID=doctorIDGlobal[patientExamRoom];

	if(patientExamSheet.xray==1 && patientExamSheet.shot==0)
	{
		if(flag6==1 || flag5==1)
		printf("Adult Patient %d has been informed by doctor %d that he needs an XRay\n", index, doctorID);
	}
	else if(patientExamSheet.xray==0 && patientExamSheet.shot==1)
	{
		if(flag6==1 || flag5==1)
		printf("Adult Patient %d has been informed by doctor %d that he will be administered a Shot\n", index, doctorID);
	}
	else if(patientExamSheet.xray==1 && patientExamSheet.shot==1)
	{
		if(flag6==1 || flag5==1)
		printf("Adult Patient %d has been informed by doctor %d that he needs an XRay\n", index, doctorID);
		if(flag6==1 || flag5==1)
		printf("Adult Patient %d has been informed by doctor %d that he will be administered a Shot\n", index, doctorID);
	}
	else
	{
	}

	//--------------------------Patient-Xray Begins------------------------------------//
	//Waiting for Nurse to come
	//int patientVisit1Done[patientExamRoom]=1;
	
	if(patientExamSheet.xray==1 && patientExamSheet.shot==0)
	{
		if(flag4==1 || flag5==1)
		printf("Adult Patient %d waits for a Nurse to Escort him/her to Xray room\n",index);
	}
	//printf("Adult Patient %d waits for a Nurse to Escort him/her to Xray room\n",index);
	RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

	//Only XRay
	if(patientExamSheet.xray==1 && patientExamSheet.shot==0)
	{
		patientXRayRoom=XRayRoomGlobal[patientExamRoom];
		waitInXRayLine=waitInXRayLineGlobal[patientExamRoom];

		if(waitInXRayLine==0)					//Not to Wait
		{
			RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
			RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
			
			//Nurse passed the necessary information
			//Now talking with XRay Technician
			
			XRayRoomLock[patientXRayRoom]->Acquire();

			//Signalling the XRayTechnician
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

		}
		else									//Need to Wait
		{
			RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
			RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
			
			//Patient done with Nurse to escort to Xray Room

			//Waiting in Line for XRay
			waitingForXRayLock[patientXRayRoom]->Acquire();

			//Flags
			PatientXRayEnter[patientXRayRoom]++;	//Enters 
			
			if(XRayPatientEnter[patientXRayRoom]==0)
			{
				//XRayCount[patientXRayRoom]++;
				printf("XRayCount %d is %d\n",patientXRayRoom, XRayCount[patientXRayRoom]);
				waitingForXRayCV[patientXRayRoom]->Wait(waitingForXRayLock[patientXRayRoom]);
				XRayPatientEnter[patientXRayRoom]=0;
				waitingForXRayCV[patientXRayRoom]->Signal(waitingForXRayLock[patientXRayRoom]);
			}
			else
			{
				XRayPatientEnter[patientXRayRoom]=0;
				waitingForXRayCV[patientXRayRoom]->Signal(waitingForXRayLock[patientXRayRoom]);
			}
			
			waitingForXRayCV[patientXRayRoom]->Wait(waitingForXRayLock[patientXRayRoom]);
			//Interacting with XRay Technician
			waitingForXRayLock[patientXRayRoom]->Release();
			
			XRayRoomLock[patientXRayRoom]->Acquire();

			XRayExamSheetGlobal[patientXRayRoom]=patientExamSheet;

			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
		}

		if(flag7==1 || flag5==1)
		printf("Adult Patient %d gets on the Table\n",index);
		if(flag7==1 || flag5==1)
		printf("Adult Patient %d Image Count: %d", index, patientExamSheet.imageCount);
		if(patientExamSheet.imageCount==1)
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//patientExamSheet.image=imageGlobal[patientXRayRoom];

		}
		else if(patientExamSheet.imageCount==2)
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//printf("Patient %d moving after 1st Image\n",index);

			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//patientExamSheet.image=imageGlobal[patientXRayRoom];

		}
		else if(patientExamSheet.imageCount==3)
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//printf("Patient %d moving after 1st Image\n",index);

			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//printf("Patient %d moving after 2nd Image\n",index);

			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//patientExamSheet.image=imageGlobal[patientXRayRoom];

		}
		else
		{}

		//----------Waiting in XRay waiting room for nurse to come----------//
		
		XRayWaitingRoomLock[patientXRayRoom]->Acquire();
		
		//Signals the XRay Technician to hang his sheet on Wall Pocket
		XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
		
		XRayRoomLock[patientXRayRoom]->Release();
		
		//Waits for the Nurse 
		XRayWaitingRoomCV[patientXRayRoom]->Wait(XRayWaitingRoomLock[patientXRayRoom]);
		
		XRayWaitingRoomLock[patientXRayRoom]->Release();

		//Going with Nurse back to his Exam Room
		nurseXRayPatientLock[index]->Acquire();

		patientNurseExamRoomGlobal[index]=patientExamRoom;
		//printf("Patient %d Going back to the exam room with nurse\n",index);

		//Signal the nurse I am coming with you
		nurseXRayPatientCV[index]->Signal(nurseXRayPatientLock[index]);
		nurseXRayPatientCV[index]->Wait(nurseXRayPatientLock[index]);
		nurseXRayPatientCV[index]->Signal(nurseXRayPatientLock[index]);
		
		nurseXRayPatientLock[index]->Release();

		//Waiting for Doctor to check me again
	    RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

		patientExamSheetGlobal[patientExamRoom]=patientExamSheet;
		
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
	
		doctorID= doctorIDGlobal[patientExamRoom];
		
		if(flag6==1 || flag5==1)
		printf("Adult Patient %d has been diagnosed by the Doctor %d\n",index);
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		//RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

	}//Only XRay If done

	//Only Shot
	else if(patientExamSheet.xray==0 && patientExamSheet.shot==1)
	{
		//Letting Nurse go to get the medicine for shot
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
		
		if(flag7==1 || flag5==1)
		printf("Adult Patient %d says,\"Yes I'm ready for the Shot\"\n",index);

		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
	}

	//Both XRay and Shot done
	else if(patientExamSheet.xray==1 && patientExamSheet.shot==1)
	{
		//---------------------1st Shot------------------------------------------//
		//Letting Nurse go to get the medicine for shot
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
		
		if(flag7==1 || flag5==1)
		printf("Adult Patient %d says,\"Yes I'm ready for the Shot\"\n",index);

		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);


		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);

		//---------------------2nd XRay-------------------------------------------//
		
		if(flag4 == 1 || flag5==1)
		printf("Adult Patient %d waits for a Nurse to Escort him/her to Xray room\n",index);
		
		//Waiting for Nurse to find the XRay Room
		RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

		patientXRayRoom=XRayRoomGlobal[patientExamRoom];
		waitInXRayLine=waitInXRayLineGlobal[patientExamRoom];

		if(waitInXRayLine==0)					//Not to Wait
		{
			RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
			RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

			//Nurse passed him necessary info
			
			//Now talking with XRay Technician
			XRayRoomLock[patientXRayRoom]->Acquire();

			//Signalling the XRayTechnician
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

		}
		else									//Need to Wait
		{
			RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
			RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
			
			//Patient done with Nurse to escort to Xray Room

			//Waiting in Line for XRay
			waitingForXRayLock[patientXRayRoom]->Acquire();

			//Flags
			PatientXRayEnter[patientXRayRoom]++;	//Enters 
			
			if(XRayPatientEnter[patientXRayRoom]==0)
			{
				printf("Patient %d is in If\n",index);
				//XRayCount[patientXRayRoom]++;
				printf("XRayCount %d is %d\n",patientXRayRoom, XRayCount[patientXRayRoom]);
				waitingForXRayCV[patientXRayRoom]->Wait(waitingForXRayLock[patientXRayRoom]);
				XRayPatientEnter[patientXRayRoom]=0;
				waitingForXRayCV[patientXRayRoom]->Signal(waitingForXRayLock[patientXRayRoom]);
			}
			else
			{
				printf("Patient %d is in Else\n",index);
				XRayPatientEnter[patientXRayRoom]=0;
				waitingForXRayCV[patientXRayRoom]->Signal(waitingForXRayLock[patientXRayRoom]);
			}
			
			waitingForXRayCV[patientXRayRoom]->Wait(waitingForXRayLock[patientXRayRoom]);
			//Interacting with XRay Technician
			waitingForXRayLock[patientXRayRoom]->Release();
			
			XRayRoomLock[patientXRayRoom]->Acquire();

			XRayExamSheetGlobal[patientXRayRoom]=patientExamSheet;

			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
		}

		if(flag7==1 || flag5==1)
		printf("Adult Patient %d gets on the Table\n",index);

		if(patientExamSheet.imageCount==1)
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//patientExamSheet.image=imageGlobal[patientXRayRoom];

		}
		else if(patientExamSheet.imageCount==2)
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//printf("Patient %d moving after 1st Image\n",index);

			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//patientExamSheet.image=imageGlobal[patientXRayRoom];

		}
		else if(patientExamSheet.imageCount==3)
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//printf("Patient %d moving after 1st Image\n",index);

			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//printf("Patient %d moving after 2nd Image\n",index);

			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//patientExamSheet.image=imageGlobal[patientXRayRoom];

		}
		else
		{}

		//----------Waiting in XRay waiting room for nurse to come----------//
		XRayWaitingRoomLock[patientXRayRoom]->Acquire();
		
		//Signals the XRay Technician to hang his sheet on Wall Pocket
		XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
		
		XRayRoomLock[patientXRayRoom]->Release();
		
		//Waits for the Nurse 
		XRayWaitingRoomCV[patientXRayRoom]->Wait(XRayWaitingRoomLock[patientXRayRoom]);
		
		XRayWaitingRoomLock[patientXRayRoom]->Release();

		//Going with Nurse back to his Exam Room
		nurseXRayPatientLock[index]->Acquire();

		patientNurseExamRoomGlobal[index]=patientExamRoom;

		//Signal the nurse I am coming with you
		nurseXRayPatientCV[index]->Signal(nurseXRayPatientLock[index]);
		nurseXRayPatientCV[index]->Wait(nurseXRayPatientLock[index]);
		nurseXRayPatientCV[index]->Signal(nurseXRayPatientLock[index]);
		
		nurseXRayPatientLock[index]->Release();

		//Waiting for Doctor to check me again
	    RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

		//doctorID= doctorIDGlobal[patientExamRoom];
		patientExamSheetGlobal[patientExamRoom]=patientExamSheet;
		
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
		
		doctorID= doctorIDGlobal[patientExamRoom];
		if(flag6==1 || flag5==1)
		printf("Adult Patient %d has been diagnosed by the Doctor %d\n",index,doctorID);
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		
	}
	else
	{
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		if(flag6==1 || flag5==1)
		printf("Adult Patient %d has been diagnosed by Doctor %d\n",index, doctorID);
	}

	//Waiting for the Nurse to carry to the Cashier
	RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
	
	RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
	
	//XRayRoomLock[patientXRayRoom]->Release();
	RoomLock[patientExamRoom]->Release();


	//Freeing the Exam Room
	ExamRoomLock->Acquire();
		ExamRoomMV[patientExamRoom]=0;
	ExamRoomLock->Release();
	
	//-----------------------------Cashier-Patient Interaction------------------------------//
	
	cashCounterLock->Acquire();
	
	if(cashierState==1)		//Cashier busy. Wait in line
	{
		patientWaitingForCashierCount++;
		
		if(flag3==1 || flag5==1)
		printf("Adult Patient %d enters the Queue for the Cashier\n",index);
		
		cashCounterCV->Wait(cashCounterLock);
	}
	else
	{
		cashierState=1;
	}
	
	cashCounterLock->Release();
	
	//Got the Cashier
	cashierLock->Acquire();
	
	//Giving Exam Sheet to the Cashier
	if(flag3==1 || flag5==1)
	printf("Adult Patient %d reaches the Cashier\n",index);
	
	if(flag3==1 || flag5==1)
	printf("Adult Patient %d hands over the Examination Sheet to the Cashier\n",index);
	
	cashierExamSheetGlobal=patientExamSheet;
	
	cashierCV->Signal(cashierLock);
	cashierCV->Wait(cashierLock);
	
	fees=feesGlobal;
	
	if(flag3==1 || flag5==1)
	printf("Adult Patient %d pays the Cashier: $%f\n", index, fees);

	cashierCV->Signal(cashierLock);
	cashierCV->Wait(cashierLock);
	
	if(flag3==1 || flag5==1)
	printf("Adult Patient %d recieves a receipt from the Cashier\n", index);
	
	cashierCV->Signal(cashierLock);
	
	if(flag3==1 || flag5==1)
	printf("Adult Patient %d leaves the Doctor's Office\n",index);
	
	cashierLock->Release();
	
	//------------------------------------Done With Cashier-----------------------------------//

}

void Parent(int index)
{

	int patientExamRoom; 				
	struct ExamSheet patientExamSheet;
	int nurseID;
	int doctorID;
	int patientXRayRoom;
	int waitInXRayLine;
	float fees;

	childPatientLock[index]->Acquire();
	parentEnter[index]=1;
	
	if(childEnter[index]= 1)
	{
		childPatientCV[index]->Signal(childPatientLock[index]);
	}
	else
	{
		childPatientCV[index]->Wait(childPatientLock[index]);
		childPatientCV[index]->Signal(childPatientLock[index]);
	}

	childPatientCV[index]->Wait(childPatientLock[index]);
	
	//--------------------Entering the queue to get the form----------------------------
	waitingRoomLock->Acquire();

	if(wrnState==1)		//WRN busy. Wait in line
	{
		patientWaitingCount++;
		if(flag2 == 1 || flag1 == 1 || flag5==1)
		printf("Parent %d gets in the line of Waiting Room Nurse to get registration form\n",index);

		patientWaitingCV->Wait(waitingRoomLock);
	}
	else
	{
		//wrnState="busy";
		wrnState=1;
	}

	waitingRoomLock->Release();

	//----------------------------WRN<--> Parent Interaction-------------------------------

	//---------------------------------GETFORM----------------------------------------------
	//Acquire Lock to get the form
	wrnLock->Acquire();

	//Get the form from WRN
	//wrnTask="getForm";
	decision = 1;
	parent =1;				//Parent of a Child
	patientIDGlobal = index;

	//Patient Signal's WRN that he needs an empty form and goes to wait
	wrnWaitingCV->Signal(wrnLock);
	wrnWaitingCV->Wait(wrnLock);

	//Patient takes empty form from WRN
	patientExamSheet=emptyExamSheet;

	patientExamSheet.visit=0;			//Visit 1
	patientExamSheet.child=1;			//Parent
	patientExamSheet.xray=0;
	patientExamSheet.shot=0;

	if(flag2 == 1 || flag1 == 1 || flag5==1)
	printf("Parent %d gets the form from the Waiting Room Nurse\n", index);

	wrnWaitingCV->Signal(wrnLock);

	wrnLock->Release();
	//Done with getForm

	//----------------------------------------GIVEFORM-----------------------------------------//
	//--------------------------------Entering the queue to submit the form--------------------//
	waitingRoomLock->Acquire();

	if(wrnState==1)		//WRN busy. Wait in line
	{
		patientWaitingCount++;
		
		if(flag2 == 1 || flag1 == 1 || flag5==1)
		printf("Parent %d gets in the line of Waiting Room Nurse to submit registration form\n",index);
		
		patientWaitingCV->Wait(waitingRoomLock);
	}
	else
	{
		//wrnState="busy";
		wrnState=1;
	}

	waitingRoomLock->Release();

	//Acquire lock to submit the form
	wrnLock->Acquire();
	
	//Give the form to WRN
	//wrnTask="giveForm";
	decision = 2;
	parent=1;	
	
	//Filling the ExamRoomSheet
	patientExamSheet.age=rand()%14;
	patientExamSheet.id=index;

	WrnFromPatientExamSheet=patientExamSheet;

	//Patient Signals WRN that he wants to submit filled form and Waits
	wrnWaitingCV->Signal(wrnLock);
	wrnWaitingCV->Wait(wrnLock);

	if(flag2 == 1 || flag1 == 1 || flag5==1)
	printf("Parent %d submits the filled form from the Waiting Room Nurse\n", index);

	wrnWaitingCV->Signal(wrnLock);

	wrnLock->Release();
	
	//-------------------------Done with giveForm and waiting for the Nurse---------------------//
	
	//#---------------------------Done with WRN-Patient Interaction----------------------------#


	//#---------------------------------Nurse-Patient-Interaction------------------------------#
	//Patient going with nurse to Exam Room
	registeredPatientLock->Acquire();

	nurseRegisteredPatientCV->Wait(registeredPatientLock);

	patientExamRoom=patientAssignedRoomGlobal;
	nurseID=nurseIDExamRoom;

	if(flag4 == 1 || flag1 == 1 || flag5==1)
	printf("Parent %d is following Nurse %d to Examination Room %d\n",index, nurseID, patientExamRoom);
	if(flag4 == 1 || flag1 == 1 || flag5==1)
	printf("Parent %d asks Child Patient %d to follow them\n",index,index);
	
	nurseRegisteredPatientCV->Signal(registeredPatientLock);

	RoomLock[patientExamRoom]->Acquire();

	registeredPatientLock->Release();

	if(flag4 == 1 || flag1 == 1 || flag5==1)
	printf("Parent %d arrived at Examination Room %d\n",index, patientExamRoom);

	//Performing symptoms check with Nurse
	//Waiting for nurse to come in the Exam Room and check my symptoms
	RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

	//Patient Signals Nurse to test Temperature and BP and goes to Wait
	RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);

	RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

	patientExamSheet=NurseToPatientExamSheet[patientExamRoom];

	patientExamSheet.pain=symptoms[rand()%2];
	patientExamSheet.nausea=symptoms[rand()%2];
	patientExamSheet.hear_alien_voices=symptoms[rand()%2];

	if(flag4 == 1 || flag1 == 1 || flag5==1)
	printf("Parent %d says, \"His Symptoms are Pain: %s, Nausea: %s, Hear Alien Voices: %s \"\n",index, patientExamSheet.pain, patientExamSheet.nausea, patientExamSheet.hear_alien_voices);

	RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);

	//--------------------------Patient done with Symptoms Check-------------------------------//

	//------------------------Patient is waiting for Doctor's Visit----------------------------//

	RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

	patientExamSheetGlobal[patientExamRoom]=patientExamSheet;

	RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
	RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
	
	RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);

	patientExamSheet=patientExamSheetGlobal[patientExamRoom];
	doctorID=doctorIDGlobal[patientExamRoom];

	//Letting Child know whether he needs an xray and shot.
	ParentToChildExamSheet[index]=patientExamSheet;
	childRoom[index]=patientExamRoom;
	childDoctor[index]=doctorID;
	
	childPatientCV[index]->Signal(childPatientLock[index]);
	childPatientCV[index]->Wait(childPatientLock[index]);


	//--------------------------Patient-Xray Begins------------------------------------//
	//Waiting for Nurse to come
	//int patientVisit1Done[patientExamRoom]=1;
	
	if(patientExamSheet.xray==1 && patientExamSheet.shot==0)
	{
	if(flag4 == 1 || flag1 == 1 || flag5==1)
	printf("Parent %d waits for a Nurse to Escort them to Xray room\n",index);
	if(flag4 == 1 || flag1 == 1 || flag5==1)
	printf("Parent %d asks Child Patient %d to follow them\n",index,index);
	}
	
	RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
	
	//Only XRay
	if(patientExamSheet.xray==1 && patientExamSheet.shot==0)
	{
		patientXRayRoom=XRayRoomGlobal[patientExamRoom];
		waitInXRayLine=waitInXRayLineGlobal[patientExamRoom];


		if(waitInXRayLine==0)					//Not to Wait
		{
		
			RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
			RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
			
			//Nurse passed him necessary info
			//Now talking with XRay Technician

			XRayRoomLock[patientXRayRoom]->Acquire();
			//RoomLock[patientExamRoom]->Release();

			//Signalling the XRayTechnician
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

		}
		else									//Need to Wait
		{
			RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
			RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

			
			//Patient done with Nurse to escort to Xray Room

			//Waiting in Line for XRay
			waitingForXRayLock[patientXRayRoom]->Acquire();

			//Flags
			PatientXRayEnter[patientXRayRoom]++;	//Enters 
			
			if(XRayPatientEnter[patientXRayRoom]==0)
			{
				//XRayCount[patientXRayRoom]++;
				printf("XRayCount %d is %d\n",patientXRayRoom, XRayCount[patientXRayRoom]);
				waitingForXRayCV[patientXRayRoom]->Wait(waitingForXRayLock[patientXRayRoom]);
				XRayPatientEnter[patientXRayRoom]=0;
				waitingForXRayCV[patientXRayRoom]->Signal(waitingForXRayLock[patientXRayRoom]);
			}
			else
			{
				XRayPatientEnter[patientXRayRoom]=0;
				waitingForXRayCV[patientXRayRoom]->Signal(waitingForXRayLock[patientXRayRoom]);
			}
			waitingForXRayCV[patientXRayRoom]->Wait(waitingForXRayLock[patientXRayRoom]);

			//Interacting with XRay Technician
			waitingForXRayLock[patientXRayRoom]->Release();
			XRayRoomLock[patientXRayRoom]->Acquire();

			XRayExamSheetGlobal[patientXRayRoom]=patientExamSheet;

			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
		}


		//Passing patient XRay Room
		childXRayRoom[index]=patientXRayRoom;
		childPatientCV[index]->Signal(childPatientLock[index]);
		childPatientCV[index]->Wait(childPatientLock[index]);
		

		if(patientExamSheet.imageCount==1)
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//patientExamSheet.image=imageGlobal[patientXRayRoom];

		}
		else if(patientExamSheet.imageCount==2)
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
			
			//printf("Patient %d moving after 1st Image\n",index);
			childPatientCV[index]->Signal(childPatientLock[index]);
			childPatientCV[index]->Wait(childPatientLock[index]);

			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//patientExamSheet.image=imageGlobal[patientXRayRoom];

		}
		else if(patientExamSheet.imageCount==3)
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//printf("Patient %d moving after 1st Image\n",index);
			childPatientCV[index]->Signal(childPatientLock[index]);
			childPatientCV[index]->Wait(childPatientLock[index]);

			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//printf("Patient %d moving after 1st Image\n",index);
			childPatientCV[index]->Signal(childPatientLock[index]);
			childPatientCV[index]->Wait(childPatientLock[index]);

			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//patientExamSheet.image=imageGlobal[patientXRayRoom];

		}
		else
		{}

		//----------Waiting in XRay waiting room for nurse to come----------//
		
		XRayWaitingRoomLock[patientXRayRoom]->Acquire();
		
		//Signals the XRay Technician to hang his sheet on Wall Pocket
		XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
		
		XRayRoomLock[patientXRayRoom]->Release();
		
		//Waits for the Nurse 
		XRayWaitingRoomCV[patientXRayRoom]->Wait(XRayWaitingRoomLock[patientXRayRoom]);
		
		XRayWaitingRoomLock[patientXRayRoom]->Release();

		//Going with Nurse back to his Exam Room
		nurseXRayPatientLock[index]->Acquire();

		patientNurseExamRoomGlobal[index]=patientExamRoom;

		//Signal the nurse I am coming with you
		nurseXRayPatientCV[index]->Signal(nurseXRayPatientLock[index]);
		nurseXRayPatientCV[index]->Wait(nurseXRayPatientLock[index]);
		nurseXRayPatientCV[index]->Signal(nurseXRayPatientLock[index]);
		
		nurseXRayPatientLock[index]->Release();

		//Waiting for Doctor to check me again
	    RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

		
		patientExamSheetGlobal[patientExamRoom]=patientExamSheet;
		
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
		
		doctorID= doctorIDGlobal[patientExamRoom];
		
		if(flag6==1 || flag1 == 1 || flag5==1)
		printf("Child Patient %d has been diagnosed by the Doctor %d\n",index);
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

	}//Only XRay If done

	//Only Shot
	else if(patientExamSheet.xray==0 && patientExamSheet.shot==1)
	{
		//Letting Nurse go to get the medicine for shot
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

		if(flag4 == 1 || flag1 == 1 || flag5==1)
		printf("Parent %d says,\"Yes He/She is ready for the Shot\"\n",index);

		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		
		childPatientCV[index]->Signal(childPatientLock[index]);
		childPatientCV[index]->Wait(childPatientLock[index]);
		
	}

	//Both XRay and Shot done
	else if(patientExamSheet.xray==1 && patientExamSheet.shot==1)
	{
		//1st Shot
		//Letting Nurse go to get the medicine for shot
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

		if(flag4 == 1 || flag1 == 1 || flag5==1)
		printf("Parent %d says,\"Yes He/She is ready for the Shot\"\n",index);

		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		
		childPatientCV[index]->Signal(childPatientLock[index]);
		childPatientCV[index]->Wait(childPatientLock[index]);
		
		RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);

		//2nd XRay
		//Waiting for Nurse to find the XRay Room
		if(flag4 == 1 || flag1 == 1 || flag5==1)
		printf("Parent %d waits for a Nurse to Escort them to Xray room\n",index);
		if(flag4 == 1 || flag1 == 1 || flag5==1)
		printf("Parent %d asks Child Patient %d to follow them\n",index,index);
		RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

		patientXRayRoom=XRayRoomGlobal[patientExamRoom];
		waitInXRayLine=waitInXRayLineGlobal[patientExamRoom];

		if(waitInXRayLine==0)					//Not to Wait
		{
			//printf("Patient %d going to Xray Room %d\n", index, patientXRayRoom);
			RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
			RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

			//Nurse passed him necessary info
			
			//Now talking with XRay Technician
			XRayRoomLock[patientXRayRoom]->Acquire();

			//Signalling the XRayTechnician
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

		}
		else									//Need to Wait
		{
			//printf("Patient %d going to Xray Room %d\n", index, patientXRayRoom);
			RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
			RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
			
			//Patient done with Nurse to escort to Xray Room

			//Waiting in Line for XRay
			waitingForXRayLock[patientXRayRoom]->Acquire();

			//Flags
			PatientXRayEnter[patientXRayRoom]++;	//Enters 
			
			if(XRayPatientEnter[patientXRayRoom]==0)
			{
				printf("Patient %d is in If\n",index);
				//XRayCount[patientXRayRoom]++;
				printf("XRayCount %d is %d\n",patientXRayRoom, XRayCount[patientXRayRoom]);
				waitingForXRayCV[patientXRayRoom]->Wait(waitingForXRayLock[patientXRayRoom]);
				XRayPatientEnter[patientXRayRoom]=0;
				waitingForXRayCV[patientXRayRoom]->Signal(waitingForXRayLock[patientXRayRoom]);
			}
			else
			{
				printf("Patient %d is in Else\n",index);
				XRayPatientEnter[patientXRayRoom]=0;
				waitingForXRayCV[patientXRayRoom]->Signal(waitingForXRayLock[patientXRayRoom]);
			}
			
			waitingForXRayCV[patientXRayRoom]->Wait(waitingForXRayLock[patientXRayRoom]);

			//Interacting with XRay Technician
			waitingForXRayLock[patientXRayRoom]->Release();
			XRayRoomLock[patientXRayRoom]->Acquire();

			XRayExamSheetGlobal[patientXRayRoom]=patientExamSheet;

			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);
		}

		childPatientCV[index]->Signal(childPatientLock[index]);
		childPatientCV[index]->Wait(childPatientLock[index]);

		if(patientExamSheet.imageCount==1)
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//patientExamSheet.image=imageGlobal[patientXRayRoom];

		}
		else if(patientExamSheet.imageCount==2)
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			printf("Patient %d moving after 1st Image\n",index);
			childPatientCV[index]->Signal(childPatientLock[index]);
			childPatientCV[index]->Wait(childPatientLock[index]);

			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//patientExamSheet.image=imageGlobal[patientXRayRoom];

		}
		else if(patientExamSheet.imageCount==3)
		{
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//printf("Patient %d moving after 1st Image\n",index);
			childPatientCV[index]->Signal(childPatientLock[index]);
			childPatientCV[index]->Wait(childPatientLock[index]);

			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//printf("Patient %d moving after 2nd Image\n",index);
			childPatientCV[index]->Signal(childPatientLock[index]);
			childPatientCV[index]->Wait(childPatientLock[index]);
		
			XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
			XRayRoomCV[patientXRayRoom]->Wait(XRayRoomLock[patientXRayRoom]);

			//patientExamSheet.image=imageGlobal[patientXRayRoom];

		}
		else
		{}

		//----------Waiting in XRay waiting room for nurse to come----------//
		XRayWaitingRoomLock[patientXRayRoom]->Acquire();
		
		//Signals the XRay Technician to hang his sheet on Wall Pocket
		XRayRoomCV[patientXRayRoom]->Signal(XRayRoomLock[patientXRayRoom]);
		
		XRayRoomLock[patientXRayRoom]->Release();
		
		//Waits for the Nurse 
		XRayWaitingRoomCV[patientXRayRoom]->Wait(XRayWaitingRoomLock[patientXRayRoom]);
		
		XRayWaitingRoomLock[patientXRayRoom]->Release();

		//Going with Nurse back to his Exam Room
		nurseXRayPatientLock[index]->Acquire();

		patientNurseExamRoomGlobal[index]=patientExamRoom;

		//Signal the nurse I am coming with you
		nurseXRayPatientCV[index]->Signal(nurseXRayPatientLock[index]);
		nurseXRayPatientCV[index]->Wait(nurseXRayPatientLock[index]);
		nurseXRayPatientCV[index]->Signal(nurseXRayPatientLock[index]);
		
		nurseXRayPatientLock[index]->Release();

		//Waiting for Doctor to check me again
	    RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);

		//doctorID= doctorIDGlobal[patientExamRoom];
		patientExamSheetGlobal[patientExamRoom]=patientExamSheet;
		
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
		
		doctorID= doctorIDGlobal[patientExamRoom];
		if(flag6==1 || flag1 == 1 || flag5==1)
		printf("Child Patient %d has been diagnosed by the Doctor %d\n",index,doctorID);
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
	}
	else
	{
		RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
		if(flag6==1 || flag1 == 1 || flag5==1)
		printf("Child Patient %d has been diagnosed by Doctor %d\n",index, doctorID);
	}

	//Waiting for the Nurse to carry to the Cashier
	RoomLockCV[patientExamRoom]->Wait(RoomLock[patientExamRoom]);
	
	RoomLockCV[patientExamRoom]->Signal(RoomLock[patientExamRoom]);
	
	RoomLock[patientExamRoom]->Release();


	//Temporary until another tasks start, freeing the Exam Room
	ExamRoomLock->Acquire();
		ExamRoomMV[patientExamRoom]=0;
	ExamRoomLock->Release();

	//printf("Patient %d left the clinic\n",index);
	
	//-----------------------------Cashier-Patient Interaction------------------------------#
	
	cashCounterLock->Acquire();
	
	if(cashierState==1)		//Cashier busy. Wait in line
	{
		patientWaitingForCashierCount++;
		if(flag3 == 1 || flag1 == 1 || flag5==1)
		printf("Parent enters the Queue for Cashier\n",index);
		cashCounterCV->Wait(cashCounterLock);
	}
	else
	{
		cashierState=1;
	}
	
	cashCounterLock->Release();
	
	//Got the Cashier
	cashierLock->Acquire();
	
	//Giving Exam Sheet to the Cashier
	if(flag3 == 1 || flag1 == 1 || flag5==1)
	printf("Parent %d reaches the Cashier\n",index);
	if(flag3 == 1 || flag1 == 1 || flag5==1)
	printf("Parent %d hands over Child Patient's %d Examination Sheet to the Cashier\n",index,index);
	
	cashierExamSheetGlobal=patientExamSheet;
	
	cashierCV->Signal(cashierLock);
	cashierCV->Wait(cashierLock);
	
	fees=feesGlobal;
	
	if(flag3 == 1 || flag1 == 1 || flag5==1)
	printf("Parent %d pays the Cashier: $%f\n", index, fees);

	cashierCV->Signal(cashierLock);
	cashierCV->Wait(cashierLock);
	
	if(flag3 == 1 || flag1 == 1 || flag5==1)
	printf("Parent %d recieves the Receipt for Child Patient %d from the Cashier\n",index,index);
	
	cashierCV->Signal(cashierLock);
	
	if(flag3 == 1 || flag1 == 1 || flag5==1)
	printf("Parent %d leaves the Doctor's Office with Child Patient %d\n",index,index);
	cashierLock->Release();
	
	childPatientCV[index]->Signal(childPatientLock[index]);
	childPatientLock[index]->Release();
	
	//------------------------------------Done With Cashier-----------------------------------//

}

void Child(int index)
{
	int childExamRoom;
	int doctorID;
	int waitInXrayLine;
	int childXray;
	struct ExamSheet childExamSheet;
	childPatientLock[index]->Acquire();
	
	childEnter[index]=1;
	
	if(parentEnter[index]==0)
	{
		childPatientCV[index]->Wait(childPatientLock[index]);
	}
	else
	{
		childPatientCV[index]->Signal(childPatientLock[index]);
		childPatientCV[index]->Wait(childPatientLock[index]);
	}
	
	if(flag2==1 || flag1==1 || flag5==1)
	printf("Child Patient %d has entered the Doctor's Office Waiting Room with Parent %d.\n", index, index);
	childPatientCV[index]->Signal(childPatientLock[index]);
	
	childPatientCV[index]->Wait(childPatientLock[index]);
	childExamRoom=childRoom[index];;
	doctorID=childDoctor[index];
	childExamSheet=ParentToChildExamSheet[index];
	if(childExamSheet.xray==1 && childExamSheet.shot==0)
	{
		if(flag6==1 || flag1==1 || flag5==1)
		printf("Child Patient %d has been informed by Doctor %d that he needs an XRay\n", index, doctorID);
	}
	else if(childExamSheet.xray==0 && childExamSheet.shot==1)
	{
		if(flag6==1 || flag1==1 || flag5==1)
		printf("Child Patient %d has been informed by Doctor %d that he will be administered a Shot\n", index, doctorID);
	}
	else if(childExamSheet.xray==1 && childExamSheet.shot==1)
	{
		if(flag6==1 || flag1==1 || flag5==1)
		printf("Child Patient %d has been informed by doctor that he needs an XRay\n", index, doctorID);
		if(flag6==1 || flag1==1 || flag5==1)
		printf("Child Patient %d has been informed by doctor that he will be administered a Shot\n", index, doctorID);
	}
	else{}
	childPatientCV[index]->Signal(childPatientLock[index]);
	

	
	if(childExamSheet.xray==1 && childExamSheet.shot==0)
	{	
		childPatientCV[index]->Wait(childPatientLock[index]);
		childXray=childXRayRoom[index];
		if(flag1==1)
		printf("Child Patient %d gets on to the table\n", index);
		childPatientCV[index]->Signal(childPatientLock[index]);
		
		if(childExamSheet.imageCount==1)
		{
		}
		else if(childExamSheet.imageCount==2)
		{
			
			childPatientCV[index]->Wait(childPatientLock[index]);
			if(flag1==1)
			printf("Child Patienht %d moves for the next Xray.\n",index);
			childPatientCV[index]->Signal(childPatientLock[index]);
		}
		else
		{
			childPatientCV[index]->Wait(childPatientLock[index]);
			if(flag1==1)
			printf("Child Patienht %d moves for the next Xray.\n",index);
			childPatientCV[index]->Signal(childPatientLock[index]);
			
			childPatientCV[index]->Wait(childPatientLock[index]);
			if(flag1==1)
			printf("Child Patienht %d moves for the next Xray.\n",index);
			childPatientCV[index]->Signal(childPatientLock[index]);
			
		}
	}
	else if(childExamSheet.xray==0 && childExamSheet.shot==1)
	{
		childPatientCV[index]->Wait(childPatientLock[index]);
		if(flag4==1)
		printf("Child patient %d has been given a shot.\n", index);
		childPatientCV[index]->Signal(childPatientLock[index]);
	}
	else if(childExamSheet.xray==1 && childExamSheet.shot==1)
	{
		childPatientCV[index]->Wait(childPatientLock[index]);
		if(flag4==1 || flag1==1 || flag5==1)
		printf("Child Patient %d has been given a shot.\n", index);
		childPatientCV[index]->Signal(childPatientLock[index]);
		
		childPatientCV[index]->Wait(childPatientLock[index]);
		childXray=childXRayRoom[index];
		if(flag1==1 || flag5==1)
		printf("Child Patient %d gets on to the table\n", index);
		childPatientCV[index]->Signal(childPatientLock[index]);
		
		if(childExamSheet.imageCount==1)
		{
		}
		else if(childExamSheet.imageCount==2)
		{
			childPatientCV[index]->Wait(childPatientLock[index]);
			if(flag1==1 || flag5==1)
			printf("Child Patienht %d moves for the next Xray.\n",index);
			childPatientCV[index]->Signal(childPatientLock[index]);
		}
		else
		{
			childPatientCV[index]->Wait(childPatientLock[index]);
			if(flag1==1 || flag5==1)
			printf("Child Patient %d moves for the next Xray.\n",index);
			childPatientCV[index]->Signal(childPatientLock[index]);
			
			childPatientCV[index]->Wait(childPatientLock[index]);
			if(flag1==1 || flag5==1)
			printf("Child Patient %d moves for the next Xray.\n",index);
			childPatientCV[index]->Signal(childPatientLock[index]);
		}	
		
	}
	childPatientCV[index]->Wait(childPatientLock[index]);
	
	childPatientLock[index]->Release();
}




void Nurse(int index)
{
	int NurseExamRoom;								// To associate an Exam room with Nurse
	int ExamRoomCount = 0;							//To chek if there is an Exam Room Available
	struct ExamSheet nurseExamSheet;
	int DoctorFree;
	int NurseXRayRoom;

	while(1)
	{

	nurseExamSheet.id=0;

	if(patientSize1<n)
	{
	//-------Task 1 begins:Getting patient from the queue and escort to the exam room------//

	ExamRoomLock->Acquire();

	//Check which waiting room is free--//
	ExamRoomCount=0;
	for(int i=1;i<=examrooms;i++)
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
	if(ExamRoomCount<examrooms)
	{
		//--------------------Entering the queue to talk with WRN----------------------------//
		waitingRoomLock->Acquire();
		
		if(wrnNurseState==1)						//WRN busy. Wait in line
		{
			nurseWaitingCount++;					//Notify you entered the Queue
			nurseWaitingCV->Wait(waitingRoomLock);	//Go wait in Queue
		}
		else
		{
			wrnNurseState=1;
		}

		waitingRoomLock->Release();

		//----------------------------WRN<-->Nurse Interaction-----------------------------//
		/*Acquire the wrnLock to check if there is any patient to be escorted to Examination Room
		/*------------Lock to get the patient form----------------*/
		wrnLock->Acquire();

		//Nurse Signals WRN that she has a ExamRoom Free a nd waits if there are any patient waiting to be treated
		nurseIDGlobal=index;
		if(flag7==1)
		printf("Nurse %d tells Waiting Room Nurse to give a new Examination Sheet\n", index);

		nurseWrnWaitingCV->Signal(wrnLock);
		nurseWrnWaitingCV->Wait(wrnLock);

		//----------------Escort any Waiting Patient from Waiting Room--------------------------------//
		//If any form recieved from WRN notify the patient
		if(WrnToNurseExamSheet.id!=0)
		{
			//EscortPatient=1;
			nurseExamSheet=WrnToNurseExamSheet;				// Take the ExamSheet from WRN

			patientSize1++;
			
			nurseWrnWaitingCV->Signal(wrnLock);
			
			//NURSE-Patient Interaction
			registeredPatientLock-> Acquire();
			
			patientAssignedRoomGlobal=NurseExamRoom;
			nurseIDExamRoom=index;

			nurseRegisteredPatientCV->Signal(registeredPatientLock);
			nurseRegisteredPatientCV->Wait(registeredPatientLock);
			
			if(nurseExamSheet.child==0)
			{
				if(flag4==1)
				printf("Nurse %d escorts Adult Patient %d to the Examination Room %d\n",index,nurseExamSheet.id, NurseExamRoom);
			}
			else
			{
				if(flag4==1)
				printf("Nurse %d escorts Parent %d to the Examination Room %d\n",index,nurseExamSheet.id, NurseExamRoom);
			}
			
			registeredPatientLock-> Release();
			
			wrnLock->Release();
		
			//Performing Symptom Check with Patient
			RoomLock[NurseExamRoom]->Acquire();
		
			//Nurse enters the Exam Room to talk with patient
		
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
			RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);
		
			if(nurseExamSheet.child==0)
			{
				if(flag4==1)
				printf("Nurse %d takes the Temperature and Blood Pressure of Adult Patient %d\n",index, nurseExamSheet.id);
			}
			else
			{
				if(flag4==1)
				printf("Nurse %d takes the Temperature and Blood Pressure of Child Patient %d\n",index, nurseExamSheet.id);
			}
			
			nurseExamSheet.temperature=(80 + rand()%20);
			nurseExamSheet.BP=(80 + rand()%40);

			NurseToPatientExamSheet[NurseExamRoom]= nurseExamSheet;

			if(nurseExamSheet.child==0)
			{
				if(flag4==1)
				printf("Nurse %d asks Adult Patient %d, \"What Symptoms do you have?\"\n",index, nurseExamSheet.id);
			}
			else
			{
				if(flag4==1)
				printf("Nurse %d asks Parent %d, \"What Symptoms does he/she have?\"\n",index, nurseExamSheet.id);
			}
			
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
			RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);

			RoomLock[NurseExamRoom]->Release();
		}
		else
		{
			
			ExamRoomLock->Acquire();
			ExamRoomMV[NurseExamRoom]=0;
			ExamRoomLock->Release();
			
			nurseWrnWaitingCV->Signal(wrnLock);
			wrnLock->Release();
		}
	}//End of if Got Exam Room


	//Delay to allow busy doctors to free themselves
	int r = 50;
	for(int i=0;i<r;i++)
	{
		currentThread->Yield();
	}

	//#------------------------------Nurse-Doctor Interaction--------------------------#
	if(nurseExamSheet.id!=0)
	{
		doctorRoomLock->Acquire();
		
		//Check which Doctor is free--//
		doctorCount=0;
		for(int i=1;i<=doctors;i++)
		{
			if(DoctorMV[i]==0)
			{
				break;
			}
			else
			{
				doctorCount++;
			}
		}

		
		if(doctorCount<doctors)		//Doctor is Available
		{

			//Removing a doctor from the doctor's queue
			DoctorFree=DoctorQueue[firstDoctor++];
			
			doctorExamRoomGlobal[DoctorFree]=NurseExamRoom;
			
			if(nurseExamSheet.child==0)
			{
				if(flag7==1)
				printf("Nurse %d informs Doctor %d that Adult Patient %d is waiting in the Examination Room %d\n ",index, DoctorFree, nurseExamSheet.id, NurseExamRoom);
				if(flag7==1)
				printf("Nurse %d hands over to the Doctor %d the Examination Sheet of Adult Patient %d\n", index, DoctorFree, nurseExamSheet.id);
			}
			else
			{	
				if(flag7==1)
				printf("Nurse %d informs Doctor %d that Child Patient %d is waiting in the Examination Room %d\n ",index, DoctorFree, nurseExamSheet.id, NurseExamRoom);
				if(flag7==1)
				printf("Nurse %d hands over to the Doctor %d the Examination Sheet of Child Patient %d\n", index, DoctorFree, nurseExamSheet.id);
			}
			
			doctorWaitingCV->Signal(doctorRoomLock);
			
			doctorRoomLock->Release();
		}
		else												//Doctor is not available
		{
			//Waiting for some Doctor to come.
			nurseCount++;
			
			nurseWaitingForDoctorCV->Wait(doctorRoomLock);

			DoctorFree=doctorIDNurseGlobal;
			
			doctorNurseLock[DoctorFree]->Acquire();
			
			doctorRoomLock->Release();
			
			doctorExamRoomGlobal[DoctorFree] =NurseExamRoom;
			
			if(nurseExamSheet.child==0)
			{	
				if(flag7==1)
				printf("Nurse %d informs Doctor %d that Adult Patient %d is waiting in the Examination Room %d\n ",index, DoctorFree, nurseExamSheet.id, NurseExamRoom);
				if(flag7==1)
				printf("Nurse %d hands over to the Doctor %d the Examination Sheet of Adult Patient %d\n", index, DoctorFree, nurseExamSheet.id);
			}
			else
			{
				if(flag7==1)
				printf("Nurse %d informs Doctor %d that Child Patient %d is waiting in the Examination Room %d\n ",index, DoctorFree, nurseExamSheet.id, NurseExamRoom);
				if(flag7==1)
				printf("Nurse %d hands over to the Doctor %d the Examination Sheet of Child Patient %d\n", index, DoctorFree, nurseExamSheet.id);
			}

			doctorNurseCV[DoctorFree]->Signal(doctorNurseLock[DoctorFree]);
			
			doctorNurseLock[DoctorFree]->Release();
		}
	}//End of IF after nurse gives Sheet to Doctor

	}//End of IF checking for Exit Criteria
	//------------------------------Nurse-Doctor Interaction ends--------------------------//

	//=====================================END of TASK 1====================================//
	//Delay
	int r = 50;
	for(int i=0;i<r;i++)
	{
		currentThread->Yield();
	}


	//=================================Task-2 Begins:Escort patient to XRay room=============//

	Task2Lock->Acquire();
	int wallPocketCount=0;

	//Check each wallpocket
	for(int i=1;i<=examrooms;i++)
	{
		if(WallPocketExamSheet[i].id!=0)
		{
			NurseExamRoom=i;
			nurseExamSheet=WallPocketExamSheet[NurseExamRoom];
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
	if(wallPocketCount<examrooms)
	{
		if(nurseExamSheet.visit==1)				//It was Patient's 1st Visit
		{
		RoomLock[NurseExamRoom]->Acquire();

		//Checking the number of XRAY/Shot/Cashier

		//Only XRay
		if(nurseExamSheet.xray==1 && nurseExamSheet.shot==0)
		{
			//Taking to XRay Room

			//Checking which XRay Room to go
			XRayLock->Acquire();
			int XRayRoomCount=0;
			for(int i=1;i<=xray;i++)
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

			//XRayLock->Release();

			//Got 1 of the XRay Room
			if(XRayRoomCount<xray)
			{
				if(nurseExamSheet.child==0)
				{
					if(flag4==1)
					printf("Nurse %d Escorts Adult Patient %d to the Xray Room %d\n",index, nurseExamSheet.id,NurseXRayRoom);
				}
				else
				{
					if(flag4==1)
					printf("Nurse %d Escorts Parent %d to the Xray Room %d\n",index, nurseExamSheet.id,NurseXRayRoom);
				}
				
				XRayRoomGlobal[NurseExamRoom]=NurseXRayRoom;
				waitInXRayLineGlobal[NurseExamRoom]=0;

				XRayLock->Release();
				
				RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
				RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);


				//Giving XRay Technician the Exam Sheet
				DEBUG('x',"Nurse XRay Room: %d\n",NurseXRayRoom);
				XRayRoomLock[NurseXRayRoom]->Acquire();
				DEBUG('x',"Nurse %d acquired NurseXrayRoomLock\n",index);
				
				//XRayLock->Release();
				
				XRayExamSheetGlobal[NurseXRayRoom]=nurseExamSheet;
				

				XRayRoomNurseCV[NurseXRayRoom]->Signal(XRayRoomLock[NurseXRayRoom]);
				if(nurseExamSheet.child==0)
				{
					if(flag7==1)
					printf("Nurse %d informs XRay Technician %d about Adult Patient %d and hands over the Examination Sheet\n",index, NurseXRayRoom, nurseExamSheet.id);
				}
				else
				{	
					if(flag7==1)
					printf("Nurse %d informs XRay Technician %d about Child Patient %d and hands over the Examination Sheet\n",index, NurseXRayRoom, nurseExamSheet.id);
				}
				
				XRayRoomNurseCV[NurseXRayRoom]->Wait(XRayRoomLock[NurseXRayRoom]);
				
				if(flag7==1)
				printf("Nurse %d leaves the Xray Room %d\n",index, NurseXRayRoom);
				
				XRayRoomLock[NurseXRayRoom]->Release();

				//Signalling the patient to go talk with XRayTechnician
				RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);

			}
			else
			{
				if(xray==2)
				{
					if(XRayCount[1]<XRayCount[2])
					{
						NurseXRayRoom=1;
					}
					else
					{
						NurseXRayRoom=2;
					}
				}
				else
				{
					NurseXRayRoom=1;
				}
				
				waitingForXRayLock[NurseXRayRoom]->Acquire();
				XRayCount[NurseXRayRoom]++;
				
				XRayLock->Release();
				
				if(nurseExamSheet.child==0)
				{
					if(flag4==1)
					printf("Nurse %d Escorts Adult Patient %d to the Xray Room %d\n",index, nurseExamSheet.id,NurseXRayRoom);
				}
				else
				{
					if(flag4==1)
					printf("Nurse %d Escorts Parent %d to the Xray Room %d\n",index, nurseExamSheet.id,NurseXRayRoom);
				}
				XRayRoomGlobal[NurseExamRoom]=NurseXRayRoom;
				waitInXRayLineGlobal[NurseExamRoom]=1;

				RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
				RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);
				RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
				
				waitingForXRayLock[NurseXRayRoom]->Release();
			}

			RoomLock[NurseExamRoom]->Release();

		}

		//Only Shot
		else if(nurseExamSheet.xray==0 && nurseExamSheet.shot==1)
		{
			//Signalling Patient that she is going to get medicine
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
			RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);

			//Nurse going to take medicine for shot
		
			SupplyCabinetLock->Acquire();
			if(SupplyCabinetState==0)
			{
				SupplyCabinetState=1;
				if(nurseExamSheet.child==0)
				{
					if(flag4==1)
					printf("Nurse %d goes to the supply cabinet to take medicine for Adult Patient %d\n",index, nurseExamSheet.id);
				}
				else
				{
					if(flag4==1)
					printf("Nurse %d goes to the supply cabinet to take medicine for Child Patient %d\n",index, nurseExamSheet.id);
				}
				
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
				if(nurseExamSheet.child==0)
				{
					if(flag4==1)
					printf("Nurse %d goes to the supply cabinet to take medicine for Adult Patient %d\n",index, nurseExamSheet.id);
				}
				else
				{
					if(flag4==1)
					printf("Nurse %d goes to the supply cabinet to take medicine for Child Patient %d\n",index, nurseExamSheet.id);
				}
				
				SupplyCabinetState=0;

				//Check if some other nurse is Waiting
				if(SupplyCabinetCount>0)
				{
					SupplyCabinetCount--;
					SupplyCabinetCV->Signal(SupplyCabinetLock);
				}

				SupplyCabinetLock->Release();
			}

			if(nurseExamSheet.child==0)
			{
				if(flag4==1)
				printf("Nurse %d asks Adult Patient %d, \"Are you ready for the Shot?\"\n",index, nurseExamSheet.id);
			}
			else
			{
				if(flag4==1)
				printf("Nurse %d asks Parent %d, \"Are you ready for the Shot?\"\n",index, nurseExamSheet.id);
			}
			
			//Signalling Patient that Shot is ready
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);

			//Waiting for Patient to get ready for Shot
			RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);

			if(nurseExamSheet.child==0)
			{
				if(flag4==1)
				printf("Nurse %d tells Adult Patient %d, \"Your Shot is over\"\n",index, nurseExamSheet.id);
			}
			else
			{
				if(flag4==1)
				printf("Nurse %d tells Parent %d, \"Your Shot is over\"\n",index, nurseExamSheet.id);
			}
			
			//Taking the Patient To the Cashier
			//Nurse Letting patient go to Cashier
			if(flag4==1)
			printf("Nurse %d Escorts Adult Patient %d to the Cashier\n",index, nurseExamSheet.id);

			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
			RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);
			
			RoomLock[NurseExamRoom]->Release();
		}

		//Both XRAY && Shot
		else if(nurseExamSheet.xray==1 && nurseExamSheet.shot==1)
		{
			//1st Taking Shot
			//Signalling Patient that she is going to get medicine
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
			RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);

			//Nurse going to take medicine for shot
		
			SupplyCabinetLock->Acquire();
			if(SupplyCabinetState==0)
			{
				SupplyCabinetState=1;
				if(nurseExamSheet.child==0)
				{
					if(flag4==1)
					printf("Nurse %d goes to the supply cabinet to take medicine for Adult Patient %d\n",index, nurseExamSheet.id);
				}
				else
				{
					if(flag4==1)
					printf("Nurse %d goes to the supply cabinet to take medicine for Child Patient %d\n",index, nurseExamSheet.id);
				}
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
				if(nurseExamSheet.child==0)
				{
					if(flag4==1)
					printf("Nurse %d goes to the supply cabinet to take medicine for Adult Patient %d\n",index, nurseExamSheet.id);
				}
				else
				{
					if(flag4==1)
					printf("Nurse %d goes to the supply cabinet to take medicine for Child Patient %d\n",index, nurseExamSheet.id);
				}
				SupplyCabinetState=0;

				//Check if some other nurse is Waiting
				if(SupplyCabinetCount>0)
				{
					SupplyCabinetCount--;
					SupplyCabinetCV->Signal(SupplyCabinetLock);
				}

				SupplyCabinetLock->Release();
			}

			if(nurseExamSheet.child==0)
			{
				if(flag4==1)
				printf("Nurse %d asks Adult Patient %d, \"Are you ready for the Shot?\"\n",index, nurseExamSheet.id);
			}
			else
			{
				if(flag4==1)
				printf("Nurse %d asks Child Patient %d, \"Are you ready for the Shot?\"\n",index, nurseExamSheet.id);
			}
			
			//Signalling Patient that Shot is ready
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);

			//Waiting for Patient to get ready for Shot
			RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);

			if(nurseExamSheet.child==0)
			{
				if(flag4==1)
				printf("Nurse %d tells Adult Patient %d, \"Your Shot is over\"\n",index, nurseExamSheet.id);
			}
			else
			{
				if(flag4==1)
				printf("Nurse %d tells Child Patient %d, \"Your Shot is over\"\n",index, nurseExamSheet.id);
			}
			
			//printf("Nurse %d giving Shot to Patient %d\n",index, nurseExamSheet.id);
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);

			//------------------------------Done with giving Shot-----------------------------//

			//---------------------------------2nd Go for XRay--------------------------------//
			//Waiting For Patient to signal that he is waiting for XRay
			RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);

			//Taking to XRay Room

			//Checking which XRay Room to go
			XRayLock->Acquire();
			int XRayRoomCount=0;
			for(int i=1;i<=xray;i++)
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

			//Got 1 of the XRay Room
			if(XRayRoomCount<xray)
			{
				if(nurseExamSheet.child==0)
				{
					if(flag4==1)
					printf("Nurse %d Escorts Adult Patient %d to the Xray Room %d\n",index, nurseExamSheet.id,NurseXRayRoom);
				}
				else
				{
					if(flag4==1)
					printf("Nurse %d Escorts Parent %d to the Xray Room %d\n",index, nurseExamSheet.id,NurseXRayRoom);
				}
				
				XRayRoomGlobal[NurseExamRoom]=NurseXRayRoom;
				waitInXRayLineGlobal[NurseExamRoom]=0;

				XRayLock->Release();
				
				RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
				RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);


				//Giving XRay Technician the Exam Sheet
				DEBUG('x',"Nurse XRay Room: %d\n",NurseXRayRoom);
				XRayRoomLock[NurseXRayRoom]->Acquire();
	
				XRayExamSheetGlobal[NurseXRayRoom]=nurseExamSheet;
				
				XRayRoomNurseCV[NurseXRayRoom]->Signal(XRayRoomLock[NurseXRayRoom]);
				if(nurseExamSheet.child==0)
				{
					if(flag7==1)
					printf("Nurse %d informs XRay Technician %d about Adult Patient %d and hands over the Examination Sheet\n",index, NurseXRayRoom, nurseExamSheet.id);
				}
				else
				{
					if(flag7==1)
					printf("Nurse %d informs XRay Technician %d about Child Patient %d and hands over the Examination Sheet\n",index, NurseXRayRoom, nurseExamSheet.id);
				}
				
				XRayRoomNurseCV[NurseXRayRoom]->Wait(XRayRoomLock[NurseXRayRoom]);
				
				if(flag7==1)
				printf("Nurse %d leaves the Xray Room %d\n",index, NurseXRayRoom);

				XRayRoomLock[NurseXRayRoom]->Release();

				//Signalling the patient to go talk with XRayTechnician
				RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);

			}
			else
			{
				if(xray==2)
				{
					if(XRayCount[1]<XRayCount[2])
					{
						NurseXRayRoom=1;
					}
					else
					{
						NurseXRayRoom=2;
					}
				}
				else
				{
					NurseXRayRoom=1;
				}
				
				
				printf("Nurse %d trying to aquire lock\n",index);
				waitingForXRayLock[NurseXRayRoom]->Acquire();
				printf("Nurse %d aquired lock\n",index);
				XRayCount[NurseXRayRoom]++;
				
				XRayLock->Release();
				
				if(nurseExamSheet.child==0)
				{
					if(flag4==1)
					printf("Nurse %d Escorts Adult Patient %d to the Xray Room %d\n",index, nurseExamSheet.id,NurseXRayRoom);
				}
				else
				{
					if(flag4==1)
					printf("Nurse %d Escorts Parent %d to the Xray Room %d\n",index, nurseExamSheet.id,NurseXRayRoom);
				}
				XRayRoomGlobal[NurseExamRoom]=NurseXRayRoom;
				waitInXRayLineGlobal[NurseExamRoom]=1;

				RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
				RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);
				RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
				
				waitingForXRayLock[NurseXRayRoom]->Release();
			}

			RoomLock[NurseExamRoom]->Release();
		}

		//Nothing To do. Patient Has to Leave
		else
		{
			//Nurse Letting patient go to Cashier
			if(nurseExamSheet.child==0)
			{
				if(flag4==1)
				printf("Nurse %d Escorts Adult Patient %d to the Cashier\n",index, nurseExamSheet.id);
			}
			else
			{
				if(flag4==1)
				printf("Nurse %d Escorts Parent %d to the Cashier\n",index, nurseExamSheet.id);
			}
			
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
			RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);
			
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
			RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);
			
			RoomLock[NurseExamRoom]->Release();
		}

		}//End of If for 1st visit

		else							//This was Patients 2nd Visit
		{
			//Nurse Letting patient go to Cashier
			RoomLock[NurseExamRoom]->Acquire();
			if(nurseExamSheet.child==0)
			{
				if(flag4==1)
				printf("Nurse %d Escorts Adult Patient %d to the Cashier\n",index, nurseExamSheet.id);
			}
			else
			{
				if(flag4==1)
				printf("Nurse %d Escorts Parent %d to the Cashier\n",index, nurseExamSheet.id);
			}
			
			RoomLockCV[NurseExamRoom]->Signal(RoomLock[NurseExamRoom]);
			RoomLockCV[NurseExamRoom]->Wait(RoomLock[NurseExamRoom]);
			
			RoomLock[NurseExamRoom]->Release();
		}

	}
	//----------------------------------Task-2 End--------------------------------//

	//Delay
	r = 50;
	for(int i=0;i<r;i++)
	{
		currentThread->Yield();
	}
	
	//-----------------Task 3 starts :Escort Patient to Exam Room and Call doctor---//
	
	Task3Lock->Acquire();
	int wallPocket=0;

	//Check each wallpocket
	
	if(xray==2)
	{
	if(!(last1<first1))				//Not Empty
	{
		//Removing ExamSheet from WallPocket
		nurseExamSheet=XRayWallPocket1[first1];
		first1++;
	
		NurseXRayRoom=1;
			
		if(nurseExamSheet.child==0)
		{	
			if(flag7==1)
			printf("Nurse %d gets Examination Sheet for Adult Patient %d in Xray Waiting Room\n", index, nurseExamSheet.id);	
		}
		else
		{
			if(flag7==1)
			printf("Nurse %d gets Examination Sheet for Parent %d in Xray Waiting Room\n", index, nurseExamSheet.id);	
		}
	
		wallPocket=1;
					
	}
	else if(!(last2<first2))
	{
		//Removing ExamSheet from WallPocket
	
		nurseExamSheet=XRayWallPocket2[first2];
		first2++;
		
		NurseXRayRoom=2;
			
		if(nurseExamSheet.child==0)
		{	
			if(flag7==1)
			printf("Nurse %d gets Examination Sheet for Adult Patient %d in Xray Waiting Room\n", index, nurseExamSheet.id);	
		}
		else
		{
			if(flag7==1)
			printf("Nurse %d gets Examination Sheet for Parent %d in Xray Waiting Room\n", index, nurseExamSheet.id);	
		}
					
		wallPocket=1;
	}
	else
	{}
	}//End of If for Checking 2 Xray room Wall Pockets
	else
	{
	if(!(last1<first1))				//Not Empty
	{
		//Removing ExamSheet from WallPocket
		nurseExamSheet=XRayWallPocket1[first1];
		first1++;
	
		NurseXRayRoom=1;
			
		if(nurseExamSheet.child==0)
		{	
			if(flag7==1)
			printf("Nurse %d gets Examination Sheet for Adult Patient %d in Xray Waiting Room\n", index, nurseExamSheet.id);	
		}
		else
		{
			if(flag7==1)
			printf("Nurse %d gets Examination Sheet for Parent %d in Xray Waiting Room\n", index, nurseExamSheet.id);	
		}
	
		wallPocket=1;
					
	}
	else
	{}
	}//End of Else for checking XRay rooms
	
	if(wallPocket==1)		//Got 1 patient waiting in XRay room
	{
		XRayWaitingRoomLock[NurseXRayRoom]->Acquire();
		
		//Signalling the Patient Waiting
		XRayWaitingRoomCV[NurseXRayRoom]->Signal(XRayWaitingRoomLock[NurseXRayRoom]);
		
		nurseXRayPatientLock[nurseExamSheet.id]->Acquire();
		XRayWaitingRoomLock[NurseXRayRoom]->Release();
		
		//Interacting with Patient
		nurseXRayPatientCV[nurseExamSheet.id]->Wait(nurseXRayPatientLock[nurseExamSheet.id]);
		
		NurseExamRoom=patientNurseExamRoomGlobal[nurseExamSheet.id];
	
		nurseXRayPatientCV[nurseExamSheet.id]->Signal(nurseXRayPatientLock[nurseExamSheet.id]);
		nurseXRayPatientCV[nurseExamSheet.id]->Wait(nurseXRayPatientLock[nurseExamSheet.id]);
		
		nurseXRayPatientLock[nurseExamSheet.id]->Release();
		
		Task3Lock->Release();
		
		//Calling doctor to exam room
		
		doctorRoomLock->Acquire();

		//Check which Doctor is free--//
		doctorCount=0;
		for(int i=1;i<=doctors;i++)
		{
			if(DoctorMV[i]==0)
			{
				break;
			}
			else
			{
				doctorCount++;
			}
		}

		if(doctorCount<doctors)		//Doctor is Available
		{

			//Removing a doctor from the doctor's queue
			DoctorFree=DoctorQueue[firstDoctor++];

			doctorExamRoomGlobal[DoctorFree]=NurseExamRoom;
			
			if(nurseExamSheet.child==0)
			{
				if(flag7==1)
				printf("Nurse %d informs Doctor %d that Adult Patient %d is waiting in the Examination Room %d\n ",index, DoctorFree, nurseExamSheet.id, NurseExamRoom);
				if(flag7==1)
				printf("Nurse %d hands over to the Doctor %d the Examination Sheet of Adult Patient %d\n", index, DoctorFree, nurseExamSheet.id);
			}
			else
			{
				if(flag7==1)
				printf("Nurse %d informs Doctor %d that Child Patient %d is waiting in the Examination Room %d\n ",index, DoctorFree, nurseExamSheet.id, NurseExamRoom);
				if(flag7==1)
				printf("Nurse %d hands over to the Doctor %d the Examination Sheet of Child Patient %d\n", index, DoctorFree, nurseExamSheet.id);
			}
			
			doctorWaitingCV->Signal(doctorRoomLock);
			
			doctorRoomLock->Release();
			
		}
		else					//Doctor is not available
		{
			//Waiting for some Doctor to come.
			nurseCount++;
			
			nurseWaitingForDoctorCV->Wait(doctorRoomLock);

			DoctorFree=doctorIDNurseGlobal;
	
			doctorNurseLock[DoctorFree]->Acquire();
		
			doctorRoomLock->Release();
		
			doctorExamRoomGlobal[DoctorFree] =NurseExamRoom;
			
			if(nurseExamSheet.child==0)
			{
				if(flag7==1)
				printf("Nurse %d informs Doctor %d that Adult Patient %d is waiting in the Examination Room %d\n ",index, DoctorFree, nurseExamSheet.id, NurseExamRoom);
				if(flag7==1)
				printf("Nurse %d hands over to the Doctor %d the Examination Sheet of Adult Patient %d\n", index, DoctorFree, nurseExamSheet.id);
			}
			else
			{
				if(flag7==1)
				printf("Nurse %d informs Doctor %d that Child Patient %d is waiting in the Examination Room %d\n ",index, DoctorFree, nurseExamSheet.id, NurseExamRoom);
				if(flag7==1)
				printf("Nurse %d hands over to the Doctor %d the Examination Sheet of Child Patient %d\n", index, DoctorFree, nurseExamSheet.id);
			}
			
		
			doctorNurseCV[DoctorFree]->Signal(doctorNurseLock[DoctorFree]);
	
			doctorNurseLock[DoctorFree]->Release();

		}
		//--------------------------Nurse-Doctor Ends---------------------------------------//
		
	}
	else
	{
		Task3Lock->Release();
	}
	//--------------------------------Task 3 ends--------------------------------------------//
	

	}//End of While
}

void Doctor(int index)
{

	int doctorExamRoom;
	struct  ExamSheet doctorExamSheet;
    while(1)
	{//-------------------------Doctor-Nurse Interaction--------------------------//

		doctorRoomLock->Acquire();
		
		if(nurseCount==0)					//Waiting for nurse
		{
			//Setting Free
			DoctorMV[index]=0;
			
			//Inserting in Queue
			DoctorQueue[++lastDoctor]=index;

			doctorWaitingCV->Wait(doctorRoomLock);
			
			//Setting Busy
			DoctorMV[index]=1;
			
			doctorExamRoom=doctorExamRoomGlobal[index];
			
			if(flag7==1)
			printf("Doctor %d going to Exam Room %d\n",index, doctorExamRoom);

			//Going to exam room

			if(flag7==1)
			printf("Doctor %d is leaving their Office\n", index);
			
		}
		else								//Signal the Nurse which is Waiting
		{
			nurseCount--;

			//Setting him/her self busy
			
			DoctorMV[index]=1;
		
			doctorNurseLock[index]->Acquire();
	
			//Passing its ID to the Nurse
			doctorIDNurseGlobal=index;

			nurseWaitingForDoctorCV->Signal(doctorRoomLock);
			doctorNurseCV[index]->Wait(doctorNurseLock[index]);

			doctorExamRoom = doctorExamRoomGlobal[index];
			
			if(flag7==1)
			printf("Doctor %d going to Exam Room %d\n",index, doctorExamRoom);
			if(flag7==1)
			printf("Doctor %d is leaving their Office\n", index);

			doctorNurseLock[index]->Release();
		}


		doctorRoomLock->Release();
		
		//---------------------Doctor-Nurse Interaction End----------------------------------//

		//-------------------Doctor-Patient Interaction Start--------------------------------//

		RoomLock[doctorExamRoom]->Acquire();
		
		RoomLockCV[doctorExamRoom]->Signal(RoomLock[doctorExamRoom]);
		RoomLockCV[doctorExamRoom]->Wait(RoomLock[doctorExamRoom]);

		doctorExamSheet=patientExamSheetGlobal[doctorExamRoom];

		if(doctorExamSheet.child==0)
		{
			if(flag6==1)
			printf("Doctor %d is reading the Examination Sheet of Adult Patient %d in Examination Room %d\n",index, doctorExamSheet.id, doctorExamRoom);	
		}
		else
		{
			if(flag6==1)
			printf("Doctor %d is reading the Examination Sheet of Child Patient %d in Examination Room %d\n",index, doctorExamSheet.id, doctorExamRoom);	
		}
		
		if(doctorExamSheet.visit==0)   		//This is first visit
		{

			int probXRay=rand()%100;
			int probShot=rand()%100;
			
			if(0<probXRay<=25)
			{
				doctorExamSheet.xray=1;
				doctorExamSheet.imageCount=(rand()%3 + 1);
			}
		
			if(0<probShot<=25)
			{
				doctorExamSheet.shot=1;
			}
			
			doctorExamSheet.visit=1;

			patientExamSheetGlobal[doctorExamRoom]=doctorExamSheet;
			doctorIDGlobal[doctorExamRoom]=index;

			//Displaying the reports
			if(doctorExamSheet.xray==1 && doctorExamSheet.shot==0)
			{
				if(doctorExamSheet.child==0)
				{
					if(flag6==1)
					printf("Doctor %d notes down in the Sheet that XRay is needed for Adult Patient %d in Examination Room %d\n", index,doctorExamSheet.id, doctorExamRoom);
				}
				else
				{
					if(flag6==1)
					printf("Doctor %d notes down in the Sheet that XRay is needed for Child Patient %d in Examination Room %d\n", index,doctorExamSheet.id, doctorExamRoom);
				}
			}
			else if(doctorExamSheet.xray==0 && doctorExamSheet.shot==1)
			{
				if(doctorExamSheet.child==0)
				{
					if(flag6==1)
					printf("Doctor %d notes down in the Sheet that Adult Patient %d needs to be given a Shot in Examination Room %d\n", index,doctorExamSheet.id, doctorExamRoom);
				}
				else
				{	
					if(flag6==1)
					printf("Doctor %d notes down in the Sheet that Child Patient %d needs to be given a Shot in Examination Room %d\n", index,doctorExamSheet.id, doctorExamRoom);
				}
			}
			else if(doctorExamSheet.xray==1 && doctorExamSheet.shot==1)
			{
				if(doctorExamSheet.child==0)
				{
					if(flag6==1)
					printf("Doctor %d notes down in the Sheet that XRay is needed for Adult Patient %d in Examination Room %d\n", index,doctorExamSheet.id, doctorExamRoom);
					if(flag6==1)
					printf("Doctor %d notes down in the Sheet that Adult Patient %d needs to be given a Shot in Examination Room %d\n", index,doctorExamSheet.id, doctorExamRoom);
				}
				else
				{
					if(flag6==1)
					printf("Doctor %d notes down in the Sheet that XRay is needed for Child Patient %d in Examination Room %d\n", index,doctorExamSheet.id, doctorExamRoom);
					if(flag6==1)
					printf("Doctor %d notes down in the Sheet that Child Patient %d needs to be given a Shot in Examination Room %d\n", index,doctorExamSheet.id, doctorExamRoom);
				}
			}
			else
			{
				if(doctorExamSheet.child==0)
				{
					if(flag6==1)
					printf("Doctor %d diagnoses the Adult Patient %d to be fine and is leaving the Examination Room %d\n",index, doctorExamSheet.id, doctorExamRoom);
				}
				else
				{
					if(flag6==1)
					printf("Doctor %d diagnoses the Child Patient %d to be fine and is leaving the Examination Room %d\n",index, doctorExamSheet.id, doctorExamRoom);
				}	
			}

			RoomLockCV[doctorExamRoom]->Signal(RoomLock[doctorExamRoom]);

			RoomLockCV[doctorExamRoom]->Wait(RoomLock[doctorExamRoom]);
			//Put on Wallpocket

			Task2Lock->Acquire();
				WallPocketExamSheet[doctorExamRoom]=doctorExamSheet;
			Task2Lock->Release();

		}

		else									//Else 2nd Visit
		{
			doctorExamSheet.visit=2;

			patientExamSheetGlobal[doctorExamRoom]=doctorExamSheet;

			doctorIDGlobal[doctorExamRoom]=index;

			if(doctorExamSheet.child==0)
			{
				if(flag6==1)
				printf("Doctor %d is examining the Xrays of Adult Patient %d in the Examination Room %d\n",index, doctorExamSheet.id, doctorExamRoom);
			}
			else
			{
				if(flag6==1)
				printf("Doctor %d is examining the Xrays of Child Patient %d in the Examination Room %d\n",index, doctorExamSheet.id, doctorExamRoom);
			}
			
			RoomLockCV[doctorExamRoom]->Signal(RoomLock[doctorExamRoom]);
			
			RoomLockCV[doctorExamRoom]->Wait(RoomLock[doctorExamRoom]);

			
			//Putting exam sheet on wall pocket
			Task2Lock->Acquire();
			WallPocketExamSheet[doctorExamRoom]=doctorExamSheet;
		
			Task2Lock->Release();

		}
		
		if(flag7==1)
		printf("Doctor %d has left the Examination Room %d\n",index, doctorExamRoom);
		RoomLock[doctorExamRoom]->Release();
		if(flag7==1)
		printf("Doctor %d is going to their Office\n", index);

		//--------------------------Doctor<->patient Interaction done------------------------//

	}//End of while

}

void XRayTechnician(int index)
{
	DEBUG('p',"XT %d entered Xray Room\n", index);
	struct ExamSheet XRayExamSheet;

	while(1)
	{
		XRayExamSheet.id=0;
		
		XRayLock->Acquire();
		
		waitingForXRayLock[index]->Acquire();
		
		if(XRayCount[index]>0)			//Patient Waiting in Line
		{
			XRayLock->Release();
			
			XRayCount[index]--;

			XRayRoomLock[index]->Acquire();
			XRayPatientEnter[index]=1;	//Enters 
			
			if(PatientXRayEnter[index]==0)
			{
				printf("XRay Technician %d is in If\n",index);
				//XRayCount[patientXRayRoom]++;
				//printf("XRayCount %d is %d\n",patientXRayRoom, XRayCount[patientXRayRoom]);
				waitingForXRayCV[index]->Wait(waitingForXRayLock[index]);
				PatientXRayEnter[index]--;
				//waitingForXRayCV[index]->Signal(waitingForXRayLock[index]);
			}
			else
			{
				printf("XRay Technician %d is in Else\n",index);
				waitingForXRayCV[index]->Signal(waitingForXRayLock[index]);
				waitingForXRayCV[index]->Wait(waitingForXRayLock[index]);
				PatientXRayEnter[index]--;
			}
			
			printf("XRay Technician %d signals the waiting patient\n",index);
			waitingForXRayCV[index]->Signal(waitingForXRayLock[index]);
			
			waitingForXRayLock[index]->Release();
		
			//Waiting for Patient to talk
			XRayRoomCV[index]->Wait(XRayRoomLock[index]);
		
			//Take Examsheet From patient
			XRayExamSheet=XRayExamSheetGlobal[index];
			if(XRayExamSheet.child==0)
			{
				if(flag7==1)
				printf("XRay Technician %d got Exam Sheet %d from Adult Patient\n",index, XRayExamSheet.id);
			}
			else
			{
				if(flag7==1)
				printf("XRay Technician %d got Exam Sheet %d from Parent\n",index, XRayExamSheet.id);
			}
			
			if(XRayExamSheet.child==0)
			{
				if(flag7==1)
				printf("Xray Technician %d telling Adult Patient %d to move onto the table\n",index, XRayExamSheet.id);
			}
			else
			{
				if(flag7==1)
				printf("Xray Technician %d telling Child Patient %d to move onto the table\n",index, XRayExamSheet.id);
			}
			
			XRayRoomCV[index]->Signal(XRayRoomLock[index]);
		
			//XRayRoomCV[index]->Wait(XRayRoomLock[index]);
		}

		else
		{
			printf("XRay Technician %d is in Outer Else\n",index);
			XRayRoomLock[index]->Acquire();
		
			waitingForXRayLock[index]->Release();
		

			//Freeing Himself
			//XRayLock->Acquire();
				XRayRoomMV[index]=0;
			XRayLock->Release();

			//Waiting for Nurse to come
			XRayRoomNurseCV[index]->Wait(XRayRoomLock[index]);

			//Take Exam Sheet from Nurse
			XRayExamSheet=XRayExamSheetGlobal[index];
			if(XRayExamSheet.child==0)
			{
				if(flag7==1)
				printf("XRay Technician %d got Exam Sheet %d from Nurse\n",index, XRayExamSheet.id);
			}
			else
			{
				if(flag7==1)
				printf("XRay Technician %d got Exam Sheet %d from Nurse\n",index, XRayExamSheet.id);
			}
		
			
			XRayRoomNurseCV[index]->Signal(XRayRoomLock[index]);
			//XRayTechnician-Nurse Interaction ends
			
			//XRayTechnician-Patient Interaction starts
			XRayRoomCV[index]->Wait(XRayRoomLock[index]);
			if(XRayExamSheet.child==0)
			{
				if(flag7==1)
				printf("Xray Technician %d telling Adult Patient %d to move onto the table\n",index, XRayExamSheet.id);
			}
			else
			{
				if(flag7==1)
				printf("Xray Technician %d telling Child Patient %d to move onto the table\n",index, XRayExamSheet.id);
			}
			
			XRayRoomCV[index]->Signal(XRayRoomLock[index]);
		}

		//Taking XRay images
		if(XRayExamSheet.imageCount==1)
		{
			XRayRoomCV[index]->Wait(XRayRoomLock[index]);
			
			if(XRayExamSheet.child==0)
			{
				if(flag7==1)
				printf("XRayTechnician %d taking XRay of Adult Patient %d\n",index, XRayExamSheet.id);
			}
			else
			{
				if(flag7==1)
				printf("XRayTechnician %d taking XRay of Child Patient %d\n",index, XRayExamSheet.id);
			}
			
			imageGlobal[index] = xrayResult[((rand()%3)+1)];
			XRayRoomCV[index]->Signal(XRayRoomLock[index]);
			
			if(XRayExamSheet.child==0)
			{
				if(flag7==1)
				printf("Xray Technician %d records %d on Adult Patient %d's examination sheet\n",index,imageGlobal[index], XRayExamSheet.id);
			}
			else
			{
				if(flag7==1)
				printf("Xray Technician %d records %d on Child Patient %d's examination sheet\n",index, imageGlobal[index], XRayExamSheet.id);
			}
			
		}
		else if(XRayExamSheet.imageCount==2)
		{
			
			//1st Image 
			XRayRoomCV[index]->Wait(XRayRoomLock[index]);
			
			if(XRayExamSheet.child==0)
			{
				if(flag7==1)
				printf("XRayTechnician %d taking XRay of Adult Patient %d\n",index, XRayExamSheet.id);
			}
			else
			{
				if(flag7==1)
				printf("XRayTechnician %d taking XRay of Child Patient %d\n",index, XRayExamSheet.id);
			}
			
			if(XRayExamSheet.child==0)
			{
				if(flag7==1)
				printf("XRay Technician %d asks Adult Patient %d to move\n",index, XRayExamSheet.id);
			}
			else
			{
				if(flag7==1)
				printf("XRay Technician %d asks Child Patient %d to move\n",index, XRayExamSheet.id);
			}
			
			XRayRoomCV[index]->Signal(XRayRoomLock[index]);
			XRayRoomCV[index]->Wait(XRayRoomLock[index]);
				
			imageGlobal[index] = xrayResult[((rand()%3)+1)];
			
			XRayRoomCV[index]->Signal(XRayRoomLock[index]);
			if(XRayExamSheet.child==0)
			{
				if(flag7==1)
				printf("Xray Technician %d records %d on Adult Patient %d's examination sheet\n",index,imageGlobal[index], XRayExamSheet.id);
			}
			else
			{
				if(flag7==1)
				printf("Xray Technician %d records %d on Child Patient %d's examination sheet\n",index, imageGlobal[index], XRayExamSheet.id);
			}
			
			
		}
		else if(XRayExamSheet.imageCount==3)
		{	
			//1st Image
			XRayRoomCV[index]->Wait(XRayRoomLock[index]);
			
			if(XRayExamSheet.child==0)
			{
				if(flag7==1)
				printf("XRayTechnician %d taking XRay of Adult Patient %d\n",index, XRayExamSheet.id);
			}
			else
			{
				if(flag7==1)
				printf("XRayTechnician %d taking XRay of Child Patient %d\n",index, XRayExamSheet.id);
			}
			
			if(XRayExamSheet.child==0)
			{
				if(flag7==1)
				printf("XRay Technician %d asks Adult Patient %d to move\n",index, XRayExamSheet.id);
			}
			else
			{
				if(flag7==1)
				printf("XRay Technician %d asks Child Patient %d to move\n",index, XRayExamSheet.id);
			}
			
			XRayRoomCV[index]->Signal(XRayRoomLock[index]);
			XRayRoomCV[index]->Wait(XRayRoomLock[index]);
			
			if(XRayExamSheet.child==0)
			{
				if(flag7==1)
				printf("XRay Technician %d asks Adult Patient %d to move\n",index, XRayExamSheet.id);
			}
			else
			{
				if(flag7==1)
				printf("XRay Technician %d asks Child Patient %d to move\n",index, XRayExamSheet.id);
			}
			
			XRayRoomCV[index]->Signal(XRayRoomLock[index]);
			XRayRoomCV[index]->Wait(XRayRoomLock[index]);
			
			imageGlobal[index] = xrayResult[((rand()%3)+1)];
			XRayRoomCV[index]->Signal(XRayRoomLock[index]);
			
			if(XRayExamSheet.child==0)
			{
				if(flag7==1)
				printf("Xray Technician %d records %d on Adult Patient %d's examination sheet\n",index,imageGlobal[index], XRayExamSheet.id);
			}
			else
			{
				if(flag7==1)
				printf("Xray Technician %d records %d on Child Patient %d's examination sheet\n",index, imageGlobal[index], XRayExamSheet.id);
			}
		}
		else
		{
			
		}
		
		//Waiting for Patient to Signal that he acquired Waiting Lock
		XRayRoomCV[index]->Wait(XRayRoomLock[index]);
		
		Task3Lock->Acquire();
		
		//Hanging patients form on the wallpocket
		if(index==1)
		{
			last1++;
			XRayWallPocket1[last1]=XRayExamSheet;
		}
		else
		{
			last2++;
			XRayWallPocket2[last2]=XRayExamSheet;	
		}
		
		Task3Lock->Release();
		
		
		//----------------------XRay technicians Job is done-------------------------------//
		XRayRoomLock[index]->Release();
	    
	}//End of While

}

void Cashier(int index)
{
	struct ExamSheet cashierExamSheet;
	int patientSize=0;
	
	while(1)
	{
		
		cashCounterLock->Acquire();	
			
		if(patientWaitingForCashierCount>0)	//Patients are in line
		{
			//Waking the patient
			cashCounterCV->Signal(cashCounterLock);
			patientWaitingForCashierCount--;
			
			cashierState=1;
		}
		else
		{
			cashierState=0;
		}
		
		cashierLock->Acquire();
		
		cashCounterLock->Release();
		
		//#--------------------Cashier<-->Patient Interaction---------------------------------#
		
		cashierCV->Wait(cashierLock);
		
		//Taking Form from Patient
		patientSize++;
		
		cashierExamSheet=cashierExamSheetGlobal;
		if(cashierExamSheet.child==0)
		{
			if(flag3==1)
			printf("Cashier recieves the examination sheet from Adult Patient %d\n", cashierExamSheet.id);
		}
		else
		{
			if(flag3==1)
			printf("Cashier recieves the examination sheet for Child Patient %d from Parent %d\n", cashierExamSheet.id, cashierExamSheet.id);
		}
		
		//Calculating the fees
		feesGlobal=50;
		if(cashierExamSheet.shot==1)
		{
			feesGlobal=feesGlobal+50;
		}
		
		if(cashierExamSheet.imageCount==1)
		{
			feesGlobal=feesGlobal+50;
		}
		else if(cashierExamSheet.imageCount==2)
		{
			feesGlobal=feesGlobal+100;
		}
		else if(cashierExamSheet.imageCount==3)
		{
			feesGlobal=feesGlobal+150;
		}
		else{};
		
		
		if(cashierExamSheet.child==0)
		{
			if(flag3==1)
			printf("Cashier reads the examination sheet of Adult Patient %d and asks him to pay $%f\n",cashierExamSheet.id, feesGlobal);
		}
		else
		{
			if(flag3==1)
			printf("Cashier reads the examination sheet of Child Patient %d and asks Parent %d to pay $%f\n",cashierExamSheet.id,cashierExamSheet.id, feesGlobal);
		}
		
		cashierCV->Signal(cashierLock);
		cashierCV->Wait(cashierLock);
		
		if(cashierExamSheet.child==0)
		{
			if(flag3==1)
			printf("Cashier accepts $%f from Adult Patient %d\n", feesGlobal, cashierExamSheet.id);
		}
		else
		{
			if(flag3==1)
			printf("Cashier accepts $%f from Parent %d\n", feesGlobal, cashierExamSheet.id);
		}
		
		if(cashierExamSheet.child==0)
		{
			if(flag3==1)
			printf("Cashier gives a receipt of $%f to Adult Patient %d\n", feesGlobal, cashierExamSheet.id);
		}
		else
		{
			if(flag3==1)
			printf("Cashier gives a receipt $%f from Parent %d\n", feesGlobal, cashierExamSheet.id);
		}
		
		cashierCV->Signal(cashierLock);
		cashierCV->Wait(cashierLock);
		//#--------------------Cashier<-->Patient Interaction done---------------------------------#
		
		cashierLock->Release();
		
		if(patientSize==n)
		{
			printf("All Patients exited the Hospital\n");
			exit(0);
		}
		
	}//End of While
}


void DoctorSimulation()
{

	printf("\n\nEntering the simulation\n");
	DEBUG('ds',"Entering the simulation\n");

	//WallPocketExamSheet
	for(int i=1; i<101; i++)
	{
		nurseXRayPatientLock[i]=new Lock(" ");
		nurseXRayPatientCV[i]=new Condition(" ");
		childPatientLock[i]=new Lock(" ");
		childPatientCV[i]=new Condition(" ");
	}

	for(int i=1; i<6; i++)
	{
		WallPocketExamSheet[i].id=0;
		RoomLock[i] = new Lock(" ");
		RoomLockCV[i]=new Condition(" ");
	}
	
	for(int i=1;i<4;i++)
	{
		doctorNurseLock[i]=new Lock(" ");
		doctorNurseCV[i]=new Condition(" ");
	}
	
	for(int i=1; i<3;i++)
	{
		XRayRoomLock[i] = new Lock(" ");
		XRayRoomCV[i]=new Condition(" ");
		XRayRoomNurseCV[i]=new Condition(" ");
		waitingForXRayLock[i] = new Lock(" ");
		waitingForXRayCV[i]=new Condition(" ");
		XRayWaitingRoomLock[i]=new Lock(" ");
		XRayWaitingRoomCV[i]=new Condition(" ");
		
	}

	//WRN Thread
	Thread *wrnT=new Thread(" ");
	wrnT->Fork(waitingRoomNurse,0);

	//Cashier Thread
	Thread *cT=new Thread(" ");
	cT->Fork(Cashier,0);
	
		flag1=1;
		flag2=1;
		flag3=1;
		flag4=1;
		flag5=1;
		flag6=1;
		flag7=1;
		
		patients=30;
		parents=child=30;
		nurses=5;
		doctors=3;
		xray=2;
		examrooms=5;
		
		child = parents;
			
		n=patients+parents;
		
		//Call fork to all threads as per inputs
		/*for(int i=1;i<=patients;i++)
		{
			Thread *t=new Thread(" ");
			t->Fork(Patient,i);
		}*/
	
		for(int i=1;i<=parents;i++)
		{
			Thread *t=new Thread(" ");
			t->Fork(Patient,i);
			Thread *pt=new Thread(" ");
			pt->Fork(Parent,i);
			Thread *ct=new Thread(" ");
			ct->Fork(Child,i);
		}
	
		/*for(int i=patients;i<=parents;i++)
		{
			Thread *pt=new Thread(" ");
			pt->Fork(Parent,i);
		}*/

		for(int i=1;i<=nurses;i++)
		{
			Thread *nt=new Thread(" ");
			nt->Fork(Nurse,i);
		}

		for(int i=1;i<=doctors;i++)
		{
			Thread *dt=new Thread(" ");
			dt->Fork(Doctor,i);
		}
		
		for(int i=1;i<=xray;i++)
		{
			Thread *xt=new Thread(" ");
			xt->Fork(XRayTechnician, i);
		}
		
	
	
}

