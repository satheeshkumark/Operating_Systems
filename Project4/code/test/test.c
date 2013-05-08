#include "syscall.h"
int testLock;
int testLock2;

int testCV;

int fork_no=0;
int count=0;
int number;

void testFork()
{
	PrintTest("\nHello");
	Exit(0);
}

void customerThread()
{
    AcquireLock(testLock);
	PrintTest("\nCustomer acquired the lock ");
	if(count==0)
	{
		count++;
		PrintTest("\nCustomer waiting for Salesman signal ");
		WaitCV(testCV,testLock);
		
	}
	else
	{
		count--;
		PrintTest("\nCustomer signalling Salesman ");
		SignalCV(testCV,testLock);
	}	
	ReleaseLock(testLock);
	PrintTest("\nCustomer released the lock ");
	Exit(0);
}


void salesmanThread()
{
	AcquireLock(testLock);
	PrintTest("\nSalesman acquired the lock ");
	if(count > 0)
	{
		count--;
		PrintTest("\nSalesman signalling Customer ");
		SignalCV(testCV,testLock);
		
	}
	else
	{
		count++;
		PrintTest("\nSalesman waiting for Customer signal ");
		WaitCV(testCV,testLock);
       
	}
	ReleaseLock(testLock);
	PrintTest("\nSalesman released the lock ");
	Exit(0);
}

void yieldThread() {
	
	int myIndex,i;
	
	AcquireLock(testLock);
	myIndex = count++;
	ReleaseLock(testLock);
	Print1("Thread %d is running ",myIndex);
	for(i=0;i<3000;i++) 
	{
		Yield();
	}
	Exit(0);
}
void main(){

	int i,choice;
	PrintTest("\n/---------Menu for test cases--------/");
	PrintTest("\n1. Creating maximum locks (>1000) and CVs(>1000)");
	PrintTest("\n2. Creating lock with invalid length (500)");
	PrintTest("\n3. Destroying a created lock");
	PrintTest("\n4. Acquiring and releasing a lock");
	PrintTest("\n5. Acquiring and releasing an invalid lock");
	PrintTest("\n6. Acquiring and releasing a destroyed lock");
	PrintTest("\n7. Executing maximum processes (>10)");
	PrintTest("\n8. Forking maximum threads (>1000)");
	PrintTest("\n9. Wait and signal using fork - Customer waiting, Salesman signalling");
	PrintTest("\n10. Wait and Signal using fork - Salesman waiting, Customer signalling");
	PrintTest("\n11. Scanning a number and printing it once,twice and thrice");
	PrintTest("\n12. Yield test (Works without -rs)");
	PrintTest("\n13. Exit test");
	PrintTest("\n14. Single Simulation of Project1");
	PrintTest("\n15. Multiple Simulation of Project1 - 2 simulations");
	PrintTest("\n16. Multiple Simulation of Project1 - 3 simulations");
	PrintTest("\n\n\nYour choice :  ");
	choice=scan();
	
	switch(choice){
	
		case 1: /*1. Creating maximum locks (>1000)*/
				for(i=0;i<1200;i++){
					testLock=CreateLock("test1",5);
					testCV=CreateCV("cv1",3);
				}					
				break;
		
		case 2: /*2. Creating lock with invalid length (500)*/
				testLock=CreateLock("test",4);
				testLock=CreateLock("test",4);
				break;
		
		case 3: /*3. Destroying a created lock*/
				testLock=CreateLock("test2",5);
				DestroyLock(testLock);
				break;
		
		case 4: /*4. Acquiring and releasing a lock*/
				testLock=CreateLock("test2",5);
				AcquireLock(testLock);
				ReleaseLock(testLock);
				break;
		
		case 5: /*5. Acquiring a lock and releasing another lock*/
				testLock=CreateLock("test1",5);
				testLock2=1500;
				AcquireLock(testLock2);
				ReleaseLock(testLock2);
				break;
				
		case 6: /*6. Acquiring and releasing a destroyed lock*/
				testLock=CreateLock("test2",5);
				DestroyLock(testLock);
				AcquireLock(testLock);
				ReleaseLock(testLock);
				break;
		
		case 7: /*7. Executing maximum processes (>10)*/
				for(i=0;i<15;i++)
					Exec("../test/input",13);
				break;
		
		case 8: /*8. Forking maximum threads (>1000)*/
				for(i=0;i<10;i++)
					Fork(testFork);
				break;
				
		case 9: /*9. Wait and signal using fork - Customer waiting, Salesman signalling*/
				testLock=CreateLock("test1",5);
				testCV=CreateCV("cv1",3);
				Fork(customerThread);
				Fork(salesmanThread);
				break;
				
		case 10: /*10. Wait and Signal using fork - Salesman waiting, Customer signalling*/
				testLock=CreateLock("test1",5);
				testCV=CreateCV("cv1",3);
				Flag();
				Fork(salesmanThread);
				Fork(customerThread);
				break;
				
		case 11: /*11. Scanning a number and printing it once,twice and thrice*/
				PrintTest("\nEnter a number: ");
				number=scan();
				Print1("Print with one argument : %d",number);
				Print2("Print with two arguments : %d %d",number,number);
				Print3("Print with three arguments : %d %d %d ",number,number,number);
				break;
		
		case 12: /*12. Yield test*/
				Flag();
				testLock=CreateLock("test1",5);
				Fork(yieldThread);
				Fork(yieldThread);
				break;
				
		case 13: /*13. Exit test*/
				Exit(0);
				break;
				
		case 14: /*14. Single Simulation of Project1*/
				Exec("../test/project2",sizeof("../test/project2"));
				break;
		
		case 15: /*15. Multiple Simulation of Project1 - 2 simulations*/
				Exec("../test/project2",sizeof("../test/project2"));
				Exec("../test/project2",sizeof("../test/project2"));
				break;
				
		case 16: /*16. Multiple Simulation of Project1 - 3 simulations*/
				Exec("../test/project2",sizeof("../test/project2"));
				Exec("../test/project2",sizeof("../test/project2"));
				Exec("../test/project2",sizeof("../test/project2"));
				break;
				
		default: PrintTest("\nWrong choice. Please run the program again");
	}
	Exit(0);
}		
			
			