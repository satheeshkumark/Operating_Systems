#include "system.h"
#include "thread.h"
#include "synch.h"
#include "stdio.h"
#include "iostream.h"
#include "string.h"
#include <cstdlib>

struct CustomerStruct
{
	int customerId;
	int itemCount;
	int itemsOfChoice[10];
	int itemQuantity[10];
	int itemBoughtCount;
	int privilegeStatus;
	char privilegeStatusString[20];
	int moneyInPocket;
	int totalPurchasedCost;
};

Lock *waitingLock[5]; // Common lock for all the three waiting lines
Lock *salesmanLock[5][3];
Lock *itemLock[50];
Lock *salesGoodsLock;
Lock *goodsloaderLock[5];
Lock *trolleyFirstLock,*trolleyEndLock;
Lock *storeroomLock;
Lock *cashierLock[5];
Lock *cashierWaitingLock;
Lock *customerManagerLineLock;
Lock *customerManagerIntrLock;
Lock *totalSalesCashierLock;
Lock *totalLineCountLock;
Lock *cashCustIntrLock[5];
Lock *cashierManagerLineLock;
Lock *cashierBreakLock[5];


Condition *cashierBreakCV[5];
Condition *cashCustLineCV[10];
Condition *cashCustIntrCV[5];
Condition *salesWaitingCV[5][3];
Condition *custWaitingCV[5]; //Frontline where the customers wait to meet salesman
Condition *goodsWaitingCV[5]; //Waiting line where goodsloaders wait to meet the salesman and report about restocking
Condition *complainingCustWaitingCV[5]; // Waiting line for each item if it goes out of stock
Condition *goodsloaderCV[5];
Condition *itemWaitingCV[50];
Condition *itemLockCV[50];
Condition *trolleyWaitingCV;
Condition *cashierManagerLineCV;
Condition *customerManagerLineCV;
Condition *customerManagerIntrCV;
Condition *cashierWaitingCV[5];
Condition *custCashWaitingCV[5];

int customersCount=0;
int custWaitingLineCount[5],goodsWaitingLineCount[5],itemWaitingLinecount[50];
int itemRestockingFlag[50];
int complainingCustWaitingLineCount[5];
int salesStatus[5][3],goodsStatus[5];
int quantityOnShelf[50];
int salesCustNumber[5][3]; // Global variable for 3 salesman to get their customer number they aree dealing with
int complaintCustNumber[5][5];
int salesmanCount=0,goodsloaderCount=0;
int itemToBeRestocked_FromCustToSalesman[100];
int itemToBeRestocked_FromSalesmanToGoodsloader[5];
int itemAvailability[50];
int trolleyFirstCount=100,trolleyEndCount=0;
int trolleyWaitingCount=0;
int salesGoodsId[5];
int restockedItem[5][5];
int restockingGoodsloader[5][3];

int myCashCustomerid[5];
int cashierCount;
int customerMoneyStatus[100];
int customerManagerLineCount=0,cashierManagerLineCount=0;
int managerCustomerId;
int totalSalesCashier[5],totalSales=0;
int custCountTemp,cashierStatus[5];
int cashiersSalesBackup[5],managerSalesBackup;
int cashierBreakUsageFlag[5];
int cashCustLineCount[10];
int departmentsCount=0;

struct CustomerStruct *Customer[100];

 void setInitialItemQuantities(int noOfDepartments)
{
       for(int i=0;i<noOfDepartments*10;i++)
       {
               quantityOnShelf[i] = 5;
       }                
}

void setCustomeritemToBuy(int i,int numberOfDepartments)
{
	int noOfItems = (random()%10)+1;
	int randomUniqueFlag = 0;
	Customer[i]->itemCount = noOfItems;
	int w=0;
	int rndNumber;		
	
	int temp = 10*numberOfDepartments;
	while(w<noOfItems)
	{
		rndNumber = random()%temp;
		for(int j=0;j<noOfItems;j++)
		{
			if(rndNumber==Customer[i]->itemsOfChoice[j])
			{
				randomUniqueFlag=0;
				break;
			}
			else
			{
				randomUniqueFlag=1;
			}
		}
		
		if(randomUniqueFlag==1)
		{
			Customer[i]->itemsOfChoice[w] = rndNumber;
			w++;
		}
	}
	
	//printf("\ndone with item selection\n");
	for(int l=0;l<noOfItems;l++)
	{
		Customer[i]->itemQuantity[l] = random()%10 + 1;
	}
} 

 
void setCustomerPrivilege(int i)
{
	int rndNumber = random()%2;
	
	Customer[i]->privilegeStatus = rndNumber;
	if(rndNumber==1){
		strcpy(Customer[i]->privilegeStatusString,"Privileged ");
	}
	else
		strcpy(Customer[i]->privilegeStatusString,"");
	printf("\ncustomer privilege%d",Customer[i]->privilegeStatus);
}
 
void createCustomerInformation(int numberOfCustomers,int numberOfDepartments)
{
	for(int i=0;i<100;i++)
	{
		Customer[i] = (struct CustomerStruct *)malloc(sizeof(struct CustomerStruct));
	}
	for(int i=0;i<numberOfCustomers;i++)
	{
		Customer[i] = (struct CustomerStruct *)malloc(sizeof(struct CustomerStruct));
		Customer[i]->customerId = i;
		Customer[i]->totalPurchasedCost=0;
		Customer[i]->moneyInPocket=50;
		printf("\n processing customer information%d",Customer[i]->customerId);
		
		setCustomerPrivilege(i);
		printf("\ndone with privileging\n");
		setCustomeritemToBuy(i,numberOfDepartments);
	}		
}

