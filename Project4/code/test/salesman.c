#include "syscall.h"
#include "setup.h"

int main()
{
	int i,departmentOfChoice,waitingLineCount=0,goodsFlag=-1,myGoodsID,mvValue1,mvValue2,mvValue3,mvValue4;
	int salesmanIndex;
	
	initialize();
	
	AcquireLock(salesmanIndexLock);
	Print("\nInside salesman"); 
	i=GetMV(globalSalesmanIndex);
	SetMV(globalSalesmanIndex,i+1);
	ReleaseLock(salesmanIndexLock);
	
	salesmanIndex = i%salesmanCount;
	departmentOfChoice=i/salesmanCount;
	while(1)
	{
		AcquireLock(waitingLock[departmentOfChoice]);
		AcquireLock(salesmanLock[departmentOfChoice][salesmanIndex]);
		
		mvValue1 = GetMV(custWaitingLineCount[departmentOfChoice]);
		mvValue2 = GetMV(goodsWaitingLineCount[departmentOfChoice]);
		if(mvValue1 > 0) 																	/* If Customers are waiting in greeting queue */
		{
			SetMV(salesStatus[departmentOfChoice][salesmanIndex],3);
			SetMV(custWaitingLineCount[departmentOfChoice],--mvValue1);
			SignalCV(waitingLock[departmentOfChoice],custWaitingCV[departmentOfChoice]);
			ReleaseLock(waitingLock[departmentOfChoice]);
			WaitCV(salesmanLock[departmentOfChoice][salesmanIndex],salesWaitingCV[departmentOfChoice][salesmanIndex]);
			
			mvValue1 = GetMV(salesCustNumber[departmentOfChoice][salesmanIndex]);
			if(GetMV(Customer[mvValue1].privilegeStatus)==1)
				Print3("\nDepartmentSalesman [%d] welcomes Privileged Customer [%d] to Department [%d]",salesmanIndex,mvValue1,departmentOfChoice);
			else
				Print3("\nDepartmentSalesman [%d] welcomes Customer [%d] to Department [%d]",salesmanIndex,mvValue1,departmentOfChoice);
			SignalCV(salesmanLock[departmentOfChoice][salesmanIndex],salesWaitingCV[departmentOfChoice][salesmanIndex]);
		}
		else if(mvValue2 > 0){          						 							/* If Goodsloaders are waiting in restocking queue */
			SetMV(salesStatus[departmentOfChoice][salesmanIndex],5);
			SetMV(goodsWaitingLineCount[departmentOfChoice],--mvValue2);
			SignalCV(waitingLock[departmentOfChoice],goodsWaitingCV[departmentOfChoice]);
			ReleaseLock(waitingLock[departmentOfChoice]);
			WaitCV(salesmanLock[departmentOfChoice][salesmanIndex],salesWaitingCV[departmentOfChoice][salesmanIndex]);
																								/*Item number restocked is placed in a global variable*/
			mvValue1 = GetMV(restockingGoodsloader[departmentOfChoice][salesmanIndex]);
			mvValue2 = GetMV(restockedItem[departmentOfChoice][salesmanIndex]);
			
			Print3("\nDepartmentSalesman [%d] is informed by the Goodsloader [%d] that item%d is restocked",departmentOfChoice,mvValue1,mvValue2);
			SignalCV(salesmanLock[departmentOfChoice][salesmanIndex],salesWaitingCV[departmentOfChoice][salesmanIndex]);
			mvValue2 = GetMV(restockedItem[departmentOfChoice][salesmanIndex]);
			AcquireLock(itemLock[mvValue2]);
			SetMV(itemWaitingLinecount[mvValue2],0);
			SetMV(itemRestockingFlag[mvValue2],0);
			BroadcastCV(itemLock[mvValue2],itemWaitingCV[mvValue2]);
			Print2("\nDepartmentSalesman [%d] informs the waiting customers that item%d is restocked",salesmanIndex,mvValue2);
			ReleaseLock(itemLock[mvValue2]);
		}
		else if(GetMV(complainingCustWaitingLineCount[departmentOfChoice]) > 0){						/* If Customers are waiting in complaining queue */
			AcquireLock(salesGoodsLock);
			myGoodsID = -1;	
			for(i=0;i<goodsloaderCount;i++){
				if(GetMV(goodsStatus[i])==0){
					myGoodsID=i;
					SetMV(goodsStatus[i],2);
					break;
				}
			}
			ReleaseLock(salesGoodsLock);
			if(myGoodsID!=-1){ 																		/* Signal the customers line only if goodsloader is available to restock */
				SetMV(salesStatus[departmentOfChoice][salesmanIndex],4);
				mvValue1 = GetMV(complainingCustWaitingLineCount[departmentOfChoice]);
				SetMV(complainingCustWaitingLineCount[departmentOfChoice],--mvValue1);
				SignalCV(waitingLock[departmentOfChoice],complainingCustWaitingCV[departmentOfChoice]);
				ReleaseLock(waitingLock[departmentOfChoice]);
				WaitCV(salesmanLock[departmentOfChoice][salesmanIndex],salesWaitingCV[departmentOfChoice][salesmanIndex]);
				mvValue1 = GetMV(complaintCustNumber[departmentOfChoice][salesmanIndex]);
				mvValue2 = GetMV(itemToBeRestocked_FromCustToSalesman[mvValue1]);
				if(GetMV(Customer[mvValue1].privilegeStatus)==1)
					Print3("\nDepartmentSalesman [%d] is informed by Privileged Customer [%d] that item%d is out of stock",salesmanIndex,mvValue1,mvValue2);
				else
					Print3("\nDepartmentSalesman [%d] is informed by Customer [%d] that item%d is out of stock",salesmanIndex,mvValue1,mvValue2);
				
				
				AcquireLock(itemLock[mvValue2]);
				if(GetMV(itemRestockingFlag[mvValue2])==0){
					AcquireLock(goodsloaderLock[myGoodsID]);
					Print3("DepartmentSalesman [%d] informs the Goodsloader [%d] that item%d is out of stock",salesmanIndex,myGoodsID,mvValue2);
					SignalCV(goodsloaderLock[myGoodsID],goodsloaderCV[myGoodsID]);
					WaitCV(goodsloaderLock[myGoodsID],goodsloaderCV[myGoodsID]);
					mvValue1 = GetMV(complaintCustNumber[departmentOfChoice][salesmanIndex]);
					mvValue2 = GetMV(itemToBeRestocked_FromCustToSalesman[mvValue1]);
					SetMV(itemToBeRestocked_FromSalesmanToGoodsloader[myGoodsID],mvValue2);
					SetMV(salesGoodsId[myGoodsID],salesmanIndex);
					SignalCV(goodsloaderLock[myGoodsID],goodsloaderCV[myGoodsID]);
					ReleaseLock(goodsloaderLock[myGoodsID]);
					SetMV(itemRestockingFlag[mvValue2],1);
				}
				ReleaseLock(itemLock[mvValue2]);
			}
			else{ /* No goodsloader is available to restock */
				
				SetMV(salesStatus[departmentOfChoice][salesmanIndex],1);
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
			SetMV(salesStatus[departmentOfChoice][salesmanIndex],0);
			ReleaseLock(waitingLock[departmentOfChoice]);
		}
		WaitCV(salesmanLock[departmentOfChoice][salesmanIndex],salesWaitingCV[departmentOfChoice][salesmanIndex]);
		ReleaseLock(salesmanLock[departmentOfChoice][salesmanIndex]);
	}
	Exit(0);
}