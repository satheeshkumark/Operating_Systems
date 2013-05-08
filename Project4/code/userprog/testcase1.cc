#include "syscall.h"

struct CustomerStruct
{
	int customerId;
	int itemCount;
	int itemsOfChoice[10];
	int itemQuantity[10];
	int itemBoughtCount;
	int privilegeStatus;
	int moneyInPocket;
	int totalPurchasedCost;
};

int waitingLock[5]; 					/* Common lock per each department for all the greeting queue,complaining queue and restocking queue */
int salesmanLock[5][3]; 				/* Salesman interaction lock */
int itemLock[50]; 						/* Lock for each item */
int salesGoodsLock; 					/* Lock for goodsloader status */
int goodsloaderLock[5]; 				/* Goodsloader interaction loack */
int trolleyFirstLock,trolleyEndLock; 	/* Lock for trolleys at the front and back door */
int storeroomLock; 						/*Restocking room lock */
int customerManagerLineLock;			/* Lock for customers' line to meet the manager */
int cashierManagerLineLock;				/* Lock for cashiers' line to meet the manager */
int customerManagerIntrLock;			/* Interaction lock for the manager and customers */
int totalSalesCashierLock;				/* Lock to empty the counters of cashiers */
int totalLineCountLock;					/* Lock for all the cashiers status */
int cashCustIntrLock[5];				/* Interaction lock for cashiers and customers */
int cashierBreakLock[5];				/* Lock to send cashier on break */
int cashierIndexLock;  
int customerIndexLock; 
int goodsIndexLock; 

int cashierBreakCV[5];					/* CV for cashiers to go on break */
int cashCustLineCV[10];					/* CV for cashiers' waiting line */
int cashCustIntrCV[5];					/* Interaction CV for cashiers and customers */
int cashierManagerLineCV;				/* CV for customers' line to meet the manager */
int customerManagerLineCV;              /* CV for cashiers' line to meet the manager */
int customerManagerIntrCV;				/* Interaction CV for the manager and customers */
int custCashWaitingCV[5];				/* CV for customers' line to meet the cashier */
int salesWaitingCV[5][3];				/* Interaction CV for salesmen and customers */
int custWaitingCV[5];					/* CV for customers' line to meet the salesmen*/
int goodsWaitingCV[5]; 					/* CV for goodsloaders' line to meet the salesmen and report about restocking*/
int complainingCustWaitingCV[5]; 		/* CV for complaining customers' line to meet the salesmen and complain about items if they go out of stock*/
int goodsloaderCV[5];					/* Interaction CV for goodsloaders and salesmen */
int itemWaitingCV[50];					/* CV for customers' line to wait until item is restocked */
int trolleyWaitingCV;					/* CV for customers' line to wait for trolleys */

int customersCount;						/* Total no of customers */
int salesmanCount;						/* Total no of salesmen */
int goodsloaderCount;					/* Total no of goodsloader */
int cashierCount;						/* Total no of cashiers */
int departmentsCount;					/* Total no of departments */

int custWaitingLineCount[5];			/* No of customers in greeting queue */
int goodsWaitingLineCount[5];			/* No of goodsloaders in restocking queue */
int complainingCustWaitingLineCount[5];	/* No of customers in complaining queue */
int itemWaitingLinecount[50];			/* No of customers waiting for each item to be restocked */
int itemRestockingFlag[50];				/* Flag to know whether a goodsloader has already been sent to restock an item */
int salesStatus[5][3];					/* Status of each salesman */
int goodsStatus[5];						/* Status of each goodsloader */
int cashierStatus[5];					/* Status of each cashier */
int quantityOnShelf[50];				/* Each item's quantity on shelf */
int salesCustNumber[5][3];				/* Variable for salesman to get their customer number they are dealing with */
int complaintCustNumber[5][5];			/* Variable for salesman to get their complaining customer number they are dealing with */
int itemToBeRestocked_FromCustToSalesman[100]; /* Variable for salesman to get the item to be restocked from customer */
int itemToBeRestocked_FromSalesmanToGoodsloader[5]; /* Variable for goodsloader to get the item to be restocked from salesman */
int trolleyFirstCount=50;				/* No of trolleys at the entrance */
int trolleyEndCount=0;					/* No of trolleys at the exit */
int trolleyWaitingCount=0;				/* No of customers waiting for trolleys */
int salesGoodsId[5];					/* Variable for goodsloader to get the salesman number they are dealing with */
int restockedItem[5][5];				/* Item that has been restocked at present */
int restockingGoodsloader[5][3];		/* Variable for salesman to get the restocked goodsloader he is dealing with */

int myCashCustomerid[5];				/* Variable for cashier to get the customer number he is dealing with */ 
int customerMoneyStatus[100];			/* Status of customer whether he has enough money or not */
int customerManagerLineCount=0;			/* No of customers in manager's customer queue */
int cashierManagerLineCount=0;			/* No of cashiers in manager's cashier queue */
int managerCustomerId;					/* Variable for manager to get the customer number he is dealing with */ 
int totalSalesCashier[5];				/* Total sales done by each cashier */
int totalSales=0;						/* Total sales at the store */
int cashiersSalesBackup[5];				/* Cashier sales back up */
int managerSalesBackup;					/* Manager sales backup */	
int cashierBreakUsageFlag[5];
int cashCustLineCount[10];
int custCountTemp;						/* Temp variable to keep track of customers who exit the store */

/* Indexes for all entities */
int globalCashierIndex = 0;
int globalCustomerIndex = 0;
int globalSalesmanIndex = 0;
int salesmanIndexLock; 
int globalGoodsIndex = 0;

/* Creating customer information */
struct CustomerStruct Customer[100];

  void setInitialItemQuantities(int noOfDepartments)
{
	int i;
       for(i=0;i<noOfDepartments*10;i++)
       {
               quantityOnShelf[i] = 5;
       }                
}

void setCustomeritemToBuy(int i,int numberOfDepartments)
{
	int noOfItems = (RandomSearch()%10)+1;
	int randomUniqueFlag = 0;
	int w=0;
	int rndNumber,j,l;
	int temp = 10*numberOfDepartments;
	
	Customer[i].itemCount = noOfItems;		
	while(w<noOfItems)
	{
		rndNumber = RandomSearch()%temp;
		for(j=0;j<noOfItems;j++)
		{
			if(rndNumber==Customer[i].itemsOfChoice[j])
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
			Customer[i].itemsOfChoice[w] = rndNumber;
			w++;
		}
	}
	
	for(l=0;l<noOfItems;l++)
	{
		Customer[i].itemQuantity[l] = RandomSearch()%10 + 1;
	}
} 

 
void setCustomerPrivilege(int i)
{
	int rndNumber = RandomSearch()%2;
	
	Customer[i].privilegeStatus = rndNumber;
	Print1("customer privilege%d",Customer[i].privilegeStatus);
}
 
