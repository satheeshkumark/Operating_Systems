
int testLock;
int testLock2;

int testCV;

int testMV;
int testMV2;
int mvValue;

int main(){

	int choice;
	int i;
	int j=0;
	int k;
	int m=0;
	char name[5];
	PrintTest("\n/---------Menu for test cases--------/");
	PrintTest("\n1. Creating maximum locks (>1000)");
	PrintTest("\n2. Creating lock with invalid length (500)");
	PrintTest("\n3. Creating lock with negative index(-1)");
	PrintTest("\n4. Destroying a created lock");
	PrintTest("\n5. Acquiring and releasing a lock");
	PrintTest("\n6. Acquiring and releasing an invalid lock");
	PrintTest("\n7. Acquiring and releasing a destroyed lock");
	PrintTest("\n8. Creating maximum CVs(>1000)");
	PrintTest("\n9. Creating CV with invalid length (500)");
	PrintTest("\n10. Creating CV with negative length(-1)");
	PrintTest("\n11. Destroying a created CV"); 
	PrintTest("\n12. Create maximum MVs (>1000)");
	PrintTest("\n13. Creating MV with invalid length (500)");
	PrintTest("\n14. Creating MV with negative length(-1)");
	PrintTest("\n15. Setting and getting MV");
	PrintTest("\n16. Destroying a created MV");
	PrintTest("\n17. Setting value for a destroyed MV"); 
	PrintTest("\n\n\nYour choice :  ");
	choice=scan();
	
	switch(choice){
	
		case 1:/*Creating maximum locks (>1000)*/
			for(i=0;i<1010;i++){
				j=1000+i;
				for(k=0;k<4;k++){
					m=(j%10)+1;
					name[k]=m;
					j/=10;
				}
				testLock = CreateLock(name,4);
			}
			break;
		
		case 2:/*Creating lock with invalid length (500)*/
			testLock = CreateLock("test1",500);
			break;
		
		case 3:/*Creating lock with negative index(-1)*/
			testLock = CreateLock(-1,1);
			break;
		
		case 4:/*Destroying a created lock*/
			testLock=CreateLock("test2",5);
			DestroyLock(testLock);
			break;
		
		case 5:/*Acquiring and releasing a lock*/
			testLock=CreateLock("test3",5);
			AcquireLock(testLock);
			ReleaseLock(testLock);
			break;
		
		case 6:/*Acquiring and releasing an invalid lock*/
			testLock=CreateLock("test4",5);
			testLock2=1500;
			AcquireLock(testLock2);
			ReleaseLock(testLock2);
			break;
			
		case 7:/*Acquiring and releasing a destroyed lock*/
			testLock=CreateLock("test5",5);
			DestroyLock(testLock);
			AcquireLock(testLock);
			ReleaseLock(testLock);
			break;
		
		case 8:/*Creating maximum CVs(>1000)*/
			for(i=0;i<1010;i++){
				j=1000+i;
				for(k=0;k<4;k++){
					m=(j%10)+1;
					name[k]=m;
					j/=10;
				}
				testCV = CreateCV(name,4);
			}
			break;
		
		case 9:/*Creating cv with invalid length (500)*/
			testCV = CreateCV("test1",500);
			break;
		
		case 10:/*Creating cv with negative length(-1)*/
			testCV = CreateCV(-1,1);
			break;
		
		case 11:/*Destroying a created cv*/
			testCV=CreateCV("test2",5);
			DestroyCV(testCV);
			break;
			
		case 12:/* Creating maximum MVs (>1000)*/
			testLock=CreateLock("test7",5);
			AcquireLock(testLock);
			for(i=0;i<1010;i++){
				j=1000+i;
				for(k=0;k<4;k++){
					m=(j%10)+1;
					name[k]=m;
					j/=10;
				}
				testMV = CreateMV(name,4,10);
			}
			ReleaseLock(testLock);
		
		case 13:/*Creating MV with invalid length (500)*/
			testMV = CreateMV("test1",500,10);
			break;
		
		case 14:/*Creating MV with negative index(-1)*/
			testMV = CreateMV(-1,1,10);
			break;
			
		case 15: /* Setting and getting MV*/
			testLock=CreateLock("test8",5);
			AcquireLock(testLock);
			testMV = CreateMV("test2",5,10);
			testMV2 = CreateMV("test3",5,20);
			Print1("\nMV1 created to initial value : 10, mvID : %d \nEnter a number to set MV :",testMV);
			mvValue=scan();
			SetMV(testMV,mvValue);
			mvValue=GetMV(testMV);
			Print1("\nThe value set for MV(%d) : %d",testMV,mvValue);
			Print1("\nMV2 created to initial value : 20, mvID : %d \nEnter a number to set MV :",testMV2);
			mvValue=scan();
			SetMV(testMV2,mvValue);
			mvValue=GetMV(testMV2);
			Print1("\nThe value set for MV(%d) : %d",testMV2,mvValue);
			break;
			
		case 16:
			testMV=CreateMV("test3",5);
			DestroyMV(testMV);
			break;
			
		case 17:
			testMV=CreateMV("test4",5);
			DestroyMV(testMV);
			SetMV(testMV,10);
			break;
			
		default:
			PrintTest("\n Oops. Invalid choice");
			break;
	}
	Exit(0);
}