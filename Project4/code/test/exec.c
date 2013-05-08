/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int main()
{
	int i;
	
	for(i=0;i<5;i++){
		Exec("../test/createlock",18);
	}
	Exit(0);
   
}
