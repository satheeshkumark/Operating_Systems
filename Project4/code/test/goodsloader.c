#include "syscall.h"
#include "setup.h"

int main(){
	
	int departmentOfChoice,goodsFlag=-1,mySalesID,i,goodsIndex,mvValue1,mvValue2,mvValue3;
	
	initialize();
	
	/* Retrieving index */
	AcquireLock(goodsIndexLock);
	Print("\nInside goodsloader"); 
	goodsIndex=GetMV(globalGoodsIndex);
	SetMV(globalGoodsIndex,goodsIndex+1);
	ReleaseLock(goodsIndexLock);
	
	while(1){
		AcquireLock(salesGoodsLock);
		AcquireLock(goodsloaderLock[goodsIndex]);
		if(GetMV(goodsStatus[goodsIndex])==2){ /* Salesman signalled to restock */
			ReleaseLock(salesGoodsLock);
			SignalCV(goodsloaderLock[goodsIndex],goodsloaderCV[goodsIndex]);
			WaitCV(goodsloaderLock[goodsIndex],goodsloaderCV[goodsIndex]);
			AcquireLock(storeroomLock);
			mvValue1 = GetMV(itemToBeRestocked_FromSalesmanToGoodsloader[goodsIndex]);
			AcquireLock(itemLock[mvValue1]);
			departmentOfChoice=mvValue1/10;
			mvValue2=GetMV(salesGoodsId[goodsIndex]);
			if(departmentOfChoice==0)
				Print3("\nGoodsloader [%d] is informed by DepartmentSalesman [%d] of department [0] to restock item%d",goodsIndex,mvValue2,mvValue1);
			else if(departmentOfChoice==1)
				Print3("\nGoodsloader [%d] is informed by DepartmentSalesman [%d] of department [1] to restock item%d",goodsIndex,mvValue2,mvValue1);
			else if(departmentOfChoice==2)
				Print3("\nGoodsloader [%d] is informed by DepartmentSalesman [%d] of department [2] to restock item%d",goodsIndex,mvValue2,mvValue1);
			else if(departmentOfChoice==3)
				Print3("\nGoodsloader [%d] is informed by DepartmentSalesman [%d] of department [3] to restock item%d",goodsIndex,mvValue2,mvValue1);
			else
				Print3("\nGoodsloader [%d] is informed by DepartmentSalesman [%d] of department [4] to restock item%d",goodsIndex,mvValue2,mvValue1);
			
			Print2("\nGoodsloader [%d] is in the StockRoom and got item%d",goodsIndex,mvValue1);
			
			
			SetMV(quantityOnShelf[mvValue1],10);
			Print3("\nGoodsloader [%d] restocked item%d in Department [%d]",goodsIndex,mvValue1,departmentOfChoice);
			Print1("\nGoodsloader [%d] leaves StockRoom",goodsIndex);
			ReleaseLock(storeroomLock);
			ReleaseLock(itemLock[mvValue1]);
			AcquireLock(waitingLock[departmentOfChoice]);
			mvValue3 = GetMV(goodsWaitingLineCount[departmentOfChoice]);
			SetMV(goodsWaitingLineCount[departmentOfChoice],++mvValue3);
			mvValue2=GetMV(salesGoodsId[goodsIndex]);
			AcquireLock(salesmanLock[departmentOfChoice][mvValue2]);
			SignalCV(salesmanLock[departmentOfChoice][mvValue2],salesWaitingCV[departmentOfChoice][mvValue2]);
			ReleaseLock(salesmanLock[departmentOfChoice][mvValue2]);
			WaitCV(waitingLock[departmentOfChoice],goodsWaitingCV[departmentOfChoice]);
			
			mySalesID=-1;
			for(i=0;i<salesmanCount;i++){
				if(GetMV(salesStatus[departmentOfChoice][i]) == 5){
					mySalesID=i;
					SetMV(salesStatus[departmentOfChoice][mySalesID],1);
					break;
				}
			}
			AcquireLock(salesmanLock[departmentOfChoice][mySalesID]);
			ReleaseLock(waitingLock[departmentOfChoice]);
			mvValue1 = GetMV(itemToBeRestocked_FromSalesmanToGoodsloader[goodsIndex]);
			SetMV(restockedItem[departmentOfChoice][mySalesID],mvValue1);
			SetMV(restockingGoodsloader[departmentOfChoice][mySalesID],goodsIndex);
			SignalCV(salesmanLock[departmentOfChoice][mySalesID],salesWaitingCV[departmentOfChoice][mySalesID]);
			WaitCV(salesmanLock[departmentOfChoice][mySalesID],salesWaitingCV[departmentOfChoice][mySalesID]);
			SignalCV(salesmanLock[departmentOfChoice][mySalesID],salesWaitingCV[departmentOfChoice][mySalesID]);
			ReleaseLock(salesmanLock[departmentOfChoice][mySalesID]);
		}
		else{
			ReleaseLock(salesGoodsLock);
		}
		AcquireLock(trolleyEndLock);		
		if(GetMV(trolleyEndCount)>0){ /* If trolley is present at the exit, replace the trolley */
			mvValue1 = GetMV(trolleyEndCount);
			SetMV(trolleyEndCount,--mvValue1);
			ReleaseLock(trolleyEndLock);
			AcquireLock(trolleyFirstLock);
			mvValue1 = GetMV(trolleyFirstCount);
			SetMV(trolleyFirstCount,++mvValue1);
			Print1("\nGoodsloader [%d] replaced the trolley",goodsIndex);
			mvValue1 = GetMV(trolleyWaitingCount);
			if(mvValue1>0){
				SetMV(trolleyWaitingCount,--mvValue1);
				SignalCV(trolleyFirstLock,trolleyWaitingCV);
			}
			ReleaseLock(trolleyFirstLock);
		}
		else{
			ReleaseLock(trolleyEndLock);
		}
		AcquireLock(salesGoodsLock);	
		SetMV(goodsStatus[goodsIndex],0);
		ReleaseLock(salesGoodsLock);
		
		Print1("\nGoodsloader [%d] is waiting for orders to restock",goodsIndex);
		WaitCV(goodsloaderLock[goodsIndex],goodsloaderCV[goodsIndex]);
		ReleaseLock(goodsloaderLock[goodsIndex]);
	}
	Exit(0);
}