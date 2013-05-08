/* TEST FOR FORK.*/
/*fork 100 threads.*/

#include "syscall.h"

int a,b,count,j;
void thread1(void);

void main()
{
	a = CreateLock("lock1",5);
	b = CreateCV("cond1",5);
	count =0;
	for( j=0;j<100;j++)
	{
		Fork(thread1);
	}
}

void thread1()
{
    AcquireLock(a);
	WaitCV(b,a);
	ReleaseLock(a);
	Exit(0);
}
