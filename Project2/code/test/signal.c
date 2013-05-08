#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    int x= CreateCV("firstq",6);
	int y= CreateLock("first",5);
	AcquireLock(y);
	SignalCV(y,x);
	WaitCV(y,x);
	SignalCV(y,x);
	ReleaseLock(y);
	
    
}
