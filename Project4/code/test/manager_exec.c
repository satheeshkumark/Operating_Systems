#include "syscall.h"
#include "setup.h"
int main()
{
	int i;
	
	Exec("../test/manager",15);
	Exit(0);
}