void createCustomerInformation(int numberOfCustomers,int numberOfDepartments)
{
	int i;
	for(i=0;i<numberOfCustomers;i++)
	{
		Customer[i].customerId = i;
		Customer[i].totalPurchasedCost=0;
		Customer[i].moneyInPocket=50;
		Print1(" processing customer information%d",Customer[i].customerId);
		
		setCustomerPrivilege(i);
		Print("done with privileging");
		setCustomeritemToBuy(i,numberOfDepartments);
	}		
}
void salesman()
{
	int i,departmentOfChoice,waitingLineCount=0,goodsFlag=-1,myGoodsID;
	int salesmanIndex;
	
	AcquireLock(salesmanIndexLock);
	i=globalSalesmanIndex++;
	ReleaseLock(salesmanIndexLock);
	
	salesmanIndex = i%salesmanCount;
	departmentOfChoice=i/salesmanCount;
	while(1)
	{
		AcquireLock(waitingLock[departmentOfChoice]);
		AcquireLock(salesmanLock[departmentOfChoice][salesmanIndex]);
		if(custWaitingLineCount[departmentOfChoice] > 0) /* If Customers are waiting in greeting queue */
		{
			salesStatus[departmentOfChoice][salesmanIndex] = 3;
			custWaitingLineCount[departmentOfChoice]--;
			SignalCV(waitingLock[departmentOfChoice],custWaitingCV[departmentOfChoice]);
			ReleaseLock(waitingLock[departmentOfChoice]);
			WaitCV(salesmanLock[departmentOfChoice][salesmanIndex],salesWaitingCV[departmentOfChoice][salesmanIndex]);
			if(Customer[salesCustNumber[departmentOfChoice][salesmanIndex]].privilegeStatus==1)
				Print3("DepartmentSalesman [%d] welcomes Privileged Customer [%d] to Department [%d]",salesmanIndex,salesCustNumber[departmentOfChoice][salesmanIndex],departmentOfChoice);
			else
				Print3("DepartmentSalesman [%d] welcomes Customer [%d] to Department [%d]",salesmanIndex,salesCustNumber[departmentOfChoice][salesmanIndex],departmentOfChoice);
			SignalCV(salesmanLock[departmentOfChoice][salesmanIndex],salesWaitingCV[departmentOfChoice][salesmanIndex]);
		}
		else if(goodsWaitingLineCount[departmentOfChoice]>0){ /* If Goodsloaders are waiting in restocking queue */
			salesStatus[departmentOfChoice][salesmanIndex]=5;
			goodsWaitingLineCount[departmentOfChoice]--;
			SignalCV(waitingLock[departmentOfChoice],goodsWaitingCV[departmentOfChoice]);
			Print3("DepartmentSalesman [%d] is informed by the Goodsloader [%d] that item%d is restocked",departmentOfChoice,restockingGoodsloader[departmentOfChoice][salesmanIndex],restockedItem[departmentOfChoice][salesmanIndex]);
			ReleaseLock(waitingLock[departmentOfChoice]);
			WaitCV(salesmanLock[departmentOfChoice][salesmanIndex],salesWaitingCV[departmentOfChoice][salesmanIndex]);
			/*Item number restocked is placed in a global variable*/
			SignalCV(salesmanLock[departmentOfChoice][salesmanIndex],salesWaitingCV[departmentOfChoice][salesmanIndex]);
			AcquireLock(itemLock[restockedItem[departmentOfChoice][salesmanIndex]]);
			itemWaitingLinecount[restockedItem[departmentOfChoice][salesmanIndex]]=0;
			itemRestockingFlag[restockedItem[departmentOfChoice][salesmanIndex]]=0;
			BroadcastCV(itemLock[restockedItem[departmentOfChoice][salesmanIndex]],itemWaitingCV[restockedItem[departmentOfChoice][salesmanIndex]]);
			Print2("DepartmentSalesman [%d] informs the waiting customers that item%d is restocked",salesmanIndex,restockedItem[departmentOfChoice][salesmanIndex]);
			ReleaseLock(itemLock[restockedItem[departmentOfChoice][salesmanIndex]]);
		}
		else if(complainingCustWaitingLineCount[departmentOfChoice]>0){	/* If Customers are waiting in complaining queue */
			AcquireLock(salesGoodsLock);
			myGoodsID=-1;
			for(i=0;i<goodsloaderCount;i++){
				if(goodsStatus[i]==0){
					myGoodsID=i;
					goodsStatus[i]=2;
					break;
				}
			}
			ReleaseLock(salesGoodsLock);
			if(myGoodsID!=-1){ /* Signal the customers line only if goodsloader is available to restock */
				salesStatus[departmentOfChoice][salesmanIndex] = 4;
				complainingCustWaitingLineCount[departmentOfChoice]--;
				SignalCV(waitingLock[departmentOfChoice],complainingCustWaitingCV[departmentOfChoice]);
				ReleaseLock(waitingLock[departmentOfChoice]);
				WaitCV(salesmanLock[departmentOfChoice][salesmanIndex],salesWaitingCV[departmentOfChoice][salesmanIndex]);
				if(Customer[complaintCustNumber[departmentOfChoice][salesmanIndex]].privilegeStatus==1)
					Print3("DepartmentSalesman [%d] is informed by Privileged Customer [%d] that item%d is out of stock",salesmanIndex,complaintCustNumber[departmentOfChoice][salesmanIndex],itemToBeRestocked_FromCustToSalesman[complaintCustNumber[departmentOfChoice][salesmanIndex]]);
				else
					Print3("DepartmentSalesman [%d] is informed by Customer [%d] that item%d is out of stock",salesmanIndex,complaintCustNumber[departmentOfChoice][salesmanIndex],itemToBeRestocked_FromCustToSalesman[complaintCustNumber[departmentOfChoice][salesmanIndex]]);
				AcquireLock(itemLock[itemToBeRestocked_FromCustToSalesman[complaintCustNumber[departmentOfChoice][salesmanIndex]]]);
				if(itemRestockingFlag[itemToBeRestocked_FromCustToSalesman[complaintCustNumber[departmentOfChoice][salesmanIndex]]]==0){
					AcquireLock(goodsloaderLock[myGoodsID]);
					Print3("DepartmentSalesman [%d] informs the Goodsloader [%d] that item%d is out of stock",salesmanIndex,myGoodsID,itemToBeRestocked_FromCustToSalesman[complaintCustNumber[departmentOfChoice][salesmanIndex]]);
					SignalCV(goodsloaderLock[myGoodsID],goodsloaderCV[myGoodsID]);
					WaitCV(goodsloaderLock[myGoodsID],goodsloaderCV[myGoodsID]);
					itemToBeRestocked_FromSalesmanToGoodsloader[myGoodsID]=itemToBeRestocked_FromCustToSalesman[complaintCustNumber[departmentOfChoice][salesmanIndex]];
					salesGoodsId[myGoodsID]=salesmanIndex;
					SignalCV(goodsloaderLock[myGoodsID],goodsloaderCV[myGoodsID]);
					ReleaseLock(goodsloaderLock[myGoodsID]);
					itemRestockingFlag[itemToBeRestocked_FromCustToSalesman[complaintCustNumber[departmentOfChoice][salesmanIndex]]]=1;
				}
				ReleaseLock(itemLock[itemToBeRestocked_FromCustToSalesman[complaintCustNumber[departmentOfChoice][salesmanIndex]]]);
			}
			else{ /* No goodsloader is available to restock */
				
				salesStatus[departmentOfChoice][salesmanIndex] = 1;
				ReleaseLock(waitingLock[departmentOfChoice]);
				ReleaseLock(salesmanLock[departmentOfChoice][salesmanIndex]);
				for(i=0;i<50;i++){
					Yield();
				}
				continue;
			}
		}
		else /* Every queue is empty */
		{
			salesStatus[departmentOfChoice][salesmanIndex] = 0;
			ReleaseLock(waitingLock[departmentOfChoice]);
		}
		WaitCV(salesmanLock[departmentOfChoice][salesmanIndex],salesWaitingCV[departmentOfChoice][salesmanIndex]);
		ReleaseLock(salesmanLock[departmentOfChoice][salesmanIndex]);
	}
	Exit(0);
}

