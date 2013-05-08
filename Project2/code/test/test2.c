#include "syscall.h"
int a,b,count;

void thread1(void);
void thread2(void);
void main()
{
	count = 0;
	a = CreateLock("lock1",5);
	b=a+2;
	Fork(thread1);
	Yield();
}

void thread1()
{
    AcquireLock(b); /*Should give an error*/
	Exit(0);
}



