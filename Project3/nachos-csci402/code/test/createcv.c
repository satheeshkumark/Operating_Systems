#include "syscall.h"
int a[3];
int b, c;

int
main()
{
    int x= CreateMV("firstq",6,6);
	SetMV(x,10);
	Print2("The value of the monitor variable %d is %d",x,GetMV(x));
	DestroyMV(x);
    Exit(0);
}