void customer(int myIndex)
{
	int mySalesID1,mySalesID2,i,itemToBuy,salesBusyCount=0,departmentOfChoice;
	int item,salesFlag=-1,j,shortestLineLength = 0,mypreviouscashiersId=-1;
	int shortestLineIndex = 0,mycashiersId=-1,breakFlag=0,tempShortestValue=100;
	printf("\n%sCustomer [%d] enters the SuperMarket",Customer[myIndex]->privilegeStatusString,myIndex);
	printf("\n%sCustomer [%d] wants to buy [%d] no. of items",Customer[myIndex]->privilegeStatusString,myIndex,Customer[myIndex]->itemCount);
	trolleyFirstLock->Acquire();
	if(trolleyFirstCount>0){
		trolleyFirstCount--;
		printf("\n%sCustomer [%d] has a trolley for shopping",Customer[myIndex]->privilegeStatusString,myIndex);
		trolleyFirstLock->Release();
	}
	else{
		trolleyWaitingCount++;
		printf("\n%sCustomer [%d] gets in line for trolley",Customer[myIndex]->privilegeStatusString,myIndex);
		trolleyWaitingCV->Wait(trolleyFirstLock);
		if(trolleyFirstCount>0){
			trolleyFirstCount--;
			printf("\n%sCustomer [%d] has a trolley for shopping",Customer[myIndex]->privilegeStatusString,myIndex);
		}
		trolleyFirstLock->Release();
	}
	for(item=0;item<Customer[myIndex]->itemCount;item++){
		itemToBuy=Customer[myIndex]->itemsOfChoice[item];
		departmentOfChoice=itemToBuy/10;
		waitingLock[departmentOfChoice]->Acquire();
		printf("\n%sCustomer [%d] enters the department%d for item%d",Customer[myIndex]->privilegeStatusString,myIndex,departmentOfChoice,itemToBuy);
		salesFlag=-1;
		for(i=0;i<salesmanCount;i++){
			if(salesStatus[departmentOfChoice][i]==0){
				mySalesID1=i;
				salesmanLock[departmentOfChoice][mySalesID1]->Acquire();
				salesStatus[departmentOfChoice][i]=1;
				salesFlag=1;
				break;
			}
		}
		if(salesFlag==1){
			custWaitingLineCount[departmentOfChoice]++;
			salesCustNumber[departmentOfChoice][mySalesID1]=myIndex;
			salesWaitingCV[departmentOfChoice][mySalesID1]->Signal(salesmanLock[departmentOfChoice][mySalesID1]);
			salesmanLock[departmentOfChoice][mySalesID1]->Release();
			custWaitingCV[departmentOfChoice]->Wait(waitingLock[departmentOfChoice]);
			
		}
		else{
			custWaitingLineCount[departmentOfChoice]++;
			printf("\n%sCustomer [%d] gets in line for the department [%d]",Customer[myIndex]->privilegeStatusString,myIndex,departmentOfChoice);
			custWaitingCV[departmentOfChoice]->Wait(waitingLock[departmentOfChoice]);
		}
		
		for(i=0;i<salesmanCount;i++){
			if(salesStatus[departmentOfChoice][i]==3){
				mySalesID1=i;
				salesmanLock[departmentOfChoice][mySalesID1]->Acquire();
				salesStatus[departmentOfChoice][mySalesID1]=1;
				break;
			}
		}
		
		salesCustNumber[departmentOfChoice][mySalesID1]=myIndex;
		waitingLock[departmentOfChoice]->Release();
		salesWaitingCV[departmentOfChoice][mySalesID1]->Signal(salesmanLock[departmentOfChoice][mySalesID1]);
		salesWaitingCV[departmentOfChoice][mySalesID1]->Wait(salesmanLock[departmentOfChoice][mySalesID1]);
		printf("\n%sCustomer [%d] is interacting with DepartmentSalesman[%d] of Department[%d]",Customer[myIndex]->privilegeStatusString,myIndex,mySalesID1,departmentOfChoice);
		salesWaitingCV[departmentOfChoice][mySalesID1]->Signal(salesmanLock[departmentOfChoice][mySalesID1]);
		//printf("\nCustomer%d : End signalled the Salesman[%d][%d]",myIndex,departmentOfChoice,mySalesID1);
		//printf("\nCustomer%d : Releasing Salesman[%d][%d] lock",myIndex,departmentOfChoice,mySalesID1);
		salesmanLock[departmentOfChoice][mySalesID1]->Release();
		
		itemLock[itemToBuy]->Acquire();
		while(1){
			if(quantityOnShelf[itemToBuy]>=Customer[myIndex]->itemQuantity[item]){
				quantityOnShelf[itemToBuy]-=Customer[myIndex]->itemQuantity[item];
				printf("\n%sCustomer [%d] has found item%d and placed [%d] in the trolly",Customer[myIndex]->privilegeStatusString,myIndex,itemToBuy,Customer[myIndex]->itemQuantity[item]);
				Customer[myIndex]->itemBoughtCount++;
				itemLock[itemToBuy]->Release();
				break;
			}
			else{
				waitingLock[departmentOfChoice]->Acquire();
				itemLock[itemToBuy]->Release();
				salesFlag=-1;
				for(i=0;i<salesmanCount;i++){
					
					if(salesStatus[departmentOfChoice][i]==0){
						mySalesID2=i;
						salesmanLock[departmentOfChoice][mySalesID2]->Acquire();
						salesStatus[departmentOfChoice][mySalesID2]=1;
						salesFlag=1;
						break;
					}
				}
				if(salesFlag==-1){
					printf("\n%sCustomer [%d] is not able to find item%d and is searching for DepartmentSalesman",Customer[myIndex]->privilegeStatusString,myIndex,itemToBuy);
					complainingCustWaitingLineCount[departmentOfChoice]++;
					//printf("\nCustomer%d: Waiting Count: %d",myIndex,complainingCustWaitingLineCount[departmentOfChoice]);
					complainingCustWaitingCV[departmentOfChoice]->Wait(waitingLock[departmentOfChoice]);
					//printf("\nCustomer%d: woke up from complaint line",Customer[myIndex]->customerId);
				}
				else{
					salesWaitingCV[departmentOfChoice][mySalesID2]->Signal(salesmanLock[departmentOfChoice][mySalesID2]);
					complainingCustWaitingLineCount[departmentOfChoice]++;
					salesmanLock[departmentOfChoice][mySalesID2]->Release();
					//printf("Customer: Waiting Count: %d\n",custWaitingLineCount);
					complainingCustWaitingCV[departmentOfChoice]->Wait(waitingLock[departmentOfChoice]);
				}
				for(i=0;i<salesmanCount;i++){
					salesmanLock[departmentOfChoice][i]->Acquire();
					if(salesStatus[departmentOfChoice][i]==4){
						mySalesID2=i;
						salesStatus[departmentOfChoice][mySalesID2]=1;
						break;
					}
					salesmanLock[departmentOfChoice][i]->Release();
				}
				printf("\n%sCustomer [%d] is asking assistance from DepartmentSalesman [%d]",Customer[myIndex]->privilegeStatusString,myIndex,mySalesID2);
				complaintCustNumber[departmentOfChoice][mySalesID2]=myIndex;
				//salesStatus[departmentOfChoice][mySalesID2]=1;
				itemToBeRestocked_FromCustToSalesman[myIndex]=itemToBuy;
				waitingLock[departmentOfChoice]->Release();
				
				salesWaitingCV[departmentOfChoice][mySalesID2]->Signal(salesmanLock[departmentOfChoice][mySalesID2]);
				printf("\n%sCustomer [%d] has received assistance about restocking of item%d from DepartmentSalesman [%d]",Customer[myIndex]->privilegeStatusString,myIndex,itemToBuy,mySalesID2);
				itemLock[itemToBuy]->Acquire();				
				salesmanLock[departmentOfChoice][mySalesID2]->Release();
				itemWaitingLinecount[itemToBuy]++;
				itemWaitingCV[itemToBuy]->Wait(itemLock[itemToBuy]);
				//printf("\n[AFTER ITEM WAIT]customer%d waiting for item%d",myIndex,itemToBuy);
			}
		}		
	}
	
	//-------------------------------------Customer cashier Interaction-----------------------------------------------
	totalLineCountLock->Acquire();
	
	printf("\n%sCustomer [%d] has finished shopping and is looking for the Cashier",Customer[myIndex]->privilegeStatusString,myIndex);
	
	while(true){
		//printf("\nInside Customer %d",myIndex);
		for(j=0;j<cashierCount;j++)
		{
			if(cashierStatus[j] == 0)
			{
				mycashiersId = j;
				cashierStatus[j]=1;
				break;
			}
		}
		//printf("\nInside Customer %d, Cashier [%d] cashier status: %d",myIndex,mycashiersId,cashierStatus[mycashiersId]);
		if(mycashiersId == -1)
		{
			shortestLineLength = tempShortestValue;
			shortestLineIndex = -1;
			if(Customer[myIndex]->privilegeStatus==0)
				j=0;
			else	
				j=1;
				
			for(;j<cashierCount*2;)
			{
				if(shortestLineLength>cashCustLineCount[j] && cashierStatus[j/2]!=2)
				{
					shortestLineLength = cashCustLineCount[j];
					shortestLineIndex = j;
					
				}
				j+=2;
			}
			
			mycashiersId = shortestLineIndex;
			cashCustLineCount[mycashiersId]++;
			
			printf("\n%sCustomer [%d] chose Cashier [%d] with line of length [%d]",Customer[myIndex]->privilegeStatusString,myIndex,mycashiersId/2,shortestLineLength);
			cashCustLineCV[mycashiersId]->Wait(totalLineCountLock);
			mycashiersId/=2;
			if(cashierStatus[mycashiersId]!=2){
				break;
			}else{
				mycashiersId=-1;				
			}
		}
		else{
			printf("\n%sCustomer [%d] chose Cashier [%d] with line of length [%d]",Customer[myIndex]->privilegeStatusString,myIndex,mycashiersId,shortestLineLength);
			break;
		}
	}

	totalLineCountLock->Release();
	
	cashCustIntrLock[mycashiersId]->Acquire();
	cashCustIntrCV[mycashiersId]->Signal(cashCustIntrLock[mycashiersId]);
	myCashCustomerid[mycashiersId] = myIndex;
	cashCustIntrCV[mycashiersId]->Wait(cashCustIntrLock[mycashiersId]);
	
	
	if(customerMoneyStatus[myIndex]==1){
		printf("\n%sCustomer [%d] cannot pay $[%d]",Customer[myIndex]->privilegeStatusString,myIndex,Customer[myIndex]->totalPurchasedCost);
	
		customerManagerLineLock->Acquire();
		cashCustIntrLock[mycashiersId]->Release();
		customerManagerLineCount++;
		printf("\n%sCustomer [%d] is waiting for Manager for negotiations.",Customer[myIndex]->privilegeStatusString,myIndex);
		customerManagerLineCV->Wait(customerManagerLineLock);
		customerManagerIntrLock->Acquire();
		customerManagerLineLock->Release();
		managerCustomerId=myIndex;
		customerManagerIntrCV->Signal(customerManagerIntrLock);
		printf("\n%sCustomer [%d] pays $[%d] to Manager after removing items and is waiting for receipt from Manager. ",Customer[myIndex]->privilegeStatusString,myIndex,Customer[myIndex]->totalPurchasedCost);
		customerManagerIntrCV->Wait(customerManagerIntrLock);
		printf("\n%sCustomer [%d] got receipt from Manager and is now leaving.",Customer[myIndex]->privilegeStatusString,myIndex);
		customerManagerIntrLock->Release();	
	}
	else{
		printf("\n%sCustomer [%d] pays $[%d] to Cashier [%d] and is now waiting for receipt",Customer[myIndex]->privilegeStatusString,myIndex,Customer[myIndex]->totalPurchasedCost,mycashiersId);
		cashCustIntrLock[mycashiersId]->Release();
	}
	
	trolleyEndLock->Acquire();
	trolleyEndCount++;
	printf("\n%sCustomer [%d] is leaving the trolley",Customer[myIndex]->privilegeStatusString,myIndex);
	trolleyEndLock->Release();
	custCountTemp--;
	printf("\nCustomer%d exits the store",Customer[myIndex]->customerId);
	/*salesGoodsLock->Acquire();
	if(goodsWaitingLineCount>0){
		goodsWaitingCV->Signal(salesGoodsLock);
		goodsWaitingLineCount--;
	}
	salesGoodsLock->Release();*/
}

