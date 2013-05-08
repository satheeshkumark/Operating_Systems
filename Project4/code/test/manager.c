#include "syscall.h"
#include "setup.h"

int main(){

	int breakCount=0,activeCount=0,i,j,w,randomBreakCashier[5],randomNmbr,activeCashiers[5];
	int breakSettingFlag=0;
	int mvValue1,mvValue2,mvValue3;
	
	initialize();
	
	while(GetMV(custCountTemp)>=0)
	{
		AcquireLock(cashierManagerLineLock);
		if(GetMV(cashierManagerLineCount)>0){
			SetMV(cashierManagerLineCount,0);
			BroadcastCV(cashierManagerLineLock,cashierManagerLineCV);
		}
		ReleaseLock(cashierManagerLineLock);
		AcquireLock(customerManagerLineLock);
		mvValue1 = GetMV(customerManagerLineCount);
		if(mvValue1 > 0){ 										/* If Customers are waiting because of money shortage */
			SetMV(customerManagerLineCount,--mvValue1);
			SignalCV(customerManagerLineLock,customerManagerLineCV);
			AcquireLock(customerManagerIntrLock);
			ReleaseLock(customerManagerLineLock);
			WaitCV(customerManagerIntrLock,customerManagerIntrCV);
			mvValue1 = GetMV(managerCustomerId);
			w=GetMV(Customer[mvValue1].itemCount)-1;
			do{ 												/* Remove the item from the trolley */
				mvValue2 = GetMV(Customer[mvValue1].totalPurchasedCost);
				mvValue3 = GetMV(Customer[mvValue1].itemQuantity[w]);
				SetMV(Customer[mvValue1].totalPurchasedCost,mvValue2-mvValue3);
				/*Customer[managerCustomerId].totalPurchasedCost-=Customer[managerCustomerId].itemQuantity[w];*/
				SetMV(Customer[mvValue1].itemQuantity[w],0);
				if(GetMV(Customer[mvValue1].privilegeStatus) == 1)
					Print2("\nManager removes item%d from the trolly of Privileged Customer [%d].",GetMV(Customer[mvValue1].itemsOfChoice[w]),mvValue1);
				else
					Print2("\nManager removes item%d from the trolly of Customer [%d].",GetMV(Customer[mvValue1].itemsOfChoice[w]),mvValue1);
				w--;
				mvValue2 = GetMV(Customer[mvValue1].totalPurchasedCost);
				mvValue3 = GetMV(Customer[mvValue1].moneyInPocket);
			}while(mvValue2>mvValue3);
			
			SignalCV(customerManagerIntrLock,customerManagerIntrCV);
			WaitCV(customerManagerIntrLock,customerManagerIntrCV);
			
			mvValue1 = GetMV(managerSalesBackup);
			mvValue2 = GetMV(Customer[mvValue1].totalPurchasedCost);
			SetMV(managerSalesBackup,mvValue1+mvValue2);
			/*managerSalesBackup += Customer[managerCustomerId].totalPurchasedCost;*/
			mvValue1 = GetMV(totalSales);
			SetMV(totalSales,mvValue1+mvValue2);
			/*totalSales+=Customer[managerCustomerId].totalPurchasedCost;*/
			SignalCV(customerManagerIntrLock,customerManagerIntrCV);
			mvValue1 = GetMV(managerCustomerId);
			if(GetMV(Customer[mvValue1].privilegeStatus) == 1)
				Print1("\nManager gives receipt to Privileged Customer [%d].",mvValue1);
			else
				Print1("\nManager gives receipt to Customer [%d].",mvValue1);
			ReleaseLock(customerManagerIntrLock);
		}
		else{
			ReleaseLock(customerManagerLineLock);
		}
		AcquireLock(totalSalesCashierLock);
		
		for(i=0;i<cashierCount;i++){ /* Emptying cashiers' drawers */
			Print1("\nManager emptied Counter [%d] drawer.",i);
			mvValue1 = GetMV(totalSales);
			mvValue2 = GetMV(totalSalesCashier[i]);
			SetMV(totalSales,mvValue1+mvValue2);
			/*totalSales+=totalSalesCashier[i];*/
			SetMV(totalSalesCashier[i],0);
			Print1("\nManager has total sales of $[%d]",mvValue1);
		}
		
		ReleaseLock(totalSalesCashierLock);
		
		mvValue1 = GetMV(custCountTemp);
		if(mvValue1==0){ /* Every customer left the store, print the total sales at the store */
			SetMV(custCountTemp,--mvValue1);
			for(i=0;i<cashierCount;i++)
			{
				Print2("\nTotal Sales from Counter [%d] is $[%d]",i,GetMV(cashiersSalesBackup[i]));
			}
			Print1("\nTotal Sales from Manager counter is $[%d]",GetMV(managerSalesBackup));
			Print1("\nTotal Sale of the entire store is $[%d]",GetMV(totalSales));
		}
		
		
		for(i=0;i<1000;i++){
			Yield();
		}
		
		
		activeCount=0;
		j=0;
		AcquireLock(totalLineCountLock);
		for(i=0;i<cashierCount;i++){
			if(GetMV(cashierStatus[i])!=2){ /* Find the active cashiers to send anyone of them on break */
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
						if(GetMV(cashierStatus[i])==0){
							AcquireLock(cashCustIntrLock[i]);
							SignalCV(cashCustIntrLock[i],cashCustIntrCV[i]);
							ReleaseLock(cashCustIntrLock[i]);
						}
						SetMV(cashierStatus[i],2);
						breakCount++;
						Print1("\nManager sends Cashier [%d] on break.",i);
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
			if(GetMV(cashierStatus[i])!=2 && (GetMV(cashCustLineCount[i*2+1])+GetMV(cashCustLineCount[i*2]))>=3){ /* Bring back cashiers from break if any line count > 3 */
				for(j=0;j<cashierCount;j++){
					AcquireLock(cashierBreakLock[j]);
					if(GetMV(cashierStatus[j])==2 && GetMV(cashierBreakUsageFlag[j])==1){
						SetMV(cashierStatus[j],1);
						SetMV(cashierBreakUsageFlag[j],0);						
						SignalCV(cashierBreakLock[j],cashierBreakCV[j]);
						Print1("\nManager brings back Cashier [%d] from break.",j);
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
	if(GetMV(custCountTemp)<0)
		EndSimulation(1);
	Exit(0);
}