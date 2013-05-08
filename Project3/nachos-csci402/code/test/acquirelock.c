#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    int a=CreateLock("hi",2);
	
	AcquireLock(a);
	/*AcquireLock(a);*/
	ReleaseLock(a);
	ReleaseLock(a);
}
