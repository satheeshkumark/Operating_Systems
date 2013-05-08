/*TEST FOR LOCK AND CV*/
/*LOCK AND CV GREATER THAN THEIR RANGE*/

#include "syscall.h"

void thread1(void);
void thread2(void);

int a,b,count;
void main()
{
	a = CreateLock("lock1",5);
	b = CreateCV("cond1",5);

	Fork(thread1);
}

void thread1()
{
    AcquireLock(a+101);
	ReleaseLock(a+101);
	Exit(0);
}




