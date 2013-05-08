#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    int x= CreateCV("firstq",6);
	int y= CreateLock("first",5);
	/*WaitCV(y,x);*/
	BroadcastCV(y,x);
    
}
