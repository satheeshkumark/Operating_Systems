#include "syscall.h"
#include "setup.h"

int main()
{
	int j,cashierIndex;
	int mValue1,mValue2,mValue3,mValue4;
	
	initialize();
	
	AcquireLock(cashierIndexLock);
	cashierIndex = GetMV(globalCashierIndex);
	SetMV(globalCashierIndex,cashierIndex+1);
	ReleaseLock(cashierIndexLock);
	
	while(1)
	{
		AcquireLock(totalLineCountLock);
		AcquireLock(cashierBreakLock[cashierIndex]);
		if(cashierStatus[cashierIndex]==2){ /* Cashier is sent on break by the manager */
			SetMV(cashierBreakUsageFlag[cashierIndex],1);
			BroadcastCV(totalLineCountLock,cashCustLineCV[cashierIndex*2+1]);
			BroadcastCV(totalLineCountLock,cashCustLineCV[cashierIndex*2]);
			SetMV(cashCustLineCount[cashierIndex*2+1],0);
			SetMV(cashCustLineCount[cashierIndex*2],0);
			ReleaseLock(totalLineCountLock);
			Print1("\nCashier [%d] is going on break.",cashierIndex);
			WaitCV(cashierBreakLock[cashierIndex],cashierBreakCV[cashierIndex]);
			Print1("\nCashier [%d] was called from break by Manager to work.",cashierIndex);
			ReleaseLock(cashierBreakLock[cashierIndex]);
			AcquireLock(totalLineCountLock);
		}
		else
		{
			ReleaseLock(cashierBreakLock[cashierIndex]);
		}
		
		mValue1 = GetMV(cashCustLineCount[cashierIndex*2+1]);
		mValue2 = GetMV(cashCustLineCount[cashierIndex*2]);
		
		/* Signals the privileged queue if there are customers*/
		if(mValue1>0)
		{
			SetMV(cashCustLineCount[cashierIndex*2+1],--mValue1);
			SetMV(cashierStatus[cashierIndex],1);
			SignalCV(totalLineCountLock,cashCustLineCV[cashierIndex*2+1]);
		}
		else if(mValue2>0) /* Signals the normal queue */
		{
			SetMV(cashCustLineCount[cashierIndex*2],--mValue2);
			SetMV(cashierStatus[cashierIndex],1);
			SignalCV(totalLineCountLock,cashCustLineCV[cashierIndex*2]);
		}
		else	
		{
			SetMV(cashierStatus[cashierIndex],0);
		}
		
		AcquireLock(cashCustIntrLock[cashierIndex]);
		ReleaseLock(totalLineCountLock);
		WaitCV(cashCustIntrLock[cashierIndex],cashCustIntrCV[cashierIndex]);
		AcquireLock(totalLineCountLock);
		if(GetMV(cashierStatus[cashierIndex])!=2){
			ReleaseLock(totalLineCountLock);
			
			mValue1 = GetMV(myCashCustomerid[cashierIndex]);
			mValue2 = GetMV(Customer[mValue1].itemCount);
			
			for(j=0; j<mValue2; j++)
			{
				if(Customer[mValue1].privilegeStatus == 1)
					Print3("\nCashier [%d] got [%d] from trolly of Privileged Customer [%d].",cashierIndex,GetMV(Customer[mValue1].itemsOfChoice[j]),mValue1);
				else
					Print3("\nCashier [%d] got [%d] from trolly of Customer [%d].",cashierIndex,GetMV(Customer[mValue1].itemsOfChoice[j]),mValue1);
				
				mValue3 = GetMV(Customer[mValue1].totalPurchasedCost);
				mValue4 = GetMV(Customer[mValue1].itemQuantity[j]);
				
				SetMV(Customer[mValue1].totalPurchasedCost,mValue3 + mValue4);
			}
			
			if(GetMV(Customer[mValue1].privilegeStatus) == 1)
				Print3("\nCashier [%d] tells Privileged Customer [%d] total cost is $[%d].",cashierIndex,mValue1,GetMV(Customer[mValue1].totalPurchasedCost));
			else
				Print3("\nCashier [%d] tells Customer [%d] total cost is $[%d].",cashierIndex,mValue1,GetMV(Customer[mValue1].totalPurchasedCost));
				
			if(GetMV(Customer[mValue1].totalPurchasedCost) > GetMV(Customer[mValue1].moneyInPocket)) /* Customer doesnt have enough money */
			{
				if(GetMV(Customer[mValue1].privilegeStatus) == 1)
					Print2("\nCashier [%d] asks Privileged Customer [%d] to wait for Manager.",cashierIndex,mValue1);
				else
					Print2("\nCashier [%d] asks Customer [%d] to wait for Manager.",cashierIndex,mValue1);
				SetMV(customerMoneyStatus[mValue1],1);
				
				AcquireLock(cashierManagerLineLock);
				mValue2 = GetMV(cashierManagerLineCount);
				SetMV(cashierManagerLineCount,++mValue2);
		
				WaitCV(cashierManagerLineLock,cashierManagerLineCV);
				if(GetMV(Customer[mValue1].privilegeStatus) == 1)
					Print2("\nCashier [%d] informs the Manager that Privileged Customer [%d] does not have enough money.",cashierIndex,mValue1);
				else
					Print2("\nCashier [%d] informs the Manager that Customer [%d] does not have enough money.",cashierIndex,mValue1);
				ReleaseLock(cashierManagerLineLock);
				
			}
			else /* Customer have enough money */
			{
				if(GetMV(Customer[mValue1].privilegeStatus) == 1)
					Print2("\nCashier [%d] gave the receipt to Privileged Customer [%d] and tells him to leave.",cashierIndex,mValue1);
				else
					Print2("\nCashier [%d] gave the receipt to Customer [%d] and tells him to leave.",cashierIndex,mValue1);
				SetMV(customerMoneyStatus[mValue1],0);
				if(GetMV(Customer[mValue1].privilegeStatus) == 1)
					Print3("\nCashier [%d] got money $[%d] from Privileged Customer [%d].",cashierIndex,GetMV(Customer[mValue1].totalPurchasedCost),mValue1);
				else
					Print3("\nCashier [%d] got money $[%d] from Customer [%d].",cashierIndex,GetMV(Customer[mValue1].totalPurchasedCost),mValue1);
				AcquireLock(totalSalesCashierLock);
				
				mValue2 = GetMV(cashiersSalesBackup[cashierIndex]);
				mValue3 = GetMV(Customer[mValue1].totalPurchasedCost);
				SetMV(cashiersSalesBackup[cashierIndex], mValue2 + mValue3);
				
				mValue2 = GetMV(totalSalesCashier[cashierIndex]);
				SetMV(totalSalesCashier[cashierIndex], mValue2 + mValue3);
				ReleaseLock(totalSalesCashierLock);
			}
			SignalCV(cashCustIntrLock[cashierIndex],cashCustIntrCV[cashierIndex]);
			ReleaseLock(cashCustIntrLock[cashierIndex]);	
		}
		else{	
			ReleaseLock(totalLineCountLock);
			ReleaseLock(cashCustIntrLock[cashierIndex]);
		}
	}
}