void cashier(int myIndex)
{
	int breakstatusprintflag = 0;
	
	while(true)
	{
		//printf("\nInside cashier %d",myIndex);
		breakstatusprintflag = 0;
		totalLineCountLock->Acquire();
		cashierBreakLock[myIndex]->Acquire();
		if(cashierStatus[myIndex]==2){
			breakstatusprintflag = 1;
			cashierBreakUsageFlag[myIndex] = 1;
			cashCustLineCV[myIndex*2+1]->Broadcast(totalLineCountLock);
			cashCustLineCV[myIndex*2]->Broadcast(totalLineCountLock);
			cashCustLineCount[myIndex*2+1] = 0;
			cashCustLineCount[myIndex*2] = 0;
			totalLineCountLock->Release();
			printf("\nCashier [%d] is going on break.",myIndex);
			cashierBreakCV[myIndex]->Wait(cashierBreakLock[myIndex]);
			printf("\nCashier [%d] was called from break by Manager to work.",myIndex);
			cashierBreakLock[myIndex]->Release();
			//printf()
			totalLineCountLock->Acquire();
		}
		else
		{
			cashierBreakLock[myIndex]->Release();
		}
		
		
		
		if(cashCustLineCount[myIndex*2+1]>0)
		{
			cashCustLineCount[myIndex*2+1]--;
			cashierStatus[myIndex] = 1;
			cashCustLineCV[myIndex*2+1]->Signal(totalLineCountLock);
		}
		else if(cashCustLineCount[myIndex*2]>0)
		{
			cashCustLineCount[myIndex*2]--;
			cashierStatus[myIndex] = 1;
			cashCustLineCV[myIndex*2]->Signal(totalLineCountLock);
		}
		else	
		{
			cashierStatus[myIndex] = 0;
		}
		
		cashCustIntrLock[myIndex]->Acquire();
		totalLineCountLock->Release();
		cashCustIntrCV[myIndex]->Wait(cashCustIntrLock[myIndex]);
		totalLineCountLock->Acquire();
		if(cashierStatus[myIndex]!=2){
			totalLineCountLock->Release();
			for(int j=0; j<Customer[myCashCustomerid[myIndex]]->itemCount; j++)
			{
				printf("\nCashier [%d] got [%d] from trolly of %sCustomer [%d].",myIndex,Customer[myCashCustomerid[myIndex]]->itemsOfChoice[j],Customer[myCashCustomerid[myIndex]]->privilegeStatusString,myCashCustomerid[myIndex]);
				Customer[myCashCustomerid[myIndex]]->totalPurchasedCost += Customer[myCashCustomerid[myIndex]]->itemQuantity[j];
			}
			
			printf("\nCashier [%d] tells %sCustomer [%d] total cost is $[%d].",myIndex,Customer[myCashCustomerid[myIndex]]->privilegeStatusString,myCashCustomerid[myIndex],Customer[myCashCustomerid[myIndex]]->totalPurchasedCost);
			
			if(Customer[myCashCustomerid[myIndex]]->totalPurchasedCost > Customer[myCashCustomerid[myIndex]]->moneyInPocket)
			{
				
				printf("\nCashier [%d] asks %sCustomer [%d] to wait for Manager.",myIndex,Customer[myCashCustomerid[myIndex]]->privilegeStatusString,myCashCustomerid[myIndex]);
				customerMoneyStatus[myCashCustomerid[myIndex]]=1;
				
				cashierManagerLineLock->Acquire();
				cashierManagerLineCount++;
				cashierManagerLineCV->Wait(cashierManagerLineLock);
				printf("\nCashier [%d] informs the Manager that %sCustomer [%d] does not have enough money.",myIndex,Customer[myCashCustomerid[myIndex]]->privilegeStatusString,myCashCustomerid[myIndex]);
				cashierManagerLineLock->Release();
				
			}
			else
			{
				printf("\nCashier [%d] gave the receipt to %sCustomer [%d] and tells him to leave.",myIndex,Customer[myCashCustomerid[myIndex]]->privilegeStatusString,myCashCustomerid[myIndex]);
				customerMoneyStatus[myCashCustomerid[myIndex]]=0;
				printf("\nCashier [%d] got money $[%d] from %sCustomer [%d].",myIndex,Customer[myCashCustomerid[myIndex]]->totalPurchasedCost,Customer[myCashCustomerid[myIndex]]->privilegeStatusString,myCashCustomerid[myIndex] );
				totalSalesCashierLock->Acquire();
				cashiersSalesBackup[myIndex] += Customer[myCashCustomerid[myIndex]]->totalPurchasedCost;
				totalSalesCashier[myIndex] += Customer[myCashCustomerid[myIndex]]->totalPurchasedCost;
				totalSalesCashierLock->Release();
			}
			cashCustIntrCV[myIndex]->Signal(cashCustIntrLock[myIndex]);
			cashCustIntrLock[myIndex]->Release();	
		}
		else{	
			totalLineCountLock->Release();
		}
	}
}
void manager(int myIndex){
	int breakCount=0,activeCount=0,i,j,w,randomBreakCashier[5],randomNmbr,activeCashiers[5];
	int breakSettingFlag=0;
		
	while(custCountTemp>=0){
		cashierManagerLineLock->Acquire();
		if(cashierManagerLineCount>0){
			cashierManagerLineCount=0;
			cashierManagerLineCV->Broadcast(cashierManagerLineLock);
		}
		cashierManagerLineLock->Release();
		customerManagerLineLock->Acquire();
		if(customerManagerLineCount>0){
			customerManagerLineCount--;
			customerManagerLineCV->Signal(customerManagerLineLock);
			customerManagerIntrLock->Acquire();
			customerManagerLineLock->Release();
			customerManagerIntrCV->Wait(customerManagerIntrLock);
			w=Customer[managerCustomerId]->itemCount-1;
			do{
				Customer[managerCustomerId]->totalPurchasedCost-=Customer[managerCustomerId]->itemQuantity[w];
				Customer[managerCustomerId]->itemQuantity[w]=0;
				printf("\nManager removes [%d] from the trolly of %sCustomer [%d].",w,Customer[managerCustomerId]->privilegeStatusString,managerCustomerId);
				w--;
			}while(Customer[managerCustomerId]->totalPurchasedCost>Customer[managerCustomerId]->moneyInPocket);
			
			managerSalesBackup += Customer[managerCustomerId]->totalPurchasedCost;
			
			totalSales+=Customer[managerCustomerId]->totalPurchasedCost;
			customerManagerIntrCV->Signal(customerManagerIntrLock);
			printf("\nManager gives receipt to %sCustomer [%d].",Customer[managerCustomerId]->privilegeStatusString,managerCustomerId);
			customerManagerIntrLock->Release();
		}
		else{
			customerManagerLineLock->Release();
		}
		totalSalesCashierLock->Acquire();
		
		for(i=0;i<cashierCount;i++){
			printf("\nManager emptied Counter [%d] drawer.",i);
			totalSales+=totalSalesCashier[i];
			totalSalesCashier[i]=0;
			printf("\nManager has total sale of $[%d]",totalSales);
			printf("\nCashier %d Status : %d",i,cashierStatus[i]);
		}
		
		totalSalesCashierLock->Release();
		
		/*if(custCountTemp<=(2*cashierCount)){
			totalLineCountLock->Acquire();
			
			for(i=0;i<cashierCount;i++){
				cashierBreakLock[i]->Acquire();
				if(cashierStatus[i]==2){
					cashierStatus[i]=1;
					cashierBreakCV[i]->Signal(cashierBreakLock[i]);
					printf("\nManager brings back Cashier [%d] from break.",i);
				}
				cashierBreakLock[i]->Release();
				breakSettingFlag = 1;			
			}			
			totalLineCountLock->Release();
		}*/
		
		if(custCountTemp==0){
			custCountTemp--;
			for(i=0;i<cashierCount;i++)
			{
				printf("\nTotal Sales from Counter [%d] is $[%d]",i,cashiersSalesBackup[i]);
			}
			printf("\nTotal Sales from Manager counter is $[%d]",managerSalesBackup);
			printf("\nTotal Sale of the entire store is $[%d]",totalSales);
		}
		
		
		for(i=0;i<1000;i++){
			currentThread->Yield();
		}
		
		
		activeCount=0;
		j=0;
		//printf("\nBefore totalLine lock");
		totalLineCountLock->Acquire();
		//printf("\nAfter totalLine lock");
		for(i=0;i<cashierCount;i++){
			if(cashierStatus[i]!=2){
				activeCount++;
				activeCashiers[j]=i;
				j++;
			}
		}
		
		if(activeCount>0)
		{
			randomNmbr=rand()%activeCount;
			
			j=0;
		
			for(i=0;i<cashierCount;i++){
				
				if(j==randomNmbr)
					break;
					
				if(i==activeCashiers[j]){
						cashierBreakLock[i]->Acquire();
						//printf("\nCashier [%d] whose gonna go on break, status : %d",i,cashierStatus[i]);
						if(cashierStatus[i]==0){
							cashCustIntrLock[i]->Acquire();
							cashCustIntrCV[i]->Signal(cashCustIntrLock[i]);
							cashCustIntrLock[i]->Release();
						}
						cashierStatus[i]=2;
						breakCount++;
						printf("\nManager sends Cashier [%d] on break.",i);
						cashierBreakLock[i]->Release();
					}
				
				j++;
			}
			totalLineCountLock->Release();
		}
		else{
			totalLineCountLock->Release();
		}
					
		for(i=0;i<10000;i++){
			currentThread->Yield();
		}
		
		totalLineCountLock->Acquire();
		
		for(i=0;i<cashierCount;i++){
			if(cashierStatus[i]!=2 && (cashCustLineCount[i*2+1]+cashCustLineCount[i*2])>=3){
				for(j=0;j<cashierCount;j++){
					cashierBreakLock[j]->Acquire();
					if(cashierStatus[j]==2 && cashierBreakUsageFlag[j]==1){
						cashierStatus[j]=1;
						cashierBreakUsageFlag[j] = 0;						
						cashierBreakCV[j]->Signal(cashierBreakLock[j]);
						printf("\nManager brings back Cashier [%d] from break.",j);
						breakCount--;
					}
					cashierBreakLock[j]->Release();
				}
				break;
			}
		}
		totalLineCountLock->Release();
		for(i=0;i<100;i++){
			currentThread->Yield();
		}
	}
}
void salesman(int i)
{
	int departmentOfChoice = i/salesmanCount,waitingLineCount=0,goodsFlag=-1,myGoodsID;
	int myIndex = i%salesmanCount;
	while(1)
	{
		waitingLock[departmentOfChoice]->Acquire();
		salesmanLock[departmentOfChoice][myIndex]->Acquire();
		if(custWaitingLineCount[departmentOfChoice] > 0)
		{
			salesStatus[departmentOfChoice][myIndex] = 3;
			custWaitingLineCount[departmentOfChoice]--;
			//printf("\nSalesman[%d][%d] : Before customer%d signal Status : %d ",departmentOfChoice,myIndex,salesCustNumber[departmentOfChoice][myIndex],salesStatus[departmentOfChoice][myIndex]);
			custWaitingCV[departmentOfChoice]->Signal(waitingLock[departmentOfChoice]);
			//printf("\nSalesman[%d][%d] : After customer%d signal Status : %d ",departmentOfChoice,myIndex,salesCustNumber[departmentOfChoice][myIndex],salesStatus[departmentOfChoice][myIndex]);
			waitingLock[departmentOfChoice]->Release();
			salesWaitingCV[departmentOfChoice][myIndex]->Wait(salesmanLock[departmentOfChoice][myIndex]);
			printf("\nDepartmentSalesman [%d] welcomes %sCustomer [%d] to Department [%d]",myIndex,Customer[salesCustNumber[departmentOfChoice][myIndex]]->privilegeStatusString,salesCustNumber[departmentOfChoice][myIndex],departmentOfChoice);
			salesWaitingCV[departmentOfChoice][myIndex]->Signal(salesmanLock[departmentOfChoice][myIndex]);
		}
		else if(goodsWaitingLineCount[departmentOfChoice]>0){
			salesStatus[departmentOfChoice][myIndex]=5;
			goodsWaitingLineCount[departmentOfChoice]--;
			goodsWaitingCV[departmentOfChoice]->Signal(waitingLock[departmentOfChoice]);
			printf("\nDepartmentSalesman [%d] is informed by the Goodsloader [%d] that item%d is restocked",departmentOfChoice,restockingGoodsloader[departmentOfChoice][myIndex],restockedItem[departmentOfChoice][myIndex]);
			waitingLock[departmentOfChoice]->Release();
			salesWaitingCV[departmentOfChoice][myIndex]->Wait(salesmanLock[departmentOfChoice][myIndex]);
			//Item number restocked is placed in a global variable
			salesWaitingCV[departmentOfChoice][myIndex]->Signal(salesmanLock[departmentOfChoice][myIndex]);
			itemLock[restockedItem[departmentOfChoice][myIndex]]->Acquire();
			itemWaitingLinecount[restockedItem[departmentOfChoice][myIndex]]=0;
			itemRestockingFlag[restockedItem[departmentOfChoice][myIndex]]=0;
			itemWaitingCV[restockedItem[departmentOfChoice][myIndex]]->Broadcast(itemLock[restockedItem[departmentOfChoice][myIndex]]);
			printf("\nDepartmentSalesman [%d] informs the waiting customers that item%d is restocked",myIndex,restockedItem[departmentOfChoice][myIndex]);
			itemLock[restockedItem[departmentOfChoice][myIndex]]->Release();
		}
		else if(complainingCustWaitingLineCount[departmentOfChoice]>0){
			salesGoodsLock->Acquire();
			myGoodsID=-1;
			for(i=0;i<goodsloaderCount;i++){
				if(goodsStatus[i]==0){
					myGoodsID=i;
					goodsStatus[i]=2;
					break;
				}
			}
			salesGoodsLock->Release();
			if(myGoodsID!=-1){
				salesStatus[departmentOfChoice][myIndex] = 4;
				complainingCustWaitingLineCount[departmentOfChoice]--;
				//printf("\nSalesman[%d][%d] : Before complaining customer%d signal Status : %d ",departmentOfChoice,myIndex,salesCustNumber[departmentOfChoice][myIndex],salesStatus[departmentOfChoice][myIndex]);
				complainingCustWaitingCV[departmentOfChoice]->Signal(waitingLock[departmentOfChoice]);
				//printf("\nSalesman[%d][%d] : After complaining customer%d signal Status : %d lineCount : %d",departmentOfChoice,myIndex,salesCustNumber[departmentOfChoice][myIndex],salesStatus[departmentOfChoice][myIndex],complainingCustWaitingLineCount[departmentOfChoice]);
				waitingLock[departmentOfChoice]->Release();
				salesWaitingCV[departmentOfChoice][myIndex]->Wait(salesmanLock[departmentOfChoice][myIndex]);
				printf("\nDepartmentSalesman [%d] is informed by %sCustomer [%d] that item%d is out of stock",myIndex,Customer[complaintCustNumber[departmentOfChoice][myIndex]]->privilegeStatusString,complaintCustNumber[departmentOfChoice][myIndex],itemToBeRestocked_FromCustToSalesman[complaintCustNumber[departmentOfChoice][myIndex]]);
				itemLock[itemToBeRestocked_FromCustToSalesman[complaintCustNumber[departmentOfChoice][myIndex]]]->Acquire();
				if(itemRestockingFlag[itemToBeRestocked_FromCustToSalesman[complaintCustNumber[departmentOfChoice][myIndex]]]==0){
					goodsloaderLock[myGoodsID]->Acquire();
					printf("\nDepartmentSalesman [%d] informs the Goodsloader [%d] that item%d is out of stock",myIndex,myGoodsID,itemToBeRestocked_FromCustToSalesman[complaintCustNumber[departmentOfChoice][myIndex]]);
					goodsloaderCV[myGoodsID]->Signal(goodsloaderLock[myGoodsID]);
					goodsloaderCV[myGoodsID]->Wait(goodsloaderLock[myGoodsID]);
					itemToBeRestocked_FromSalesmanToGoodsloader[myGoodsID]=itemToBeRestocked_FromCustToSalesman[complaintCustNumber[departmentOfChoice][myIndex]];
					salesGoodsId[myGoodsID]=myIndex;
					goodsloaderCV[myGoodsID]->Signal(goodsloaderLock[myGoodsID]);
					goodsloaderLock[myGoodsID]->Release();
					itemRestockingFlag[itemToBeRestocked_FromCustToSalesman[complaintCustNumber[departmentOfChoice][myIndex]]]=1;
				}
				itemLock[itemToBeRestocked_FromCustToSalesman[complaintCustNumber[departmentOfChoice][myIndex]]]->Release();
			}
			else{
				
				salesStatus[departmentOfChoice][myIndex] = 1;
				waitingLock[departmentOfChoice]->Release();
				salesmanLock[departmentOfChoice][myIndex]->Release();
				for(i=0;i<50;i++){
					currentThread->Yield();
				}
				continue;
			}
		}
		else
		{
			salesStatus[departmentOfChoice][myIndex] = 0;
			waitingLock[departmentOfChoice]->Release();
		}
		//printf("\nSalesman[%d][%d] : Going to wait Status : %d",departmentOfChoice,myIndex,salesStatus[departmentOfChoice][myIndex]);
		salesWaitingCV[departmentOfChoice][myIndex]->Wait(salesmanLock[departmentOfChoice][myIndex]);
		//printf("\nSalesman[%d][%d] : Got the end signal, status : %d",departmentOfChoice,myIndex,salesStatus[departmentOfChoice][myIndex]);
		salesmanLock[departmentOfChoice][myIndex]->Release();
	}
}

