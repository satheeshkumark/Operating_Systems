/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"


#define printf(a) Write(a, sizeof(a), ConsoleOutput)

int main()
{
	printf("\ninside exec main function");
    
	Exec("../test/input",13);
	printf("Process id : 0 ");
	
	Exec("../test/input",13);
	printf("Process id : 1");
   
}
