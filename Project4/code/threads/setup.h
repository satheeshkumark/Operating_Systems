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
int salesmanIndexLock; 

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

int customersCount = 6;					/* Total no of customers */
int salesmanCount = 3;					/* Total no of salesmen */
int goodsloaderCount = 3;				/* Total no of goodsloader */
int cashierCount = 3;					/* Total no of cashiers */
int departmentsCount = 3;				/* Total no of departments */
int custCountTemp;						/* Temp variable to keep track of customers who exit the store */

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
int totalSales;						/* Total sales at the store */
int cashiersSalesBackup[5];				/* Cashier sales back up */
int managerSalesBackup;					/* Manager sales backup */	
int cashierBreakUsageFlag[5];
int cashCustLineCount[10];
int assigningNumber = 0;

/* Indexes for all entities */
int globalCashierIndex;
int globalCustomerIndex;
int globalSalesmanIndex;

int globalGoodsIndex;

/* Creating customer information */
struct CustomerStruct Customer[100];

void setCustomeritemToBuy(int i,int numberOfDepartments)
{
	int noOfItems = (RandomSearch()%10)+1;
	int randomUniqueFlag = 1;
	int w=0;
	int rndNumber,j,l;
	int temp = 10*numberOfDepartments;
	int assigningNumber1=0;
	int quantTemp;
	if(i<=9)
		Customer[i].itemCount = CreateMV("cstitcn",10,noOfItems,i);		
	else
		Customer[i].itemCount = CreateMV("cstitcn",10,noOfItems,i);		
	while(w < GetMV(Customer[i].itemCount))
	{
		rndNumber = (RandomSearch()%temp);
		for(j=0;j<w;j++)
		{
			if(rndNumber==GetMV(Customer[i].itemsOfChoice[j]))
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
			quantTemp = RandomSearch()%10 + 1;
			
			if(assigningNumber<=9){
				Customer[i].itemsOfChoice[w] = CreateMV("cstitmc",10,rndNumber,assigningNumber);
				Customer[i].itemQuantity[w] = CreateMV("cstitq",10,quantTemp,assigningNumber);
			}
			else if(assigningNumber<=99){
				Customer[i].itemsOfChoice[w] = CreateMV("cstitmc",10,rndNumber,assigningNumber);
				Customer[i].itemQuantity[w] = CreateMV("cstitq",10,quantTemp,assigningNumber);
			}
			else if(assigningNumber<=199){
				assigningNumber1 = assigningNumber%100;
				Customer[i].itemsOfChoice[w] = CreateMV("cstitm",10,rndNumber,assigningNumber1);
				Customer[i].itemQuantity[w] = CreateMV("cstitp",10,quantTemp,assigningNumber1);
			}
			else{
				assigningNumber1 = assigningNumber%100;
				Customer[i].itemsOfChoice[w] = CreateMV("cstite",10,rndNumber,assigningNumber1);
				Customer[i].itemQuantity[w] = CreateMV("cstitu",10,quantTemp,assigningNumber1);
			}
			assigningNumber++;
			w++;
		}
	}
	Print1("\n Assignment Number : %d",assigningNumber);
} 

 
void setCustomerPrivilege(int i)
{
	int rndNumber = RandomSearch()%2;
	
	if(i<=9)
		Customer[i].privilegeStatus = CreateMV("cststat",10,rndNumber,i);
	else
		Customer[i].privilegeStatus = CreateMV("cststat",10,rndNumber,i);
	Print1("\nCustomer privilege%d",GetMV(Customer[i].privilegeStatus));
}
 
void createCustomerInformation(int numberOfCustomers,int numberOfDepartments)
{
	int i;
	for(i=0;i<numberOfCustomers;i++)
	{
		if(i<=9){
			Customer[i].customerId = CreateMV("cxtid",10,i,i);
			Customer[i].totalPurchasedCost=CreateMV("cstpcost",10,0,i);
			Customer[i].moneyInPocket=CreateMV("cstmip",10,50,i);
			Customer[i].itemBoughtCount=CreateMV("cstbcnt",10,0,i);
		}
		else{
			Customer[i].customerId = CreateMV("cxtid",10,i,i);
			Customer[i].totalPurchasedCost=CreateMV("cstpcst",10,0,i);
			Customer[i].moneyInPocket=CreateMV("cstmip",10,50,i);
			Customer[i].itemBoughtCount=CreateMV("cstbcnt",10,0,i);
		}
		Print1("\nProcessing customer information%d",GetMV(Customer[i].customerId));
		
		setCustomerPrivilege(i);
		Print("\nDone with privileging");
		setCustomeritemToBuy(i,numberOfDepartments);
	}		
}