void goodsloader(int myIndex){
	int departmentOfChoice,goodsFlag=-1,mySalesID,i;
	while(1){
		salesGoodsLock->Acquire();
		goodsloaderLock[myIndex]->Acquire();
		if(goodsStatus[myIndex]==2){
			salesGoodsLock->Release();
			goodsloaderCV[myIndex]->Signal(goodsloaderLock[myIndex]);
			goodsloaderCV[myIndex]->Wait(goodsloaderLock[myIndex]);
			storeroomLock->Acquire();
			itemLock[itemToBeRestocked_FromSalesmanToGoodsloader[myIndex]]->Acquire();
			departmentOfChoice=itemToBeRestocked_FromSalesmanToGoodsloader[myIndex]/10;
			printf("\nGoodsloader [%d] is informed by DepartmentSalesman [%d] of department [%d] to restock item%d",myIndex,salesGoodsId[myIndex],departmentOfChoice,itemToBeRestocked_FromSalesmanToGoodsloader[myIndex]);
			printf("\nGoodsloader [%d] is in the StockRoom and got item%d",myIndex,itemToBeRestocked_FromSalesmanToGoodsloader[myIndex]);
			quantityOnShelf[itemToBeRestocked_FromSalesmanToGoodsloader[myIndex]]=10;
			printf("\nGoodsloader [%d] restocked item%d in Department [%d]",myIndex,itemToBeRestocked_FromSalesmanToGoodsloader[myIndex],departmentOfChoice);
			printf("\nGoodsloader [%d] leaves StockRoom",myIndex);
			storeroomLock->Release();
			itemLock[itemToBeRestocked_FromSalesmanToGoodsloader[myIndex]]->Release();
			waitingLock[departmentOfChoice]->Acquire();
			goodsWaitingLineCount[departmentOfChoice]++;
			salesmanLock[departmentOfChoice][salesGoodsId[myIndex]]->Acquire();
			salesWaitingCV[departmentOfChoice][salesGoodsId[myIndex]]->Signal(salesmanLock[departmentOfChoice][salesGoodsId[myIndex]]);
			salesmanLock[departmentOfChoice][salesGoodsId[myIndex]]->Release();
			goodsWaitingCV[departmentOfChoice]->Wait(waitingLock[departmentOfChoice]);
			
			mySalesID=-1;
			for(i=0;i<salesmanCount;i++){
				if(salesStatus[departmentOfChoice][i]==5){
					mySalesID=i;
					salesStatus[departmentOfChoice][mySalesID]=1;
					break;
				}
			}
			salesmanLock[departmentOfChoice][mySalesID]->Acquire();
			waitingLock[departmentOfChoice]->Release();
			restockedItem[departmentOfChoice][mySalesID]=itemToBeRestocked_FromSalesmanToGoodsloader[myIndex];
			restockingGoodsloader[departmentOfChoice][mySalesID]=myIndex;
			salesWaitingCV[departmentOfChoice][mySalesID]->Signal(salesmanLock[departmentOfChoice][mySalesID]);
			salesWaitingCV[departmentOfChoice][mySalesID]->Wait(salesmanLock[departmentOfChoice][mySalesID]);
			salesWaitingCV[departmentOfChoice][mySalesID]->Signal(salesmanLock[departmentOfChoice][mySalesID]);
			salesmanLock[departmentOfChoice][mySalesID]->Release();
		}
		else{
			salesGoodsLock->Release();
		}
		trolleyEndLock->Acquire();		
		if(trolleyEndCount>0){
			trolleyEndCount--;
			trolleyEndLock->Release();
			trolleyFirstLock->Acquire();
			trolleyFirstCount++;
			printf("\nGoodsloader [%d] replaced the trolley",myIndex);
			//trolleyFirstLock->Release();
			if(trolleyWaitingCount>0){
				trolleyWaitingCount--;
				trolleyWaitingCV->Signal(trolleyFirstLock);
			}
			trolleyFirstLock->Release();
		}
		else{
			trolleyEndLock->Release();
		}
		salesGoodsLock->Acquire();	
		goodsStatus[myIndex]=0;
		salesGoodsLock->Release();
		
		printf("\nGoodsloader [%d] is waiting for orders to restock",myIndex);
		goodsloaderCV[myIndex]->Wait(goodsloaderLock[myIndex]);
		goodsloaderLock[myIndex]->Release();
	}
}
void simulation()
{	
	int i,j;

	printf("\nEnter the count of Departments(3 to 5)");
	scanf("%d",&departmentsCount);
	
	printf("No of departments: %d",departmentsCount);
	printf("\nEnter the count of Salesmen(1 to 3)");
	scanf("%d",&salesmanCount);
	printf("\nEnter the count of Customers(>30)");
	scanf("%d",&customersCount);
	custCountTemp=customersCount;
	
	printf("\nNo of customers: %d",customersCount);
	printf("\nEnter the count of Goodsloaders(3 to 5)");
	scanf("%d",&goodsloaderCount);
	printf("\nEnter the number of cashiers");
	scanf("%d",&cashierCount);
	
	for(i=0;i<goodsloaderCount;i++){
		itemToBeRestocked_FromSalesmanToGoodsloader[i]=0;
		goodsStatus[i]=1;
		goodsloaderLock[i]=new Lock("goodsloader_lock");
		goodsloaderCV[i]=new Condition("goodsloader_CV");
	}
	
	trolleyFirstLock=new Lock("Trolley_first_Lock");
	trolleyEndLock=new Lock("Trolley_end_lock");
	trolleyWaitingCV=new Condition("Trolley_waiting_CV");
	
	createCustomerInformation(customersCount,departmentsCount);
	setInitialItemQuantities(departmentsCount);
	
	
	cashierManagerLineCV=new Condition("Cashier_Manager_Lock");
	cashierManagerLineLock=new Lock("Cashier_Manager_CV");
	customerManagerLineCV=new Condition("Customer_Manager_CV");
	customerManagerLineLock=new Lock("Customer_Manager_Lock");
	customerManagerIntrLock=new Lock("Customer_Manager_Interaction_Lock");
	customerManagerIntrCV=new Condition("Customer_Manager_Interaction_CV");
	totalSalesCashierLock=new Lock("Totalsales_cashier");
	
	for(i=0;i<cashierCount;i++)
	{
	 cashierStatus[i]=1;
	 totalSalesCashier[i]=0;
	 cashierLock[i] = new Lock("Cashier_lock");
	 cashierWaitingCV[i] = new Condition("Cashier_CV");
	 custCashWaitingCV[i] = new Condition("Cust_cash_waiting_CV");
	 cashCustIntrCV[i] = new Condition("Customer_Cashier_Interaction_CV");
	 cashCustIntrLock[i] = new Lock("Cashier_Customer_Interaction_Lock");
	 cashierBreakCV[i] = new Condition("Cashier_Break_CV");
	 cashierBreakLock[i] = new Lock("Cashier_Break_Lock");
	}
	
	for(i=0;i<cashierCount*2;i++)
	{
	cashCustLineCV[i]  = new Condition("Customer_cashier_Line_CV");
	}
	
	totalLineCountLock = new Lock("Total_LineCount_Lock");
	cashierWaitingLock = new Lock("Cashier_waiting_lock");
	
	
	for(i=0;i<customersCount;i++)
	{
		printf("\nCustomer%d\n",Customer[i]->customerId);
		Customer[i]->itemBoughtCount=0;
		for(j=0;j<Customer[i]->itemCount;j++)
		{
			printf("\n\tItem%d",Customer[i]->itemsOfChoice[j]);
			printf("->Quantity%d",Customer[i]->itemQuantity[j]);
		}
		
		if(Customer[i]->privilegeStatus == 0)
			printf("\nUnprevileged Customer");
		else
			printf("\nPrivileged Customer");
	}
	
	salesGoodsLock=new Lock("Salesman_Goodsloader_Lock");
	storeroomLock=new Lock("Storeroom_lock");
	for(i=0;i<departmentsCount;i++)
	{
		for(j=0;j<salesmanCount;j++)
		{
			salesWaitingCV[i][j] =  new Condition("Salesman_Waiting_CV");
			salesmanLock[i][j] = new Lock("Salesman_lock");
			salesStatus[i][j] = 1;
		}
	}
	for(j=0;j<departmentsCount;j++)
	{
		goodsWaitingCV[j]=new Condition("Goodsloader_Waiting_CV");
		waitingLock[j] = new Lock("Customer_Sales_Lock");	
		custWaitingCV[j] =  new Condition("Customer_Waiting_CV");
		custWaitingLineCount[j] = 0;
		complainingCustWaitingLineCount[j]=0;
		complainingCustWaitingCV[j] = new Condition("Customer_Complaining_Waiting_CV");
	}
	for(i=0;i<(departmentsCount*10);i++){
		itemLockCV[i]=new Condition("itemlock_CV");
		itemAvailability[i]=1;
		itemLock[i]=new Lock("item_Lock");
		itemWaitingCV[i]=new Condition("item_Waiting_CV");
		itemWaitingLinecount[i]=0;
	}
	//printf("\nend of customers thread\n");
	for(i=0;i<(departmentsCount*salesmanCount);i++)
	{
		Thread *salesmanThread = new Thread("New_Salesman");
		salesmanThread->Fork((VoidFunctionPtr)salesman,i);	
	}
	for(i=0;i<customersCount;i++)
	{
		Thread *customerThread = new Thread("New_Customer");
		customerThread->Fork((VoidFunctionPtr)customer,i);
	}
	for(i=0;i<goodsloaderCount;i++)
	{
		Thread *goodsloaderThread = new Thread("New_Customer");
		goodsloaderThread->Fork((VoidFunctionPtr)goodsloader,i);
	}	
	for(i=0;i<cashierCount;i++)
	{
		Thread *cashierThread = new Thread("New_Cashier");
		cashierThread->Fork((VoidFunctionPtr)cashier,i);	
	}
	
	Thread *managerThread = new Thread("New_Cashier");
	managerThread->Fork((VoidFunctionPtr)manager,1);	
	
	
}

