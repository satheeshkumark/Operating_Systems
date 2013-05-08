#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    int a=CreateLock("lk",3,0);
	int i;
	
	AcquireLock(a);
	Print1("\nLOCK%d ACQUIRED",a);
	Print("\nEnter a number : ");
	i = scan();
	Exit(0);
}
