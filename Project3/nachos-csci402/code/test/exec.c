/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"


#define printf(a) Write(a, sizeof(a), ConsoleOutput)

int main()
{
	int a=1;
	printf("\ninside exec main function");
    switch(a){
	case 1:
	Exec("../test/fork",12);
	Print("Process id : 0 ");
	
	Exec("../test/fork",12);
	Print("Process id : 1");
	break;
	default:
	Print("\nWrong option");
	break;
	}
	Exit(0);
   
}
