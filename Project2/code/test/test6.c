/*TEST FOR WAIT AND CV*/
/*One thread waiting in one lock*/
/*Other thread signalling from another lock*/

#include "syscall.h"
void thread1(void);
void thread2(void);

int a,b,c,count;

void main()
 {
	a = CreateLock("lock1",5);
	b = CreateCV("cond1",5);
	c = CreateLock("lock2",5);
	count =0;
	Fork(thread1);
	Fork(thread2);
}

void thread1()
{
    AcquireLock(a);
	WaitCV(b,a);
	ReleaseLock(a);
	Exit(0);
}

void thread2()
{
	Yield();
	Yield();
	AcquireLock(a);
	SignalCV(b,c);
	ReleaseLock(a);
	Exit(0);
}
