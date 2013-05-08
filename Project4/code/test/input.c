#include "syscall.h"
#include "setup.h"

int main()
{
	int testLock;
	int testMV;
	int i;
	
	/*for(i=0;i<100;i++){
		testLock = CreateMV("mv",3,0,i);
		Print2("\n Lock%d : %d",i,testLock);
	}*/
	
	/*testLock = CreateLock("lk",3,3);
	Print1("\n Lock : %d",testLock);
	
	testLock = CreateLock("lk",3,4);
	Print1("\n Lock : %d",testLock);
	
	testLock = CreateLock("lj",3,5);
	Print1("\n Lock : %d",testLock);
	
	testLock = CreateLock("wlk",5,10);
	testLock = atoi("name");
	Print1("\n Lock : %d",testLock);
	testLock = atoi("name");
	Print1("\n Lock : %d",testLock);*/
	initialize();

	/*testLock = CreateLock("mv",3,i);
	Print1("\n Lock : %d",testLock);
	
	testMV = CreateMV("mv",3,5,5);
	
		
	AcquireLock(testLock);
	i = GetMV(testMV);
	Print2("\n MV%d value : %d",testMV,i);
	SetMV(testMV,10);
	i = GetMV(testMV);
	Print2("\n Set MV%d value : %d",testMV,i);
	SetMV(testMV,5);
	ReleaseLock(testLock);*/
	Exit(0);
}
