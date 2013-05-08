 /*lock Acquired by one thread so all other should wait for the thread to release the lock*/
#include "syscall.h"
int a,b,count;

void thread1(void);
void thread2(void);
void thread3(void);

void main()
 {
	a = CreateLock("lock1",5);
	b = CreateCV("cond1",5);
	count =0;
	Fork(thread1);
	Fork(thread2);
	Fork(thread3);
 }

void thread1()
{
    AcquireLock(a);
	Yield();
	Yield();
	Yield();
	Yield();
	ReleaseLock(a);
	Exit(0);
}

void thread2()
{
	AcquireLock(a);
	ReleaseLock(a);
	Exit(0);
}

void thread3()
{
    AcquireLock(a);
	Yield();
	Yield();
	Exit(0);
}
