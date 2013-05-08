#include "syscall.h"

int main()
{
    int x;
	int i;
	
	for(i=0;i<10;i++){
		x= CreateLock("firs",10,i);
		Print1("\nLock Created : %d",x);
	}
	Exit(0);
 }
