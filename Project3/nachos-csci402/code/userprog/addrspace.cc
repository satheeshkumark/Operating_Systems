// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include "table.h"
#include "synch.h"


extern "C" { int bzero(char *, int); };

Table::Table(int s) : map(s), table(0), lock(0), size(s) {
    table = new void *[size];
    lock = new Lock("TableLock");
}

Table::~Table() {
    if (table) {
	delete table;
	table = 0;
    }
    if (lock) {
	delete lock;
	lock = 0;
    }
}

void *Table::Get(int i) {
    // Return the element associated with the given if, or 0 if
    // there is none.

    return (i >=0 && i < size && map.Test(i)) ? table[i] : 0;
}

int Table::Put(void *f) {
    // Put the element in the table and return the slot it used.  Use a
    // lock so 2 files don't get the same space.
    int i;	// to find the next slot

    lock->Acquire();
    i = map.Find();
    lock->Release();
    if ( i != -1)
	table[i] = f;
    return i;
}

void *Table::Remove(int i) {
    // Remove the element associated with identifier i from the table,
    // and return it.

    void *f =0;

    if ( i >= 0 && i < size ) {
	lock->Acquire();
	if ( map.Test(i) ) {
	    map.Clear(i);
	    f = table[i];
	    table[i] = 0;
	}
	lock->Release();
    }
    return f;
}

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	"executable" is the file containing the object code to load into memory
//
//      It's possible to fail to fully construct the address space for
//      several reasons, including being unable to allocate memory,
//      and being unable to read key parts of the executable.
//      Incompletely consretucted address spaces have the member
//      constructed set to false.
//----------------------------------------------------------------------

