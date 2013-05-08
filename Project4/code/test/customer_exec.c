#include "syscall.h"
#include "setup.h"
int main()
{
	int i;
	
	for(i=0;i<customersCount;i++)
		Exec("../test/customer",16);
	Exit(0);
}