void TestSuite()
{

}

void test_case_1()
{
	int i=0,j=0;
	
	printf("\nEnter the count of Customers");
	scanf("%d",&customersCount);
	
	printf("\nEnter the count of the cashiers");
	scanf("%d",&cashierCount);
	
	printf("\nEnter the count of Departments(3 to 5)");
	scanf("%d",&departmentsCount);
		
	createCustomerInformation(customersCount,departmentsCount);
	setInitialItemQuantities(departmentsCount);
	
	for(i=0;i<goodsloaderCount;i++){
		itemToBeRestocked_FromSalesmanToGoodsloader[i]=0;
		goodsStatus[i]=1;
		goodsloaderLock[i]=new Lock("goodsloader_lock");
		goodsloaderCV[i]=new Condition("goodsloader_CV");
	}
	
	trolleyFirstLock=new Lock("Trolley_first_Lock");
	trolleyEndLock=new Lock("Trolley_end_lock");
	trolleyWaitingCV=new Condition("Trolley_waiting_CV");
	
	createCustomerInformation(customersCount,departmentsCount);
	setInitialItemQuantities(departmentsCount);
	
	cashierManagerLineCV=new Condition("Cashier_Manager_Lock");
	cashierManagerLineLock=new Lock("Cashier_Manager_CV");
	customerManagerLineCV=new Condition("Customer_Manager_CV");
	customerManagerLineLock=new Lock("Customer_Manager_Lock");
	customerManagerIntrLock=new Lock("Customer_Manager_Interaction_Lock");
	customerManagerIntrCV=new Condition("Customer_Manager_Interaction_CV");
	totalSalesCashierLock=new Lock("Totalsales_cashier");
	
	for(i=0;i<cashierCount;i++)
	{
	 cashierStatus[i]=1;
	 totalSalesCashier[i]=0;
	 cashierLock[i] = new Lock("Cashier_lock");
	 cashierWaitingCV[i] = new Condition("Cashier_CV");
	 custCashWaitingCV[i] = new Condition("Cust_cash_waiting_CV");
	 cashCustIntrCV[i] = new Condition("Customer_Cashier_Interaction_CV");
	 cashCustIntrLock[i] = new Lock("Cashier_Customer_Interaction_Lock");
	}
	for(i=0;i<cashierCount*2;i++)
	{
	cashCustLineCV[i]  = new Condition("Customer_cashier_Line_CV");
	}
	
	totalLineCountLock = new Lock("Total_LineCount_Lock");
	cashierWaitingLock = new Lock("Cashier_waiting_lock");
	
	salesGoodsLock=new Lock("Salesman_Goodsloader_Lock");
	storeroomLock=new Lock("Storeroom_lock");
	
	for(i=0;i<departmentsCount;i++)
	{
		for(j=0;j<salesmanCount;j++)
		{
			salesWaitingCV[i][j] =  new Condition("Salesman_Waiting_CV");
			salesmanLock[i][j] = new Lock("Salesman_lock");
			salesStatus[i][j] = 1;
		}
	}
	for(j=0;j<departmentsCount;j++)
	{
		goodsWaitingCV[j]=new Condition("Goodsloader_Waiting_CV");
		waitingLock[j] = new Lock("Customer_Sales_Lock");	
		custWaitingCV[j] =  new Condition("Customer_Waiting_CV");
		custWaitingLineCount[j] = 0;
		complainingCustWaitingLineCount[j]=0;
		complainingCustWaitingCV[j] = new Condition("Customer_Complaining_Waiting_CV");
	}
	for(i=0;i<(departmentsCount*10);i++){
		itemLockCV[i]=new Condition("itemlock_CV");
		itemAvailability[i]=1;
		itemLock[i]=new Lock("item_Lock");
		itemWaitingCV[i]=new Condition("item_Waiting_CV");
		itemWaitingLinecount[i]=0;
	}
	//printf("\nend of customers thread\n");
	for(i=0;i<(departmentsCount*salesmanCount);i++)
	{
		Thread *salesmanThread = new Thread("New_Salesman");
		salesmanThread->Fork((VoidFunctionPtr)salesman,i);	
	}
	for(i=0;i<customersCount;i++)
	{
		Thread *customerThread = new Thread("New_Customer");
		customerThread->Fork((VoidFunctionPtr)customer,i);
	}
	for(i=0;i<goodsloaderCount;i++)
	{
		Thread *goodsloaderThread = new Thread("New_Customer");
		goodsloaderThread->Fork((VoidFunctionPtr)goodsloader,i);
	}	
	for(i=0;i<cashierCount;i++)
	{
		Thread *cashierThread = new Thread("New_Cashier");
		cashierThread->Fork((VoidFunctionPtr)cashier,i);	
	}
	
	Thread *managerThread = new Thread("New_Cashier");
	managerThread->Fork((VoidFunctionPtr)manager,1);		
}