void goodsloader(){
	
	int departmentOfChoice,goodsFlag=-1,mySalesID,i,goodsIndex;
	/* Retrieving index */
	AcquireLock(goodsIndexLock);
	goodsIndex=globalGoodsIndex++;
	ReleaseLock(goodsIndexLock);
	
	while(1){
		AcquireLock(salesGoodsLock);
		AcquireLock(goodsloaderLock[goodsIndex]);
		if(goodsStatus[goodsIndex]==2){ /* Salesman signalled to restock */
			ReleaseLock(salesGoodsLock);
			SignalCV(goodsloaderLock[goodsIndex],goodsloaderCV[goodsIndex]);
			WaitCV(goodsloaderLock[goodsIndex],goodsloaderCV[goodsIndex]);
			AcquireLock(storeroomLock);
			AcquireLock(itemLock[itemToBeRestocked_FromSalesmanToGoodsloader[goodsIndex]]);
			departmentOfChoice=itemToBeRestocked_FromSalesmanToGoodsloader[goodsIndex]/10;
			if(departmentOfChoice==0)
				Print3("Goodsloader [%d] is informed by DepartmentSalesman [%d] of department [0] to restock item%d",goodsIndex,salesGoodsId[goodsIndex],itemToBeRestocked_FromSalesmanToGoodsloader[goodsIndex]);
			else if(departmentOfChoice==1)
				Print3("Goodsloader [%d] is informed by DepartmentSalesman [%d] of department [1] to restock item%d",goodsIndex,salesGoodsId[goodsIndex],itemToBeRestocked_FromSalesmanToGoodsloader[goodsIndex]);
			else if(departmentOfChoice==2)
				Print3("Goodsloader [%d] is informed by DepartmentSalesman [%d] of department [2] to restock item%d",goodsIndex,salesGoodsId[goodsIndex],itemToBeRestocked_FromSalesmanToGoodsloader[goodsIndex]);
			else if(departmentOfChoice==3)
				Print3("Goodsloader [%d] is informed by DepartmentSalesman [%d] of department [3] to restock item%d",goodsIndex,salesGoodsId[goodsIndex],itemToBeRestocked_FromSalesmanToGoodsloader[goodsIndex]);
			else
				Print3("Goodsloader [%d] is informed by DepartmentSalesman [%d] of department [4] to restock item%d",goodsIndex,salesGoodsId[goodsIndex],itemToBeRestocked_FromSalesmanToGoodsloader[goodsIndex]);
			
			Print2("Goodsloader [%d] is in the StockRoom and got item%d",goodsIndex,itemToBeRestocked_FromSalesmanToGoodsloader[goodsIndex]);
			quantityOnShelf[itemToBeRestocked_FromSalesmanToGoodsloader[goodsIndex]]=10;
			Print3("Goodsloader [%d] restocked item%d in Department [%d]",goodsIndex,itemToBeRestocked_FromSalesmanToGoodsloader[goodsIndex],departmentOfChoice);
			Print1("Goodsloader [%d] leaves StockRoom",goodsIndex);
			ReleaseLock(storeroomLock);
			ReleaseLock(itemLock[itemToBeRestocked_FromSalesmanToGoodsloader[goodsIndex]]);
			AcquireLock(waitingLock[departmentOfChoice]);
			goodsWaitingLineCount[departmentOfChoice]++;
			AcquireLock(salesmanLock[departmentOfChoice][salesGoodsId[goodsIndex]]);
			SignalCV(salesmanLock[departmentOfChoice][salesGoodsId[goodsIndex]],salesWaitingCV[departmentOfChoice][salesGoodsId[goodsIndex]]);
			ReleaseLock(salesmanLock[departmentOfChoice][salesGoodsId[goodsIndex]]);
			WaitCV(waitingLock[departmentOfChoice],goodsWaitingCV[departmentOfChoice]);
			
			mySalesID=-1;
			for(i=0;i<salesmanCount;i++){
				if(salesStatus[departmentOfChoice][i]==5){
					mySalesID=i;
					salesStatus[departmentOfChoice][mySalesID]=1;
					break;
				}
			}
			AcquireLock(salesmanLock[departmentOfChoice][mySalesID]);
			ReleaseLock(waitingLock[departmentOfChoice]);
			restockedItem[departmentOfChoice][mySalesID]=itemToBeRestocked_FromSalesmanToGoodsloader[goodsIndex];
			restockingGoodsloader[departmentOfChoice][mySalesID]=goodsIndex;
			SignalCV(salesmanLock[departmentOfChoice][mySalesID],salesWaitingCV[departmentOfChoice][mySalesID]);
			WaitCV(salesmanLock[departmentOfChoice][mySalesID],salesWaitingCV[departmentOfChoice][mySalesID]);
			SignalCV(salesmanLock[departmentOfChoice][mySalesID],salesWaitingCV[departmentOfChoice][mySalesID]);
			ReleaseLock(salesmanLock[departmentOfChoice][mySalesID]);
		}
		else{
			ReleaseLock(salesGoodsLock);
		}
		AcquireLock(trolleyEndLock);		
		if(trolleyEndCount>0){ /* If trolley is present at the exit, replace the trolley */
			trolleyEndCount--;
			ReleaseLock(trolleyEndLock);
			AcquireLock(trolleyFirstLock);
			trolleyFirstCount++;
			Print1("Goodsloader [%d] replaced the trolley",goodsIndex);
			if(trolleyWaitingCount>0){
				trolleyWaitingCount--;
				SignalCV(trolleyFirstLock,trolleyWaitingCV);
			}
			ReleaseLock(trolleyFirstLock);
		}
		else{
			ReleaseLock(trolleyEndLock);
		}
		AcquireLock(salesGoodsLock);	
		goodsStatus[goodsIndex]=0;
		ReleaseLock(salesGoodsLock);
		
		Print1("Goodsloader [%d] is waiting for orders to restock",goodsIndex);
		WaitCV(goodsloaderLock[goodsIndex],goodsloaderCV[goodsIndex]);
		ReleaseLock(goodsloaderLock[goodsIndex]);
	}
	Exit(0);
}

