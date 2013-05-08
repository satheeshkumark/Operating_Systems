
#include "syscall.h"


#define Dim 	20	/* sum total of the arrays doesn't fit in 
			 * physical memory 
			 */

int A1[Dim][Dim];
int B1[Dim][Dim];
int C1[Dim][Dim];

int A2[Dim][Dim];
int B2[Dim][Dim];
int C2[Dim][Dim];

int D1[1024];
int D2[1024];

void matmult1()
{
    int i, j, k;

    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
	for (j = 0; j < Dim; j++) {
	     A1[i][j] = i;
	     B1[i][j] = j;
	     C1[i][j] = 0;
	}
    for (i = 0; i < Dim; i++){	/* then multiply them together */
	for (j = 0; j < Dim; j++){
            for (k = 0; k < Dim; k++){
		 C1[i][j] += A1[i][k] * B1[k][j];
		 }
	}
	}
		 
	
    Exit(C1[Dim-1][Dim-1]);		/* and then we're done */
}

void matmult2()
{
    int i, j, k;

    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
	for (j = 0; j < Dim; j++) {
	     A2[i][j] = i;
	     B2[i][j] = j;
	     C2[i][j] = 0;
	}
    for (i = 0; i < Dim; i++){	/* then multiply them together */
	for (j = 0; j < Dim; j++){
            for (k = 0; k < Dim; k++){
		 C2[i][j] += A2[i][k] * B2[k][j];
		 }
	}
	}
		 
	
    Exit(C2[Dim-1][Dim-1]);		/* and then we're done */
}

void sortTest1()
{
	int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < 1024; i++){		
        D1[i] = 1024 - i;
	}

    /* then sort! */
    for (i = 0; i < 1023; i++)
        for (j = i; j < (1023 - i); j++)
	   if (D1[j] > D1[j + 1]) {	/* out of order -> need to swap ! */
	      tmp = D1[j];
	      D1[j] = D1[j + 1];
	      D1[j + 1] = tmp;
    	   }
	Exit(D1[0]);		/* and then we're done -- should be 0! */
}

void sortTest2()
{
	int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < 1024; i++){		
        D2[i] = 1024 - i;
	}

    /* then sort! */
    for (i = 0; i < 1023; i++)
        for (j = i; j < (1023 - i); j++)
	   if (D2[j] > D2[j + 1]) {	/* out of order -> need to swap ! */
	      tmp = D2[j];
	      D2[j] = D2[j + 1];
	      D2[j + 1] = tmp;
    	   }
	Exit(D2[0]);		/* and then we're done -- should be 0! */
}
int main(){

	int choice;
	PrintTest("\n---------------Menu for test cases--------");
	PrintTest("\n1. Matmult - Single Exec");
	PrintTest("\n2. Sort - Single Exec");
	PrintTest("\n3. Matmult - Single Fork");
	PrintTest("\n4. Sort - Single Fork");
	PrintTest("\n5. Matmult - Double Exec");
	PrintTest("\n6. Sort - Double Exec");
	PrintTest("\n7. Matmult - Double Fork");
	PrintTest("\n8. Sort - Double Fork");
	PrintTest("\n\nEnter your choice : ");
	choice=scan();
	
	switch(choice){
		case 1:
			Exec("../test/matmult",15);
			break;
		
		case 2:
			Exec("../test/sort",12);
			break;
		
		case 3:
			Fork(matmult1);
			break;
		
		case 4:
			Fork(sortTest1);
			break;
		
		case 5:
			Exec("../test/matmult",15);
			Exec("../test/matmult",15);
			break;
		
		case 6:
			Exec("../test/sort",12);
			Exec("../test/sort",12);
			break;
		
		case 7:
			Fork(matmult1);
			Fork(matmult2);
			break;
		
		case 8:
			Fork(sortTest1);
			Fork(sortTest2);
			break;
		
		default:
			Print("\n Oops. Invalid choice");
			break;
	}
}