void test_case_2()
{
	int i=0,j=0;
	printf("\nEnter the count of Departments(3 to 5)");
	scanf("%d",&departmentsCount);
	
	printf("\nEnter the count of Salesmen(1 to 3)");
	scanf("%d",&salesmanCount);
	
	printf("\nEnter the count of Customers(>30)");
	scanf("%d",&customersCount);
	
	createCustomerInformation(customersCount,departmentsCount);
	setInitialItemQuantities(departmentsCount);
	
	for(i=0;i<goodsloaderCount;i++){
		itemToBeRestocked_FromSalesmanToGoodsloader[i]=0;
		goodsStatus[i]=1;
		goodsloaderLock[i]=new Lock("goodsloader_lock");
		goodsloaderCV[i]=new Condition("goodsloader_CV");
	}
	
	trolleyFirstLock=new Lock("Trolley_first_Lock");
	trolleyEndLock=new Lock("Trolley_end_lock");
	trolleyWaitingCV=new Condition("Trolley_waiting_CV");
	
	createCustomerInformation(customersCount,departmentsCount);
	setInitialItemQuantities(departmentsCount);
	
	salesGoodsLock=new Lock("Salesman_Goodsloader_Lock");
	storeroomLock=new Lock("Storeroom_lock");
	
	for(i=0;i<departmentsCount;i++)
	{
		for(j=0;j<salesmanCount;j++)
		{
			salesWaitingCV[i][j] =  new Condition("Salesman_Waiting_CV");
			salesmanLock[i][j] = new Lock("Salesman_lock");
			salesStatus[i][j] = 1;
		}
	}
	
	for(j=0;j<departmentsCount;j++)
	{
		goodsWaitingCV[j]=new Condition("Goodsloader_Waiting_CV");
		waitingLock[j] = new Lock("Customer_Sales_Lock");	
		custWaitingCV[j] =  new Condition("Customer_Waiting_CV");
		custWaitingLineCount[j] = 0;
		complainingCustWaitingLineCount[j]=0;
		complainingCustWaitingCV[j] = new Condition("Customer_Complaining_Waiting_CV");
	}
	
	for(i=0;i<(departmentsCount*10);i++){
		itemLockCV[i]=new Condition("itemlock_CV");
		itemAvailability[i]=1;
		itemLock[i]=new Lock("item_Lock");
		itemWaitingCV[i]=new Condition("item_Waiting_CV");
		itemWaitingLinecount[i]=0;
	}
	//printf("\nend of customers thread\n");
	for(i=0;i<(departmentsCount*salesmanCount);i++)
	{
		Thread *salesmanThread = new Thread("New_Salesman");
		salesmanThread->Fork((VoidFunctionPtr)salesman,i);	
	}
	for(i=0;i<customersCount;i++)
	{
		Thread *customerThread = new Thread("New_Customer");
		customerThread->Fork((VoidFunctionPtr)customer,i);
	}
	for(i=0;i<goodsloaderCount;i++)
	{
		Thread *goodsloaderThread = new Thread("New_Customer");
		goodsloaderThread->Fork((VoidFunctionPtr)goodsloader,i);
	}	
}