void customer()
{
	int mySalesID1,mySalesID2,i,itemToBuy,salesBusyCount=0,departmentOfChoice;
	int item,salesFlag=-1,j,shortestLineLength = 0,mypreviouscashiersId=-1;
	int shortestLineIndex = 0,mycashiersId=-1,breakFlag=0,tempShortestValue=100;
	int customerIndex;
	
	AcquireLock(customerIndexLock);
	customerIndex=globalCustomerIndex++;
	ReleaseLock(customerIndexLock);
	
	if(Customer[customerIndex].privilegeStatus==1)
		Print1("Privileged Customer [%d] enters the SuperMarket",customerIndex);
	else
		Print1("Customer [%d] enters the SuperMarket",customerIndex);
		
	if(Customer[customerIndex].privilegeStatus==1)
		Print2("Privileged Customer [%d] wants to buy [%d] no. of items",customerIndex,Customer[customerIndex].itemCount);
	else
			Print2("Customer [%d] wants to buy [%d] no. of items",customerIndex,Customer[customerIndex].itemCount);
	
	AcquireLock(trolleyFirstLock);
	if(trolleyFirstCount>0){
		trolleyFirstCount--;
		if(Customer[customerIndex].privilegeStatus==1)
			Print1("Privileged Customer [%d] has a trolley for shopping",customerIndex);
		else
			Print1("Customer [%d] has a trolley for shopping",customerIndex);
		ReleaseLock(trolleyFirstLock);
	}
	else{
		trolleyWaitingCount++;
		if(Customer[customerIndex].privilegeStatus==1)
			Print1("Privileged Customer [%d] gets in line for trolley",customerIndex);
		else
			Print1("Customer [%d] gets in line for trolley",customerIndex);
		
		WaitCV(trolleyFirstLock,trolleyWaitingCV);
		if(trolleyFirstCount>0){
			trolleyFirstCount--;
			if(Customer[customerIndex].privilegeStatus==1)
				Print1("Privileged Customer [%d] has a trolley for shopping",customerIndex);
			else
				Print1("Customer [%d] has a trolley for shopping",customerIndex);
		}
		ReleaseLock(trolleyFirstLock);
	}
	for(item=0;item<Customer[customerIndex].itemCount;item++){ /* Buying every item in the list */
		itemToBuy=Customer[customerIndex].itemsOfChoice[item];
		departmentOfChoice=itemToBuy/10;
		AcquireLock(waitingLock[departmentOfChoice]);
		if(Customer[customerIndex].privilegeStatus==1)
			Print3("Privileged Customer [%d] enters the department%d for item%d",customerIndex,departmentOfChoice,itemToBuy);
		else
			Print3("Customer [%d] enters the department%d for item%d",customerIndex,departmentOfChoice,itemToBuy);
		salesFlag=-1;
		for(i=0;i<salesmanCount;i++){ /* Checking if any salesman in the department is available */
			if(salesStatus[departmentOfChoice][i]==0){
				mySalesID1=i;
				AcquireLock(salesmanLock[departmentOfChoice][mySalesID1]);
				salesStatus[departmentOfChoice][i]=1;
				salesFlag=1;
				break;
			}
		}
		if(salesFlag==1){ /* Salesman is available */
			custWaitingLineCount[departmentOfChoice]++;
			salesCustNumber[departmentOfChoice][mySalesID1]=customerIndex;
			SignalCV(salesmanLock[departmentOfChoice][mySalesID1],salesWaitingCV[departmentOfChoice][mySalesID1]);
			ReleaseLock(salesmanLock[departmentOfChoice][mySalesID1]);
			WaitCV(waitingLock[departmentOfChoice],custWaitingCV[departmentOfChoice]);
			
		}
		else{ /* All the salesamn are busy */
			custWaitingLineCount[departmentOfChoice]++;
			if(Customer[customerIndex].privilegeStatus==1)
				Print2("Privileged Customer [%d] gets in line for the department [%d]",customerIndex,departmentOfChoice);
			else
				Print2("Customer [%d] gets in line for the department [%d]",customerIndex,departmentOfChoice);
				
			WaitCV(waitingLock[departmentOfChoice],custWaitingCV[departmentOfChoice]);
		}
		
		for(i=0;i<salesmanCount;i++){ /* Identifying which salesman signalled  */
			if(salesStatus[departmentOfChoice][i]==3){
				mySalesID1=i;
				AcquireLock(salesmanLock[departmentOfChoice][mySalesID1]);
				salesStatus[departmentOfChoice][mySalesID1]=1;
				break;
			}
		}
		
		/* Interaction with salesman */
		salesCustNumber[departmentOfChoice][mySalesID1]=customerIndex;
		ReleaseLock(waitingLock[departmentOfChoice]);
		SignalCV(salesmanLock[departmentOfChoice][mySalesID1],salesWaitingCV[departmentOfChoice][mySalesID1]);
		WaitCV(salesmanLock[departmentOfChoice][mySalesID1],salesWaitingCV[departmentOfChoice][mySalesID1]);
		if(Customer[customerIndex].privilegeStatus==1)
			Print3("Privileged Customer [%d] is interacting with DepartmentSalesman[%d] of Department[%d]",customerIndex,mySalesID1,departmentOfChoice);
		else
				Print3("Customer [%d] is interacting with DepartmentSalesman[%d] of Department[%d]",customerIndex,mySalesID1,departmentOfChoice);
		SignalCV(salesmanLock[departmentOfChoice][mySalesID1],salesWaitingCV[departmentOfChoice][mySalesID1]);
		
		ReleaseLock(salesmanLock[departmentOfChoice][mySalesID1]);
		
		/* Buying the item */
		AcquireLock(itemLock[itemToBuy]);
		while(1){
			if(quantityOnShelf[itemToBuy]>=Customer[customerIndex].itemQuantity[item]){
				quantityOnShelf[itemToBuy]-=Customer[customerIndex].itemQuantity[item];
				if(Customer[customerIndex].privilegeStatus==1)
					Print3("Privileged Customer [%d] has found item%d and placed [%d] in the trolly",customerIndex,itemToBuy,Customer[customerIndex].itemQuantity[item]);
				else
					Print3("Privileged Customer [%d] has found item%d and placed [%d] in the trolly",customerIndex,itemToBuy,Customer[customerIndex].itemQuantity[item]);
				Customer[customerIndex].itemBoughtCount++;
				ReleaseLock(itemLock[itemToBuy]);
				break;
			}
			else{ /* Item is not available */
				AcquireLock(waitingLock[departmentOfChoice]);
				ReleaseLock(itemLock[itemToBuy]);
				salesFlag=-1;
				for(i=0;i<salesmanCount;i++){
					
					if(salesStatus[departmentOfChoice][i]==0){
						mySalesID2=i;
						AcquireLock(salesmanLock[departmentOfChoice][mySalesID2]);
						salesStatus[departmentOfChoice][mySalesID2]=1;
						salesFlag=1;
						break;
					}
				}
				if(salesFlag==-1){
					if(Customer[customerIndex].privilegeStatus==1)
						Print2("Privileged Customer [%d] is not able to find item%d and is searching for DepartmentSalesman",customerIndex,itemToBuy);
					else
						Print2("Customer [%d] is not able to find item%d and is searching for DepartmentSalesman",customerIndex,itemToBuy);
					complainingCustWaitingLineCount[departmentOfChoice]++;
					WaitCV(waitingLock[departmentOfChoice],complainingCustWaitingCV[departmentOfChoice]);
				}
				else{
					SignalCV(salesmanLock[departmentOfChoice][mySalesID2],salesWaitingCV[departmentOfChoice][mySalesID2]);
					complainingCustWaitingLineCount[departmentOfChoice]++;
					ReleaseLock(salesmanLock[departmentOfChoice][mySalesID2]);
					WaitCV(waitingLock[departmentOfChoice],complainingCustWaitingCV[departmentOfChoice]);
				}
				for(i=0;i<salesmanCount;i++){
					AcquireLock(salesmanLock[departmentOfChoice][i]);
					if(salesStatus[departmentOfChoice][i]==4){
						mySalesID2=i;
						salesStatus[departmentOfChoice][mySalesID2]=1;
						break;
					}
					ReleaseLock(salesmanLock[departmentOfChoice][i]);
				}
				
				if(Customer[customerIndex].privilegeStatus==1)
					Print2("Privileged Customer [%d] is asking assistance from DepartmentSalesman [%d]",customerIndex,mySalesID2);
				else
					Print2("Customer [%d] is asking assistance from DepartmentSalesman [%d]",customerIndex,mySalesID2);
				complaintCustNumber[departmentOfChoice][mySalesID2]=customerIndex;
				itemToBeRestocked_FromCustToSalesman[customerIndex]=itemToBuy;
				ReleaseLock(waitingLock[departmentOfChoice]);
				/* Finished complaining to the salesman about the item */
				SignalCV(salesmanLock[departmentOfChoice][mySalesID2],salesWaitingCV[departmentOfChoice][mySalesID2]);
				if(Customer[customerIndex].privilegeStatus==1)
					Print3("Privileged Customer [%d] has received assistance about restocking of item%d from DepartmentSalesman [%d]",customerIndex,itemToBuy,mySalesID2);
				else
					Print3("Customer [%d] has received assistance about restocking of item%d from DepartmentSalesman [%d]",customerIndex,itemToBuy,mySalesID2);
				AcquireLock(itemLock[itemToBuy]);				
				ReleaseLock(salesmanLock[departmentOfChoice][mySalesID2]);
				itemWaitingLinecount[itemToBuy]++;
				WaitCV(itemLock[itemToBuy],itemWaitingCV[itemToBuy]);
			}
		}/* while */		
	}/* Item for loop */
		
	/*-------------------------------------Customer cashier Interaction-----------------------------------------------*/
	AcquireLock(totalLineCountLock);
	if(Customer[customerIndex].privilegeStatus == 1)
		Print1("Privileged Customer [%d] has finished shopping and is looking for the Cashier",customerIndex);
	else
		Print1("Customer [%d] has finished shopping and is looking for the Cashier",customerIndex);
	while(1){
		for(j=0;j<cashierCount;j++)
		{
			if(cashierStatus[j] == 0)
			{
				mycashiersId = j;
				cashierStatus[j]=1;
				break;
			}
		}
		if(mycashiersId == -1)  /* No cashier is free, gets in the shortestline where the cashier is not on break */
		{
			shortestLineLength = tempShortestValue;
			shortestLineIndex = -1;
			if(Customer[customerIndex].privilegeStatus==0)
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
			
			if(Customer[customerIndex].privilegeStatus == 1)
				Print3("Privileged Customer [%d] chose Cashier [%d] with line of length [%d]",customerIndex,mycashiersId/2,shortestLineLength);
			else
				Print3("Customer [%d] chose Cashier [%d] with line of length [%d]",customerIndex,mycashiersId/2,shortestLineLength);
				
			WaitCV(totalLineCountLock,cashCustLineCV[mycashiersId]);
			mycashiersId/=2;
			Print3("Customer [%d] was signalled by Cashier [%d] with line of length [%d]",customerIndex,mycashiersId/2,shortestLineLength);
			if(cashierStatus[mycashiersId]!=2){
				break;
			}else{
				mycashiersId=-1;				
			}
		}
		else{
			if(Customer[customerIndex].privilegeStatus == 1)
			{
				Print3("Privileged Customer [%d] chose Cashier [%d] with line of length [%d]",customerIndex,mycashiersId,shortestLineLength);
				break;
			}
			else
			{
				Print3("Customer [%d] chose Cashier [%d] with line of length [%d]",customerIndex,mycashiersId,shortestLineLength);
				break;
			}
		}
	}
	ReleaseLock(totalLineCountLock);	
	AcquireLock(cashCustIntrLock[mycashiersId]);
	myCashCustomerid[mycashiersId] = customerIndex;
	SignalCV(cashCustIntrLock[mycashiersId],cashCustIntrCV[mycashiersId]);
	WaitCV(cashCustIntrLock[mycashiersId],cashCustIntrCV[mycashiersId]);
	
	/* Interacted with cashier and go the info about the money status */
	if(customerMoneyStatus[customerIndex]==1){
		if(Customer[customerIndex].privilegeStatus == 1) /* Money is not enough */
			Print2("Privileged Customer [%d] cannot pay $[%d]",customerIndex,Customer[customerIndex].totalPurchasedCost);
		else
			Print2("Customer [%d] cannot pay $[%d]",customerIndex,Customer[customerIndex].totalPurchasedCost);
			
		AcquireLock(customerManagerLineLock);
		ReleaseLock(cashCustIntrLock[mycashiersId]);
		customerManagerLineCount++;
		
		if(Customer[customerIndex].privilegeStatus == 1)
			Print1("Privileged Customer [%d] is waiting for Manager for negotiations.",customerIndex);
		else
			Print1("Customer [%d] is waiting for Manager for negotiations.",customerIndex);
		
		/* Waits for the manager */
		WaitCV(customerManagerLineLock,customerManagerLineCV);		
		AcquireLock(customerManagerIntrLock);
		ReleaseLock(customerManagerLineLock);
		managerCustomerId=customerIndex;
		SignalCV(customerManagerIntrLock,customerManagerIntrCV);
		WaitCV(customerManagerIntrLock,customerManagerIntrCV);
		
		/* Paid the money after negotiations*/
		if(Customer[customerIndex].privilegeStatus == 1)
			Print2("Privileged Customer [%d] pays $[%d] to Manager after removing items and is waiting for receipt from Manager. ",customerIndex,Customer[customerIndex].totalPurchasedCost);
		else
			Print2("Customer [%d] pays $[%d] to Manager after removing items and is waiting for receipt from Manager. ",customerIndex,Customer[customerIndex].totalPurchasedCost);
		
		SignalCV(customerManagerIntrLock,customerManagerIntrCV);
		WaitCV(customerManagerIntrLock,customerManagerIntrCV);
		
		/* Received the receipt from manager */
		if(Customer[customerIndex].privilegeStatus == 1)
			Print1("Privileged Customer [%d] got receipt from Manager and is now leaving.",customerIndex);
		else
			Print1("Customer [%d] got receipt from Manager and is now leaving.",customerIndex);
			
		ReleaseLock(customerManagerIntrLock);	
	}
	else{
		if(Customer[customerIndex].privilegeStatus == 1) /* Money is enough */
			Print3("Privileged Customer [%d] pays $[%d] to Cashier [%d] and is now waiting for receipt",customerIndex,Customer[customerIndex].totalPurchasedCost,mycashiersId);
		else
			Print3("Customer [%d] pays $[%d] to Cashier [%d] and is now waiting for receipt",customerIndex,Customer[customerIndex].totalPurchasedCost,mycashiersId);
		ReleaseLock(cashCustIntrLock[mycashiersId]);
	}
	
	AcquireLock(trolleyEndLock);
	trolleyEndCount++;
	if(Customer[customerIndex].privilegeStatus==1)
		Print1("Privileged Customer [%d] is leaving the trolley",customerIndex);
	ReleaseLock(trolleyEndLock);
	custCountTemp--;
	if(Customer[customerIndex].privilegeStatus==1)
		Print1("Privileged Customer [%d] exits the store",customerIndex);
	else
		Print1("Customer [%d] exits the store",customerIndex);
	Exit(0);
}

