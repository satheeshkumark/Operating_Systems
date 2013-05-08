#include "syscall.h"

int a,b,count;
void thread1(void);
void thread2(void);


void thread1(void)
{
    AcquireLock(a);
	if(count ==0)
	{
		count++;
		WaitCV(b,a);
		
	}

	else
	{
		count--;
		SignalCV(b,a);
	}	
	ReleaseLock(a);
	Exit(0);
}


void thread2()
{
	AcquireLock(a);
	if(count > 0)
	{
		count--;
		SignalCV(b,a);
		
	}
	else
	{
		count++;
		WaitCV(b,a);
       
	}
	ReleaseLock(a);
	Exit(0);
}

int main()
{
	count =0;
	
	a = CreateLock("lock1",5);
	b = CreateCV("cond1",5);
	Fork(thread1);
	Fork(thread2);
	Yield();
	Yield();
	Yield();
}