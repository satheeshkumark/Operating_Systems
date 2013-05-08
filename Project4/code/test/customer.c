#include "syscall.h"
#include "setup.h"

int main()
{
	int mySalesID1,mySalesID2,i,itemToBuy,salesBusyCount=0,departmentOfChoice;
	int item,salesFlag=-1,j,shortestLineLength = 0,mypreviouscashiersId=-1;
	int shortestLineIndex = 0,mycashiersId=-1,breakFlag=0,tempShortestValue=100;
	int customerIndex;
	int mValue1,mValue2,mValue3;
	
	initialize();
	
	AcquireLock(customerIndexLock);
	Print1("\nInside customer, globalCustomerIndex : %d",globalCustomerIndex); 
	customerIndex = GetMV(globalCustomerIndex);
	Print1("\nCustomer Index : %d",customerIndex);
	SetMV(globalCustomerIndex,customerIndex+1);
	ReleaseLock(customerIndexLock);
	
	/*mValue1 = GetMV(Customer[customerIndex].itemCount);
	for(j=0;j<mValue1;j++)
	{
			Print2("\nItem%d->Quantity%d",GetMV(Customer[customerIndex].itemsOfChoice[j]),GetMV(Customer[customerIndex].itemQuantity[j]));
	}
		
	if(GetMV(Customer[customerIndex].privilegeStatus) == 0)
		Print1("\nUnprevileged Customer%d",customerIndex);
	else
		Print1("\nPrivileged Customer%d",customerIndex);*/
	
	if(GetMV(Customer[customerIndex].privilegeStatus)==1)
		Print1("\nPrivileged Customer [%d] enters the SuperMarket",customerIndex);
	else
		Print1("\nCustomer [%d] enters the SuperMarket",customerIndex);
		
	if(GetMV(Customer[customerIndex].privilegeStatus)==1)
		Print2("\nPrivileged Customer [%d] wants to buy [%d] no. of items",customerIndex,mValue1);
	else
			Print2("\nCustomer [%d] wants to buy [%d] no. of items",customerIndex,mValue1);
	
	AcquireLock(trolleyFirstLock);
	
	mValue1 = GetMV(trolleyFirstCount);

	if(mValue1>0){
		SetMV(trolleyFirstCount,--mValue1);
		if(GetMV(Customer[customerIndex].privilegeStatus)==1)
			Print1("\nPrivileged Customer [%d] has a trolley for shopping",customerIndex);
		else
			Print1("\nCustomer [%d] has a trolley for shopping",customerIndex);
		ReleaseLock(trolleyFirstLock);
	}
	else{
		mValue2 = GetMV(trolleyWaitingCount);
		SetMV(trolleyWaitingCount,++mValue2);
		if(GetMV(Customer[customerIndex].privilegeStatus)==1)
			Print1("\nPrivileged Customer [%d] gets in line for trolley",customerIndex);
		else
			Print1("\nCustomer [%d] gets in line for trolley",customerIndex);
		
		WaitCV(trolleyFirstLock,trolleyWaitingCV);
		mValue1 = GetMV(trolleyFirstCount);
		if(mValue1>0){
			SetMV(trolleyFirstCount,--mValue1);
			if(GetMV(Customer[customerIndex].privilegeStatus)==1)
				Print1("\nPrivileged Customer [%d] has a trolley for shopping",customerIndex);
			else
				Print1("\nCustomer [%d] has a trolley for shopping",customerIndex);
		}
		ReleaseLock(trolleyFirstLock);
	}
	
	mValue3 = GetMV(Customer[customerIndex].itemCount);
	for(item=0;item<mValue3;item++){ /* Buying every item in the list */
		itemToBuy=GetMV(Customer[customerIndex].itemsOfChoice[item]);
		departmentOfChoice=itemToBuy/10;
		AcquireLock(waitingLock[departmentOfChoice]);
		if(GetMV(Customer[customerIndex].privilegeStatus)==1)
			Print3("\nPrivileged Customer [%d] enters the department%d for item%d",customerIndex,departmentOfChoice,itemToBuy);
		else
			Print3("\nCustomer [%d] enters the department%d for item%d",customerIndex,departmentOfChoice,itemToBuy);
		salesFlag=-1;
		for(i=0;i<salesmanCount;i++){ /* Checking if any salesman in the department is available */
			
			if(GetMV(salesStatus[departmentOfChoice][i])==0){
				mySalesID1=i;
				AcquireLock(salesmanLock[departmentOfChoice][mySalesID1]);
				SetMV(salesStatus[departmentOfChoice][i],1);
				salesFlag=1;
				break;
			}
		}
		if(salesFlag==1){ /* Salesman is available */
			mValue2 = GetMV(custWaitingLineCount[departmentOfChoice]);
			SetMV(custWaitingLineCount[departmentOfChoice],++mValue2);
			SetMV(salesCustNumber[departmentOfChoice][mySalesID1],customerIndex);
			SignalCV(salesmanLock[departmentOfChoice][mySalesID1],salesWaitingCV[departmentOfChoice][mySalesID1]);
			ReleaseLock(salesmanLock[departmentOfChoice][mySalesID1]);
			WaitCV(waitingLock[departmentOfChoice],custWaitingCV[departmentOfChoice]);
			
		}
		else{ /* All the salesamn are busy */
			mValue2 = GetMV(custWaitingLineCount[departmentOfChoice]);
			SetMV(custWaitingLineCount[departmentOfChoice],++mValue2);
			if(GetMV(Customer[customerIndex].privilegeStatus)==1)
				Print2("\nPrivileged Customer [%d] gets in line for the department [%d]",customerIndex,departmentOfChoice);
			else
				Print2("\nCustomer [%d] gets in line for the department [%d]",customerIndex,departmentOfChoice);
				
			WaitCV(waitingLock[departmentOfChoice],custWaitingCV[departmentOfChoice]);
		}
		
		for(i=0;i<salesmanCount;i++){ /* Identifying which salesman signalled  */
			if(GetMV(salesStatus[departmentOfChoice][i])==3){
				mySalesID1=i;
				AcquireLock(salesmanLock[departmentOfChoice][mySalesID1]);
				SetMV(salesStatus[departmentOfChoice][mySalesID1],1);
				break;
			}
		}
		
		/* Interaction with salesman */
		SetMV(salesCustNumber[departmentOfChoice][mySalesID1],customerIndex);
		ReleaseLock(waitingLock[departmentOfChoice]);
		SignalCV(salesmanLock[departmentOfChoice][mySalesID1],salesWaitingCV[departmentOfChoice][mySalesID1]);
		WaitCV(salesmanLock[departmentOfChoice][mySalesID1],salesWaitingCV[departmentOfChoice][mySalesID1]);
		if(GetMV(Customer[customerIndex].privilegeStatus)==1)
			Print3("\nPrivileged Customer [%d] is interacting with DepartmentSalesman[%d] of Department[%d]",customerIndex,mySalesID1,departmentOfChoice);
		else
				Print3("\nCustomer [%d] is interacting with DepartmentSalesman[%d] of Department[%d]",customerIndex,mySalesID1,departmentOfChoice);
		SignalCV(salesmanLock[departmentOfChoice][mySalesID1],salesWaitingCV[departmentOfChoice][mySalesID1]);
		
		ReleaseLock(salesmanLock[departmentOfChoice][mySalesID1]);
		
		/* Buying the item */
		AcquireLock(itemLock[itemToBuy]);
		while(1){
			mValue1 = GetMV(quantityOnShelf[itemToBuy]);
			if(mValue1 >=GetMV(Customer[customerIndex].itemQuantity[item])){
				SetMV(quantityOnShelf[itemToBuy],mValue1-Customer[customerIndex].itemQuantity[item]);
				if(GetMV(Customer[customerIndex].privilegeStatus)==1)
					Print3("\nPrivileged Customer [%d] has found item%d and placed [%d] in the trolly",customerIndex,itemToBuy,GetMV(Customer[customerIndex].itemQuantity[item]));
				else
					Print3("\nPrivileged Customer [%d] has found item%d and placed [%d] in the trolly",customerIndex,itemToBuy,GetMV(Customer[customerIndex].itemQuantity[item]));
				mValue2 = Customer[customerIndex].itemBoughtCount;
				SetMV(Customer[customerIndex].itemBoughtCount,++mValue2);
				ReleaseLock(itemLock[itemToBuy]);
				break;
			}
			else{ /* Item is not available */
				AcquireLock(waitingLock[departmentOfChoice]);
				ReleaseLock(itemLock[itemToBuy]);
				salesFlag=-1;
				for(i=0;i<salesmanCount;i++){
					
					if(GetMV(salesStatus[departmentOfChoice][i])==0){
						mySalesID2=i;
						AcquireLock(salesmanLock[departmentOfChoice][mySalesID2]);
						SetMV(salesStatus[departmentOfChoice][mySalesID2],1);
						salesFlag=1;
						break;
					}
				}
				if(salesFlag==-1){
					if(GetMV(Customer[customerIndex].privilegeStatus)==1)
						Print2("\nPrivileged Customer [%d] is not able to find item%d and is searching for DepartmentSalesman",customerIndex,itemToBuy);
					else
						Print2("\nCustomer [%d] is not able to find item%d and is searching for DepartmentSalesman",customerIndex,itemToBuy);
						mValue2 = GetMV(complainingCustWaitingLineCount[departmentOfChoice]);
					SetMV(complainingCustWaitingLineCount[departmentOfChoice],++mValue2);
					WaitCV(waitingLock[departmentOfChoice],complainingCustWaitingCV[departmentOfChoice]);
				}
				else{
					SignalCV(salesmanLock[departmentOfChoice][mySalesID2],salesWaitingCV[departmentOfChoice][mySalesID2]);
					mValue2 = GetMV(complainingCustWaitingLineCount[departmentOfChoice]);
					SetMV(complainingCustWaitingLineCount[departmentOfChoice],++mValue2);
					ReleaseLock(salesmanLock[departmentOfChoice][mySalesID2]);
					WaitCV(waitingLock[departmentOfChoice],complainingCustWaitingCV[departmentOfChoice]);
				}
				for(i=0;i<salesmanCount;i++){
					AcquireLock(salesmanLock[departmentOfChoice][i]);
					if(GetMV(salesStatus[departmentOfChoice][i])==4){
						mySalesID2=i;
						SetMV(salesStatus[departmentOfChoice][mySalesID2],1);
						break;
					}
					ReleaseLock(salesmanLock[departmentOfChoice][i]);
				}
				
				if(GetMV(Customer[customerIndex].privilegeStatus)==1)
					Print2("\nPrivileged Customer [%d] is asking assistance from DepartmentSalesman [%d]",customerIndex,mySalesID2);
				else
					Print2("\nCustomer [%d] is asking assistance from DepartmentSalesman [%d]",customerIndex,mySalesID2);
				SetMV(complaintCustNumber[departmentOfChoice][mySalesID2],customerIndex);
				SetMV(itemToBeRestocked_FromCustToSalesman[customerIndex],itemToBuy);
				ReleaseLock(waitingLock[departmentOfChoice]);
				/* Finished complaining to the salesman about the item */
				SignalCV(salesmanLock[departmentOfChoice][mySalesID2],salesWaitingCV[departmentOfChoice][mySalesID2]);
				if(GetMV(Customer[customerIndex].privilegeStatus)==1)
					Print3("\nPrivileged Customer [%d] has received assistance about restocking of item%d from DepartmentSalesman [%d]",customerIndex,itemToBuy,mySalesID2);
				else
					Print3("\nCustomer [%d] has received assistance about restocking of item%d from DepartmentSalesman [%d]",customerIndex,itemToBuy,mySalesID2);
				AcquireLock(itemLock[itemToBuy]);				
				ReleaseLock(salesmanLock[departmentOfChoice][mySalesID2]);
				mValue1 = GetMV(itemWaitingLinecount[itemToBuy]);
				SetMV(itemWaitingLinecount[itemToBuy],++mValue1);
				WaitCV(itemLock[itemToBuy],itemWaitingCV[itemToBuy]);
			}
		}/* while */		
	}/* Item for loop */
		
	/*-------------------------------------Customer cashier Interaction-----------------------------------------------*/
	AcquireLock(totalLineCountLock);
	if(GetMV(Customer[customerIndex].privilegeStatus) == 1)
		Print1("\nPrivileged Customer [%d] has finished shopping and is looking for the Cashier",customerIndex);
	else
		Print1("\nCustomer [%d] has finished shopping and is looking for the Cashier",customerIndex);
	while(1){
		for(j=0;j<cashierCount;j++)
		{
			if(GetMV(cashierStatus[j]) == 0)
			{
				mycashiersId = j;
				SetMV(cashierStatus[j],1);
				break;
			}
		}
		if(mycashiersId == -1)  /* No cashier is free, gets in the shortestline where the cashier is not on break */
		{
			shortestLineLength = tempShortestValue;
			shortestLineIndex = -1;
			if(GetMV(Customer[customerIndex].privilegeStatus)==0)
				j=0;
			else	
				j=1;
				
			for(;j<cashierCount*2;)
			{
				mValue1 = GetMV(cashCustLineCount[j]);
				if(shortestLineLength>mValue1 && GetMV(cashierStatus[j/2])!=2)
				{
					shortestLineLength = mValue1;
					shortestLineIndex = j;
					
				}
				j+=2;
			}
			
			mycashiersId = shortestLineIndex;
			mValue2 = GetMV(cashCustLineCount[mycashiersId]);
			SetMV(cashCustLineCount[mycashiersId],++mValue2);
			
			if(GetMV(Customer[customerIndex].privilegeStatus) == 1)
				Print3("\nPrivileged Customer [%d] chose Cashier [%d] with line of length [%d]",customerIndex,mycashiersId/2,shortestLineLength);
			else
				Print3("\nCustomer [%d] chose Cashier [%d] with line of length [%d]",customerIndex,mycashiersId/2,shortestLineLength);
				
			WaitCV(totalLineCountLock,cashCustLineCV[mycashiersId]);
			mycashiersId/=2;
			Print3("\nCustomer [%d] was signalled by Cashier [%d] with line of length [%d]",customerIndex,mycashiersId/2,shortestLineLength);
			if(GetMV(cashierStatus[mycashiersId])!=2){
				break;
			}else{
				mycashiersId=-1;				
			}
		}
		else{
			if(GetMV(Customer[customerIndex].privilegeStatus) == 1)
			{
				Print3("\nPrivileged Customer [%d] chose Cashier [%d] with line of length [%d]",customerIndex,mycashiersId,shortestLineLength);
				break;
			}
			else
			{
				Print3("\nCustomer [%d] chose Cashier [%d] with line of length [%d]",customerIndex,mycashiersId,shortestLineLength);
				break;
			}
		}
	}
	ReleaseLock(totalLineCountLock);	
	AcquireLock(cashCustIntrLock[mycashiersId]);
	SetMV(myCashCustomerid[mycashiersId],customerIndex);
	SignalCV(cashCustIntrLock[mycashiersId],cashCustIntrCV[mycashiersId]);
	WaitCV(cashCustIntrLock[mycashiersId],cashCustIntrCV[mycashiersId]);
	
	/* Interacted with cashier and go the info about the money status */
	if(GetMV(customerMoneyStatus[customerIndex])==1){
		if(GetMV(Customer[customerIndex].privilegeStatus) == 1) /* Money is not enough */
			Print2("\nPrivileged Customer [%d] cannot pay $[%d]",customerIndex,GetMV(Customer[customerIndex].totalPurchasedCost));
		else
			Print2("\nCustomer [%d] cannot pay $[%d]",customerIndex,GetMV(Customer[customerIndex].totalPurchasedCost));
			
		AcquireLock(customerManagerLineLock);
		ReleaseLock(cashCustIntrLock[mycashiersId]);
		mValue2 = GetMV(customerManagerLineCount);
		SetMV(customerManagerLineCount,++mValue2);
		
		if(GetMV(Customer[customerIndex].privilegeStatus) == 1)
			Print1("\nPrivileged Customer [%d] is waiting for Manager for negotiations.",customerIndex);
		else
			Print1("\nCustomer [%d] is waiting for Manager for negotiations.",customerIndex);
		
		/* Waits for the manager */
		WaitCV(customerManagerLineLock,customerManagerLineCV);		
		AcquireLock(customerManagerIntrLock);
		ReleaseLock(customerManagerLineLock);
		SetMV(managerCustomerId,customerIndex);
		SignalCV(customerManagerIntrLock,customerManagerIntrCV);
		WaitCV(customerManagerIntrLock,customerManagerIntrCV);
		
		/* Paid the money after negotiations */
		if(GetMV(Customer[customerIndex].privilegeStatus) == 1)
			Print2("\nPrivileged Customer [%d] pays $[%d] to Manager after removing items and is waiting for receipt from Manager. ",customerIndex,GetMV(Customer[customerIndex].totalPurchasedCost));
		else
			Print2("\nCustomer [%d] pays $[%d] to Manager after removing items and is waiting for receipt from Manager. ",customerIndex,GetMV(Customer[customerIndex].totalPurchasedCost));
		
		SignalCV(customerManagerIntrLock,customerManagerIntrCV);
		WaitCV(customerManagerIntrLock,customerManagerIntrCV);
		
		/* Received the receipt from manager */
		if(GetMV(Customer[customerIndex].privilegeStatus) == 1)
			Print1("\nPrivileged Customer [%d] got receipt from Manager and is now leaving.",customerIndex);
		else
			Print1("\nCustomer [%d] got receipt from Manager and is now leaving.",customerIndex);
			
		ReleaseLock(customerManagerIntrLock);	
	}
	else{
		if(GetMV(Customer[customerIndex].privilegeStatus) == 1) /* Money is enough */
			Print3("\nPrivileged Customer [%d] pays $[%d] to Cashier [%d] and is now waiting for receipt",customerIndex,GetMV(Customer[customerIndex].totalPurchasedCost),mycashiersId);
		else
			Print3("\nCustomer [%d] pays $[%d] to Cashier [%d] and is now waiting for receipt",customerIndex,GetMV(Customer[customerIndex].totalPurchasedCost),mycashiersId);
		ReleaseLock(cashCustIntrLock[mycashiersId]);
	}
	
	AcquireLock(trolleyEndLock);
	mValue2 = GetMV(trolleyEndCount);
	SetMV(trolleyEndCount,++mValue2);
	if(GetMV(Customer[customerIndex].privilegeStatus)==1)
		Print1("\nPrivileged Customer [%d] is leaving the trolley",customerIndex);
	ReleaseLock(trolleyEndLock);
	mValue2 = GetMV(custCountTemp);
	SetMV(custCountTemp,--mValue2);
	Print1("\nTemporary Customer Number : %d",mValue2);
	if(GetMV(Customer[customerIndex].privilegeStatus)==1)
		Print1("\nPrivileged Customer [%d] exits the store",customerIndex);
	else
		Print1("\nCustomer [%d] exits the store",customerIndex);
	Exit(0);
}