void cashier()
{
	int j,cashierIndex;
	
	AcquireLock(cashierIndexLock);
	cashierIndex=globalCashierIndex++;
	ReleaseLock(cashierIndexLock);
	
	while(1)
	{
		AcquireLock(totalLineCountLock);
		AcquireLock(cashierBreakLock[cashierIndex]);
		if(cashierStatus[cashierIndex]==2){ /* Cashier is sent on break by the manager */
			cashierBreakUsageFlag[cashierIndex] = 1;
			BroadcastCV(totalLineCountLock,cashCustLineCV[cashierIndex*2+1]);
			BroadcastCV(totalLineCountLock,cashCustLineCV[cashierIndex*2]);
			cashCustLineCount[cashierIndex*2+1] = 0;
			cashCustLineCount[cashierIndex*2] = 0;
			ReleaseLock(totalLineCountLock);
			Print1("Cashier [%d] is going on break.",cashierIndex);
			WaitCV(cashierBreakLock[cashierIndex],cashierBreakCV[cashierIndex]);
			Print1("Cashier [%d] was called from break by Manager to work.",cashierIndex);
			ReleaseLock(cashierBreakLock[cashierIndex]);
			AcquireLock(totalLineCountLock);
		}
		else
		{
			ReleaseLock(cashierBreakLock[cashierIndex]);
		}
		
		/* Signals the privileged queue if there are customers*/
		if(cashCustLineCount[cashierIndex*2+1]>0)
		{
			cashCustLineCount[cashierIndex*2+1]--;
			cashierStatus[cashierIndex] = 1;
			SignalCV(totalLineCountLock,cashCustLineCV[cashierIndex*2+1]);
		}
		else if(cashCustLineCount[cashierIndex*2]>0) /* Signals the normal queue */
		{
			cashCustLineCount[cashierIndex*2]--;
			cashierStatus[cashierIndex] = 1;
			SignalCV(totalLineCountLock,cashCustLineCV[cashierIndex*2]);
		}
		else	
		{
			cashierStatus[cashierIndex] = 0;
		}
		
		AcquireLock(cashCustIntrLock[cashierIndex]);
		ReleaseLock(totalLineCountLock);
		WaitCV(cashCustIntrLock[cashierIndex],cashCustIntrCV[cashierIndex]);
		AcquireLock(totalLineCountLock);
		if(cashierStatus[cashierIndex]!=2){
			ReleaseLock(totalLineCountLock);
			for(j=0; j<Customer[myCashCustomerid[cashierIndex]].itemCount; j++)
			{
				if(Customer[myCashCustomerid[cashierIndex]].privilegeStatus == 1)
					Print3("Cashier [%d] got [%d] from trolly of Privileged Customer [%d].",cashierIndex,Customer[myCashCustomerid[cashierIndex]].itemsOfChoice[j],myCashCustomerid[cashierIndex]);
				else
					Print3("Cashier [%d] got [%d] from trolly of Customer [%d].",cashierIndex,Customer[myCashCustomerid[cashierIndex]].itemsOfChoice[j],myCashCustomerid[cashierIndex]);
				
				Customer[myCashCustomerid[cashierIndex]].totalPurchasedCost += Customer[myCashCustomerid[cashierIndex]].itemQuantity[j];
			}
			
			if(Customer[myCashCustomerid[cashierIndex]].privilegeStatus == 1)
				Print3("Cashier [%d] tells Privileged Customer [%d] total cost is $[%d].",cashierIndex,myCashCustomerid[cashierIndex],Customer[myCashCustomerid[cashierIndex]].totalPurchasedCost);
			else
				Print3("Cashier [%d] tells Customer [%d] total cost is $[%d].",cashierIndex,myCashCustomerid[cashierIndex],Customer[myCashCustomerid[cashierIndex]].totalPurchasedCost);
			if(Customer[myCashCustomerid[cashierIndex]].totalPurchasedCost > Customer[myCashCustomerid[cashierIndex]].moneyInPocket) /* Customer doesnt have enough money */
			{
				if(Customer[myCashCustomerid[cashierIndex]].privilegeStatus == 1)
					Print2("Cashier [%d] asks Privileged Customer [%d] to wait for Manager.",cashierIndex,myCashCustomerid[cashierIndex]);
				else
					Print2("Cashier [%d] asks Customer [%d] to wait for Manager.",cashierIndex,myCashCustomerid[cashierIndex]);
				customerMoneyStatus[myCashCustomerid[cashierIndex]]=1;
				
				AcquireLock(cashierManagerLineLock);
				cashierManagerLineCount++;
				
				/* Waits to report to the manager */
				WaitCV(cashierManagerLineLock,cashierManagerLineCV);
				if(Customer[myCashCustomerid[cashierIndex]].privilegeStatus == 1)
					Print2("Cashier [%d] informs the Manager that Privileged Customer [%d] does not have enough money.",cashierIndex,myCashCustomerid[cashierIndex]);
				else
					Print2("Cashier [%d] informs the Manager that Customer [%d] does not have enough money.",cashierIndex,myCashCustomerid[cashierIndex]);
				ReleaseLock(cashierManagerLineLock);
				
			}
			else /* Customer have enough money */
			{
				if(Customer[myCashCustomerid[cashierIndex]].privilegeStatus == 1)
					Print2("Cashier [%d] gave the receipt to Privileged Customer [%d] and tells him to leave.",cashierIndex,myCashCustomerid[cashierIndex]);
				else
					Print2("Cashier [%d] gave the receipt to Customer [%d] and tells him to leave.",cashierIndex,myCashCustomerid[cashierIndex]);
				customerMoneyStatus[myCashCustomerid[cashierIndex]]=0;
				if(Customer[myCashCustomerid[cashierIndex]].privilegeStatus == 1)
					Print3("Cashier [%d] got money $[%d] from Privileged Customer [%d].",cashierIndex,Customer[myCashCustomerid[cashierIndex]].totalPurchasedCost,myCashCustomerid[cashierIndex] );
				else
					Print3("Cashier [%d] got money $[%d] from Customer [%d].",cashierIndex,Customer[myCashCustomerid[cashierIndex]].totalPurchasedCost,myCashCustomerid[cashierIndex] );
				AcquireLock(totalSalesCashierLock);
				cashiersSalesBackup[cashierIndex] += Customer[myCashCustomerid[cashierIndex]].totalPurchasedCost;
				totalSalesCashier[cashierIndex] += Customer[myCashCustomerid[cashierIndex]].totalPurchasedCost;
				ReleaseLock(totalSalesCashierLock);
			}
			SignalCV(cashCustIntrLock[cashierIndex],cashCustIntrCV[cashierIndex]);
			ReleaseLock(cashCustIntrLock[cashierIndex]);	
		}
		else{	
			ReleaseLock(totalLineCountLock);
		}
	}
}

