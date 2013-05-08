#include "syscall.h"
#include "setup.h"
int main()
{
	int i;
			
	for(i=0;i<goodsloaderCount;i++)
		Exec("../test/goodsloader",19);
		
	Exit(0);
	
}