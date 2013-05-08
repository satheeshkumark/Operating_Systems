/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"


#define printf(a) Write(a, sizeof(a), ConsoleOutput)
void Display()
{
	printf("fork is working");
	printf("hi");
	Exit(0);

}

int main()
 {

	printf("hello");

	Fork(Display);

   
}