void manager(){
	int breakCount=0,activeCount=0,i,j,w,randomBreakCashier[5],randomNmbr,activeCashiers[5];
	int breakSettingFlag=0;
		
	while(custCountTemp>=0){
		AcquireLock(cashierManagerLineLock);
		if(cashierManagerLineCount>0){
			cashierManagerLineCount=0;
			BroadcastCV(cashierManagerLineLock,cashierManagerLineCV);
		}
		ReleaseLock(cashierManagerLineLock);
		AcquireLock(customerManagerLineLock);
		if(customerManagerLineCount>0){ /* If Customers are waiting because of money shortage */
			customerManagerLineCount--;
			SignalCV(customerManagerLineLock,customerManagerLineCV);
			AcquireLock(customerManagerIntrLock);
			ReleaseLock(customerManagerLineLock);
			WaitCV(customerManagerIntrLock,customerManagerIntrCV);
			w=Customer[managerCustomerId].itemCount-1;
			do{ /* Remove the item from the trolley */
				Customer[managerCustomerId].totalPurchasedCost-=Customer[managerCustomerId].itemQuantity[w];
				Customer[managerCustomerId].itemQuantity[w]=0;
				if(Customer[managerCustomerId].privilegeStatus == 1)
					Print2("Manager removes item%d from the trolly of Privileged Customer [%d].",Customer[managerCustomerId].itemsOfChoice[w],managerCustomerId);
				else
					Print2("Manager removes item%d from the trolly of Customer [%d].",Customer[managerCustomerId].itemsOfChoice[w],managerCustomerId);
				w--;
			}while(Customer[managerCustomerId].totalPurchasedCost>Customer[managerCustomerId].moneyInPocket);
			
			SignalCV(customerManagerIntrLock,customerManagerIntrCV);
			WaitCV(customerManagerIntrLock,customerManagerIntrCV);
			managerSalesBackup += Customer[managerCustomerId].totalPurchasedCost;
			totalSales+=Customer[managerCustomerId].totalPurchasedCost;
			SignalCV(customerManagerIntrLock,customerManagerIntrCV);
			if(Customer[managerCustomerId].privilegeStatus == 1)
				Print1("Manager gives receipt to Privileged Customer [%d].",managerCustomerId);
			else
				Print1("Manager gives receipt to Customer [%d].",managerCustomerId);
			ReleaseLock(customerManagerIntrLock);
		}
		else{
			ReleaseLock(customerManagerLineLock);
		}
		AcquireLock(totalSalesCashierLock);
		
		for(i=0;i<cashierCount;i++){ /* Emptying cashiers' drawers */
			Print1("Manager emptied Counter [%d] drawer.",i);
			totalSales+=totalSalesCashier[i];
			totalSalesCashier[i]=0;
			Print1("Manager has total sale of $[%d]",totalSales);
			Print2("Cashier %d Status : %d",i,cashierStatus[i]);
		}
		
		ReleaseLock(totalSalesCashierLock);
		
		
		if(custCountTemp==0){ /* Every customer left the store, print the total sales at the store */
			custCountTemp--;
			for(i=0;i<cashierCount;i++)
			{
				Print2("Total Sales from Counter [%d] is $[%d]",i,cashiersSalesBackup[i]);
			}
			Print1("Total Sales from Manager counter is $[%d]",managerSalesBackup);
			Print1("Total Sale of the entire store is $[%d]",totalSales);
		}
		
		
		for(i=0;i<1000;i++){
			Yield();
		}
		
		
		activeCount=0;
		j=0;
		AcquireLock(totalLineCountLock);
		for(i=0;i<cashierCount;i++){
			if(cashierStatus[i]!=2){ /* Find the active cashiers to send anyone of them on break */
				activeCount++;
				activeCashiers[j]=i;
				j++;
			}
		}
		
		if(activeCount>0)
		{
			randomNmbr=RandomSearch()%activeCount;
			
			j=0;
		
			for(i=0;i<cashierCount;i++){
				
				if(j==randomNmbr)
					break;
					
				if(i==activeCashiers[j]){ /* Send someone to break */
						AcquireLock(cashierBreakLock[i]);
						if(cashierStatus[i]==0){
							AcquireLock(cashCustIntrLock[i]);
							SignalCV(cashCustIntrLock[i],cashCustIntrCV[i]);
							ReleaseLock(cashCustIntrLock[i]);
						}
						cashierStatus[i]=2;
						breakCount++;
						Print1("Manager sends Cashier [%d] on break.",i);
						ReleaseLock(cashierBreakLock[i]);
					}
				
				j++;
			}
			ReleaseLock(totalLineCountLock);
		}
		else{
			ReleaseLock(totalLineCountLock);
		}
					
		for(i=0;i<10000;i++){
			Yield();
		}
		
		AcquireLock(totalLineCountLock);
		
		for(i=0;i<cashierCount;i++){
			if(cashierStatus[i]!=2 && (cashCustLineCount[i*2+1]+cashCustLineCount[i*2])>=3){ /* Bring back cashiers from break if any line count > 3 */
				for(j=0;j<cashierCount;j++){
					AcquireLock(cashierBreakLock[j]);
					if(cashierStatus[j]==2 && cashierBreakUsageFlag[j]==1){
						cashierStatus[j]=1;
						cashierBreakUsageFlag[j] = 0;						
						SignalCV(cashierBreakLock[j],cashierBreakCV[j]);
						Print1("Manager brings back Cashier [%d] from break.",j);
						breakCount--;
					}
					ReleaseLock(cashierBreakLock[j]);
				}
				break;
			}
		}
		ReleaseLock(totalLineCountLock);
		for(i=0;i<100;i++){
			Yield();
		}
	}
	Exit(0);
}

