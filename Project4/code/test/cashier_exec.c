#include "syscall.h"
#include "setup.h"
int main()
{
	int i;
	
	for(i=0;i<cashierCount;i++)
		Exec("../test/cashier",15);
		
	Exit(0);
	
}