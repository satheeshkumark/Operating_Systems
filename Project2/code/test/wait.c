#include "syscall.h"
int a[3];
int b, c;

    int x;
	int y;

void hi()
{
 
	AcquireLock(y);
	
	SignalCV(y,x);
		ReleaseLock(y);
    
}

void hello()
 {
   AcquireLock(y);
   WaitCV(y,x);
   ReleaseLock(y);
   
 }

int main()
{
	x= CreateCV("firstq",6);
	y= CreateLock("first",5);
	hello();
	hi();
	

}
