#include "syscall.h"

int a,b,count;
void thread1(void);


void main()
{
	a = CreateLock("lock1",5);
	b = CreateCV("cond1",5);
    Fork(thread1);

}

void thread1()
{
	DestroyLock(a);
	DestroyCV(b);
	Exit(0);
}