void Problem2()
{	
	int choice;	
	
	printf("\nChoose an option:\n");
	
	printf("\n1.You have selected the simulation");
	printf("\n2.You have selected the test cases for part1");
	printf("\n3.Customers always take the shortest line, but no 2 customers ever choose the same shortest line at the same time");
	printf("\n4.Test case1 : Customers wait for items to be restocked - when told by the Department Salesman ");
	printf("\n5.Test case2 : Goods Loader don't try to restock an item on a shelf when a Customer is trying to take an item off the shelf");
	printf("\n6.Test Case4: Cashiers are sent on break by the Manager randomly.");
	printf("\n7.Test Case5: Cashier scans the items till the trolly is empty");
	printf("\n8.Test Case6: Customers do not leave until they are given their receipt by the Cashier. The Cashier does not start on another customer until they know that the last Customer has left their area");
	printf("\n9.Test Case7: Managers get Cashiers off their break when lines have 3, or more, Customers ");
	printf("\n10.Test Case8: Total sales never suffers from a race condition ");
	printf("\n11.Test Case9: Managers can only talk to one Cashier, or one Customer, at a time ");
	printf("\n12.Only one Goods Loader enters the stock room at a time.\n");
	
	//printf("Select a Test Case\n");
	scanf("%d",&choice);	

	switch(choice)
	{	
		case 1: 
		printf("\nYou have selected the simulation");
		simulation();
		break;
		
		
		case 2:
		printf("\nYou have selected the test cases for part1");
		TestSuite();
		break;
		
		case 3:
		printf("\nCustomers always take the shortest line, but no 2 customers ever choose the same shortest line at the same time");
		test_case_1();
		
		
		case 4:
		printf("\nTest case1 : Customers wait for items to be restocked - when told by the Department Salesman ");
		test_case_2();
		break;
		
		case 5:
		printf("\nTest case2 : Goods Loader don't try to restock an item on a shelf when a Customer is trying to take an item off the shelf");
		test_case_2();
		break;
		
		case 6:
		printf("\nTest Case4: Cashiers are sent on break by the Manager randomly.");
		simulation();
		break;
		
		case 7:
		printf("\nTest Case5: Cashier scans the items till the trolly is empty");
		simulation();
		break;
		
		case 8:
		printf("\nTest Case6: Customers do not leave until they are given their receipt by the Cashier. The Cashier does not start on another customer until they know that the last Customer has left their area");
		simulation();
		break;
		
		case 9:
		printf("\nTest Case7: Managers get Cashiers off their break when lines have 3, or more, Customers ");
		simulation();
		break;
		
		case 10:
		printf("\nTest Case8: Total sales never suffers from a race condition ");
		simulation();
		break;
		
		case 11:
		printf("\nTest Case9: Managers can only talk to one Cashier, or one Customer, at a time ");
		simulation();
		break;	
		
		case 12:
		printf("\nOnly one Goods Loader enters the stock room at a time. ");
		simulation();
		break;
	}
}