void initialize()
{

	int i,j,k;
	
	createCustomerInformation(customersCount,departmentsCount);
	
	
	
	for(i=0;i<departmentsCount;i++){
		waitingLock[i]=CreateLock("waitingl",9,i);		
		
		goodsWaitingCV[i]=CreateCV("gwtcv",6,i);							
		custWaitingCV[i]=CreateCV("cwtcv",6,i);							
		complainingCustWaitingCV[i] = CreateCV("ccwtcv",7,i);		
	  	
		complainingCustWaitingLineCount[i] = CreateMV("ccwcnt",7,0,i);  	
		custWaitingLineCount[i] = CreateMV("cwlcnt",7,0,i);				
		goodsWaitingLineCount[i] = CreateMV("gwcnt",6,0,i);				
		for(j=0;j<salesmanCount;j++)
		{
			k=((i+1)*10)+j;
			salesmanLock[i][j] = CreateLock("salmanl",9,k);				
			salesWaitingCV[i][j] =  CreateCV("swaitcv",9,k);				
			salesStatus[i][j] = CreateMV("sstatus",9,1,k);				
			salesCustNumber[i][j] = CreateMV("scnum",7,0,k);				
			complaintCustNumber[i][j] = CreateMV("ccnum",7,0,k);		
			restockedItem[i][j] = CreateMV("rstkt",7,0,k);
			restockingGoodsloader[i][j] = CreateMV("rgdload",9,0,k);		
		}
	}
	
	for(i=0;i<(departmentsCount*10);i++){
		if(i<=9){
			itemLock[i]=CreateLock("itemlk",7,i);
			itemWaitingCV[i]=CreateCV("iwtcv",6,i);
			itemWaitingLinecount[i] = CreateMV("iwcnt",10,0,i);
			itemRestockingFlag[i] = CreateMV("irfag",10,0,i);
			quantityOnShelf[i] = CreateMV("qnshelf",10,5,i);
		}
		else{
			itemLock[i]=CreateLock("itmlk",7,i);
			itemWaitingCV[i]=CreateCV("iwtcv",7,i);								
			itemWaitingLinecount[i] = CreateMV("iwcnt",10,0,i);  				
			itemRestockingFlag[i] = CreateMV("irfag",10,0,i);					
			quantityOnShelf[i] = CreateMV("qnshelf",10,5,i);					
		}
	}
	
	storeroomLock = CreateLock("storlk",7,0);
	salesGoodsLock = CreateLock("sgdslk",7,0);
	
	
	for(i=0;i<(cashierCount*2);i++)
	{
		cashCustLineCV[i]  = CreateCV("cshcstcv",9,i);					
		cashCustLineCount[i]  = CreateMV("cclcount",9,0,i);				
	}
	
	for(i=0;i<cashierCount;i++)
	{
		cashierBreakLock[i] = CreateLock("cshbrklk",9,i);				
		cashCustIntrLock[i] = CreateLock("cshcstlk",9,i);				
		cashCustIntrCV[i] = CreateCV("chcsircv",9,i);					
		cashierBreakCV[i] = CreateCV("cshbrkcv",9,i);					
		cashierStatus[i]=CreateMV("cstatus",10,1,i);						
		totalSalesCashier[i]=CreateMV("ttlslsch",9,0,i);				
		myCashCustomerid[i]=CreateMV("cshcstid",9,0,i);					
		cashiersSalesBackup[i] = CreateMV("chslsbk",10,0,i);				
	}
	
	cashierManagerLineLock=CreateLock("cshmgrlk",9,0);					
	customerManagerLineLock=CreateLock("cstmgrlk",9,0);					
	customerManagerIntrLock=CreateLock("cstmirlk",9,0);					
	totalSalesCashierLock=CreateLock("ttlslshl",9,0);					
	
	cashierManagerLineCV=CreateCV("cshmgrcv",9,0);						
	customerManagerLineCV=CreateCV("cstmgrcv",9,0);						
	customerManagerIntrCV=CreateCV("cstmircv",9,0);						
	
	customerIndexLock = CreateLock("custindl",9,0);						
	cashierIndexLock = CreateLock("cshidl",7,0);						
	salesmanIndexLock = CreateLock("saleindl",9,0);						
	goodsIndexLock = CreateLock("goodindl",9,0);						
	
	trolleyFirstLock=CreateLock("trlyfl",7,0);							
	trolleyEndLock=CreateLock("trlyel",7,0);	
	
	trolleyWaitingCV=CreateCV("twtcv",6,0);							
	
	trolleyFirstCount = CreateMV("trfcount",9,50,0);						
	trolleyEndCount = CreateMV("trecount",9,0,0);							
	
	trolleyWaitingCount = CreateMV("twcount",10,0,0);						
	
	for(i=0;i<goodsloaderCount;i++){
		
		goodsloaderLock[i] = CreateLock("goodldlk",9,i);				
		goodsloaderCV[i]=CreateCV("goodldcv",9,i);						
		itemToBeRestocked_FromSalesmanToGoodsloader[i]=CreateMV("itbrfstg",9,0,i); 
		goodsStatus[i]=CreateMV("gstatus",10,1,i);								
		salesGoodsId[i] = CreateMV("salegdid",9,50,i);						
	}
	for(i=0;i<customersCount;i++){
		if(i<=9){
			itemToBeRestocked_FromCustToSalesman[i] = CreateMV("itrfcts",10,0,i);
			customerMoneyStatus[i] = CreateMV("cmnysts",10,0,i);
		}
		else{	
			itemToBeRestocked_FromCustToSalesman[i] = CreateMV("itrfcts",10,0,i);
			customerMoneyStatus[i] = CreateMV("cmnysts",10,0,i);		
		}
	}
	
	customerManagerLineCount = CreateMV("cmcount",10,0,0);				
	cashierManagerLineCount = CreateMV("chmcount",9,0,0);				
	managerCustomerId = CreateMV("mgrcstid",10,0,0);						
	managerSalesBackup = CreateMV("mgrslsbc",10,0,0);					
	
	totalSales = CreateMV("ttlsales",9,0,0);							
	custCountTemp =  CreateMV("cstcount",9,customersCount,0);						
	globalCashierIndex =  CreateMV("gcshind",10,0,0);					
	globalCustomerIndex =  CreateMV("gcstind",10,0,0);					
	
	globalGoodsIndex =  CreateMV("ggoodind",9,0,0);						
	globalSalesmanIndex =  CreateMV("gsaleind",9,0,0);
	totalLineCountLock = CreateLock("ttlcntlk",9,0);
		
}