int main()
{	
	int i,j;
	/*Hardcoding user inputs*/
	departmentsCount=3;
	Print1("\nDepartments : %d",departmentsCount);
	salesmanCount=3;
	Print1("\nSalesmen per Department : %d",salesmanCount);
	customersCount=30;
	custCountTemp=customersCount;
	Print1("\nCustomers : %d",customersCount);
	goodsloaderCount=3;
	Print1("\nGoodsloaders : %d",goodsloaderCount);
	cashierCount=3;
	Print1("\nCashiers : %d",cashierCount);
	
	Flag();
	/*Initializing data structurs, locks and CVs*/
	for(i=0;i<goodsloaderCount;i++){
		itemToBeRestocked_FromSalesmanToGoodsloader[i]=0;
		goodsStatus[i]=1;
		goodsloaderLock[i] = CreateLock(" ",1);
		goodsloaderCV[i]=CreateCV(" ",1);
	}
	
	customerIndexLock = CreateLock(" ",1);
	cashierIndexLock = CreateLock(" ",1);
	salesmanIndexLock = CreateLock(" ",1);
	goodsIndexLock = CreateLock(" ",1);
	
	trolleyFirstLock=CreateLock(" ",1);
	trolleyEndLock=CreateLock(" ",1);
	trolleyWaitingCV=CreateCV(" ",1);
	
	createCustomerInformation(customersCount,departmentsCount);
	setInitialItemQuantities(departmentsCount);
	
	
	cashierManagerLineCV=CreateCV(" ",1);
	cashierManagerLineLock=CreateLock(" ",1);
	customerManagerLineCV=CreateCV(" ",1);
	customerManagerLineLock=CreateLock(" ",1);
	customerManagerIntrLock=CreateLock(" ",1);
	customerManagerIntrCV=CreateCV(" ",1);
	totalSalesCashierLock=CreateLock(" ",1);
	
	for(i=0;i<cashierCount;i++)
	{
	 cashierStatus[i]=1;
	 totalSalesCashier[i]=0;
	 custCashWaitingCV[i] = CreateCV(" ",1);
	 cashCustIntrCV[i] = CreateCV(" ",1);
	 cashCustIntrLock[i] = CreateLock(" ",1);
	 cashierBreakCV[i] = CreateCV(" ",1);
	 cashierBreakLock[i] = CreateLock(" ",1);
	}
	
	for(i=0;i<cashierCount*2;i++)
	{
	cashCustLineCV[i]  = CreateCV(" ",1);
	}
	
	totalLineCountLock = CreateLock(" ",1);
	
	
	
	for(i=0;i<customersCount;i++)
	{
		Print1("Customer%d",Customer[i].customerId);
		Customer[i].itemBoughtCount=0;
		for(j=0;j<Customer[i].itemCount;j++)
		{
			Print2("\tItem%d->Quantity%d",Customer[i].itemsOfChoice[j],Customer[i].itemQuantity[j]);
		}
		
		if(Customer[i].privilegeStatus == 0)
			Print("Unprevileged Customer");
		else
			Print("Privileged Customer");
	}
	
	salesGoodsLock=CreateLock(" ",1);
	storeroomLock=CreateLock(" ",1);
	for(i=0;i<departmentsCount;i++)
	{
		for(j=0;j<salesmanCount;j++)
		{
			salesWaitingCV[i][j] =  CreateCV(" ",1);
			salesmanLock[i][j] = CreateLock(" ",1);
			salesStatus[i][j] = 1;
		}
	}
	for(j=0;j<departmentsCount;j++)
	{
		goodsWaitingCV[j]=CreateCV(" ",1);
		waitingLock[j] = CreateLock(" ",1);	
		custWaitingCV[j] =  CreateCV(" ",1);
		custWaitingLineCount[j] = 0;
		complainingCustWaitingLineCount[j]=0;
		complainingCustWaitingCV[j] = CreateCV(" ",1);
	}
	for(i=0;i<(departmentsCount*10);i++){
		itemLock[i]=CreateLock(" ",1);
		itemWaitingCV[i]=CreateCV(" ",1);
		itemWaitingLinecount[i]=0;
	}
	
	/*Forking necessary entities*/
	for(i=0;i<(departmentsCount*salesmanCount);i++)
	{
		Fork(salesman);	
	}
	for(i=0;i<cashierCount;i++)
	{
		Fork(cashier);	
	}
	for(i=0;i<customersCount;i++)
	{
		Fork(customer);
	}
	for(i=0;i<goodsloaderCount;i++)
	{
		Fork(goodsloader);
	}		
	Fork(manager);	
	Exit(0);
}