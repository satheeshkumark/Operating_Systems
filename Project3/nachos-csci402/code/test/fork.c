#include "syscall.h"

void sort()
{
	
	int A[1024];
	
    int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < 1024; i++){		
        A[i] = 1024 - i;
		Print1("\nInside sort: %d",A[i]);
	}

    /* then sort! */
    for (i = 0; i < 1023; i++)
        for (j = i; j < (1023 - i); j++)
	   if (A[j] > A[j + 1]) {	/* out of order -> need to swap ! */
	      tmp = A[j];
	      A[j] = A[j + 1];
	      A[j + 1] = tmp;
    	   }
	Exit(A[0]);		/* and then we're done -- should be 0! */
}

int main()
 {
	Print("\nIn main thread");
	Fork(sort);
	Exit(0);   
}