AddrSpace::AddrSpace(char *filename) : fileTable(MaxOpenFiles) {
    NoffHeader noffH;
    unsigned int i, size,codeDataPages;
	
    // Don't allocate the input or output to disk files
    fileTable.Put(0);
    fileTable.Put(0);

	executable = fileSystem->Open(filename);
	
	if (executable == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
    }
	
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size ;
	codeDataPages=divRoundUp(size, PageSize);
    numPages = divRoundUp(size, PageSize) + divRoundUp(UserStackSize,PageSize);
                                                // we need to increase the size
						// to leave room for the stack
	//printf("\nNumpages for main thread Size : %d   StackSize : %d",divRoundUp(size, PageSize),divRoundUp(UserStackSize,PageSize));
    //size = numPages * PageSize;

    //ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);

// first, set up the translation 
    pageTable = new newTranslationEntry[numPages + 10000];
    
	int availablePhyPage,ppn,j;
	
	for (i = 0; i < numPages; i++) 
	{
		ppn=-1;
		pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
		pageTable[i].physicalPage = -1;
		pageTable[i].valid = false;
		pageTable[i].use = FALSE;
		pageTable[i].dirty = FALSE;
		pageTable[i].readOnly = FALSE;  
		pageTable[i].pageLocation = 4;
		pageTable[i].byteOffset = -1;
				
		if(i<codeDataPages){
			pageTable[i].pageLocation=1;
			pageTable[i].byteOffset=noffH.code.inFileAddr+(pageTable[i].virtualPage*PageSize);
		}				
    }
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
// 	Dealloate an address space.  release pages, page tables, files
// 	and file tables
//----------------------------------------------------------------------
AddrSpace::~AddrSpace()
{
    delete pageTable;
}

/*-------------------------------------------------------------------

HandleMemoryFull ::
	-> Evicts a page when the memory is full
	-> If the evicted page is dirty, writes it in a swap file
	-> Saves the location in the pageTable
	
---------------------------------------------------------------------*/
	
int AddrSpace::handleMemoryFull(){
	int ppn;
	int i;
	iptLock->Acquire();
	
	if(evictionOption==1){
        do{
            ppn=rand()%NumPhysPages;
            if(ipt[ppn].use==false){
                ipt[ppn].use=true;
                break;
            }
        }while(1);
    }
    else{
        while(1){
            ppn = (int)evictionQueue->Remove();							// Evicts a page when the memory is full
            if(ipt[ppn].use==false)
            {
				ipt[ppn].use=true;
                break;                
            }
            else
                evictionQueue->Append((void *)ppn);
        }
    }
	
	
	IntStatus oldLevel=interrupt->SetLevel(IntOff);
	for(i=0;i<TLBSize;i++){
		if(machine->tlb[i].valid && ppn==machine->tlb[i].physicalPage){
			ipt[ppn].dirty=machine->tlb[i].dirty;
			machine->tlb[i].valid=false;
			break;
		}
	}
	interrupt->SetLevel(oldLevel);
	
	processTable[currentThread->myProcessID].PageTableLock->Acquire();
	if(ipt[ppn].dirty){													//If the evicted page is dirty, writes it in a swap file
		swapBitmapLock->Acquire();
		int swapLocation=swapBitmap->Find();
		swapBitmapLock->Release();
		
		swapFileLock->Acquire();
		swapFile->WriteAt(&(machine->mainMemory[ppn*PageSize]),PageSize,swapLocation*PageSize);
		swapFileLock->Release();
																		//Saves the location in the pageTable
																		
		ipt[ppn].addrSpace->pageTable[ipt[ppn].virtualPage].pageLocation=2;				
		ipt[ppn].addrSpace->pageTable[ipt[ppn].virtualPage].byteOffset=swapLocation*PageSize;		
	}
	
	ipt[ppn].addrSpace->pageTable[ipt[ppn].virtualPage].physicalPage=-1;
	ipt[ppn].addrSpace->pageTable[ipt[ppn].virtualPage].valid=false;
	processTable[currentThread->myProcessID].PageTableLock->Release();
	iptLock->Release();
	return ppn;
	
}

/*-------------------------------------------------------------------

HandleIptMiss ::
	-> Find an empty page in the memory to place the required page 
	-> If there is no empty page, call handleMemoryFull which finds an empty page
	-> Read the required page either from swap file of from executable and place it in memory
	-> Update the pageTable and IPT
	
---------------------------------------------------------------------*/

int AddrSpace::handleIptMiss(int vpn){
	int ppn = -1;
	int pid=currentThread->myProcessID;
	memoryBitmapLock->Acquire();
	ppn=memoryBitmap->Find();						//Find an empty page in the memory to place the required page
	memoryBitmapLock->Release();
	
	if(ppn==-1){
	ppn=handleMemoryFull();							//If there is no empty page, call handleMemoryFull which finds an empty page
		}
	
	processTable[pid].PageTableLock->Acquire();
	
	if(pageTable[vpn].pageLocation==1)				//Read the required page either from swap file of from executable and place it in memory
	{
		executable->ReadAt(&(machine->mainMemory[ppn*PageSize]),
						PageSize,pageTable[vpn].byteOffset);	
		ipt[ppn].dirty=false;
		
	}
	else if(pageTable[vpn].pageLocation==2){
		swapFileLock->Acquire();
		swapFile->ReadAt(&(machine->mainMemory[ppn*PageSize]),
						PageSize,pageTable[vpn].byteOffset);
		swapFileLock->Release();
		ipt[ppn].dirty=true;
		
	}
	else{
	}
													//Update the pageTable and IPT
	pageTable[vpn].physicalPage = ppn;
	pageTable[vpn].valid = TRUE;
	
	processTable[pid].PageTableLock->Release();
	iptLock->Acquire();
	
	ipt[ppn].virtualPage=vpn;
	ipt[ppn].physicalPage=ppn;
	ipt[ppn].valid=TRUE;
	ipt[ppn].use=true;
	ipt[ppn].readOnly=FALSE;
	
	ipt[ppn].processID=currentThread->myProcessID;
	ipt[ppn].addrSpace=currentThread->space;
	
	evictionQueue->Append((void *)ppn);
	
	iptLock->Release();
	return ppn;

}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %x\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
//Allocates 8 pages for current thread's stack
//----------------------------------------------------------------------

int AddrSpace::allocateStack()
{
	int i,j,ppn;
	//iptLock->Acquire();
	for (i = numPages; i < numPages + 8; i++) {
		ppn=-1;
		pageTable[i].virtualPage = i;
		pageTable[i].physicalPage=-1;
		pageTable[i].valid=false;
		pageTable[i].use = FALSE;
		pageTable[i].dirty = FALSE;
		pageTable[i].readOnly = FALSE;  
		pageTable[i].pageLocation = 3;
		pageTable[i].byteOffset = -1;	
	}
	numPages = numPages+8;
	return numPages;
}

void AddrSpace::handlePageFault()
{
	int ppn=-1,i;
	int vpn=(machine->ReadRegister(39))/PageSize;
	
	iptLock->Acquire();
	for(i=0;i<NumPhysPages;i++){
		if(ipt[i].valid && vpn==ipt[i].virtualPage && ipt[i].processID==currentThread->myProcessID && !ipt[i].use){
			ppn=i;
			ipt[i].use=true;
			//printf("\ninside handle page fault vpn: %d ppn: %d",vpn,ppn);
			break;
		}
	}
	//printf("\nOutside handle page fault vpn: %d ppn: %d",vpn,ppn);
	iptLock->Release();
	
	if(ppn==-1){
		ppn=handleIptMiss(vpn);
	}
	/*printf("\nAfter handling ipt miss, inside handle page fault vpn: %d ppn: %d",vpn,ppn);
	printf("\nPPN\tVPN\tDISKLOC\tOFFSET");
	for(i=0;i<NumPhysPages;i++)
		printf("\n%d\t%d\t%d\t%d",ipt[i].physicalPage,ipt[i].virtualPage,pageTable[ipt[i].virtualPage].pageLocation,pageTable[ipt[i].virtualPage].byteOffset);
		printf("\nPID\tPPN\tVPN\tDISKLOC\tOFFSET");
	for(i=0;i<numPages;i++)
		printf("\n%d\t%d\t%d\t%d\t%d",pageTable[i].processID,pageTable[i].physicalPage,pageTable[i].virtualPage,pageTable[i].pageLocation,pageTable[i].byteOffset);
	*/
	iptLock->Acquire();
	IntStatus oldLevel=interrupt->SetLevel(IntOff);
	if(machine->tlb[currentTLB].valid)
		ipt[machine->tlb[currentTLB].physicalPage].dirty=machine->tlb[currentTLB].dirty;
	
	machine->tlb[currentTLB].virtualPage=ipt[ppn].virtualPage;
	machine->tlb[currentTLB].physicalPage=ipt[ppn].physicalPage;
	machine->tlb[currentTLB].valid=true;
	machine->tlb[currentTLB].use=ipt[ppn].use;
	machine->tlb[currentTLB].dirty=ipt[ppn].dirty;
	machine->tlb[currentTLB].readOnly=ipt[ppn].readOnly;
	
	currentTLB=(currentTLB+1)%TLBSize;
	ipt[ppn].use=false;
	iptLock->Release();
	interrupt->SetLevel(oldLevel);
	//return;
}

//Returns the current numPages since it is the private variable of Addrspace
int AddrSpace::getNumPages()
{
	return numPages;
}

//Dealloating the stack memory for the current thread
void AddrSpace::clearMemory()
{
	int i;
	
	//printf("\ninside the clear memory function");
	
	iptLock->Acquire();
	for(i=currentThread->numPages-1; i>=currentThread->numPages-8; i--){
		if(ipt[pageTable[i].physicalPage].valid==true){
			ipt[pageTable[i].physicalPage].valid = false;
			memoryBitmapLock->Acquire();
			memoryBitmap->Clear(pageTable[i].physicalPage);
			memoryBitmapLock->Release();
		}
	}
	iptLock->Release();
	
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{
	
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
	int i = 0;
    //machine->pageTable = pageTable;
    //machine->pageTableSize = numPages;
	IntStatus oldLevel=interrupt->SetLevel(IntOff);
	for(i = 0; i < TLBSize; i++){
		if(machine->tlb[i].valid == true){
			ipt[machine->tlb[i].physicalPage].dirty=machine->tlb[i].dirty;
		}
			machine->tlb[i].valid = false;
	}
	interrupt->SetLevel(oldLevel);
	
}