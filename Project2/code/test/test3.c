/*TEST FOR LOCK AND CV*/
#include"syscall.h"
int a,b;
int count =0;
void thread1(void);
void thread2(void);

void thread3(void);
void thread4(void);
void main()
{
	a = CreateLock("lock1",5);
	b = CreateCV("cond1",5);
	Fork(thread1);
	Fork(thread2);
	Fork(thread3);
	Fork(thread4);

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
	AcquireLock(a);
	WaitCV(b,a);
	ReleaseLock(a);
	Exit(0);
}

void thread3()
{
	AcquireLock(a);
	WaitCV(b,a);
	ReleaseLock(a);
	Exit(0);
}

void thread4()
{
	AcquireLock(a);
	Yield();
	Yield();
	Yield();
	BroadcastCV(b,a);
	ReleaseLock(a);
	Exit(0);
}
