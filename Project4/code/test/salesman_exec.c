#include "syscall.h"
#include "setup.h"
int main()
{
	int i;
	
	for(i=0;i<(departmentsCount*salesmanCount);i++)
		Exec("../test/salesman",16);
	
	Exit(0);
}