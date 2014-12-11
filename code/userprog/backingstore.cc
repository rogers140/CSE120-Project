#include "copyright.h"
#include "backingstore.h"
#include "system.h"
#include "machine.h"
#include "memorymanager.h"
extern MemoryManager *TheMemoryManager;
BackingStore::BackingStore(){
	backingLock = new Lock("backingStore");
	usedPage = 0;
	char *fileName = "backingstore";
	backingStoreFile = fileSystem->Open(fileName);
	for(int i = 0; i < maxPageNum; ++i) {
		lookUpTable[i].space = NULL;
		lookUpTable[i].virtualPageNum = -1;
		lookUpTable[i].storePageNum = -1;
	}
	for(int i = 0; i < maxAddressSpaceNum; ++i) {
		addrspaceList[i] = NULL;
	}
	if(backingStoreFile == NULL) {
		if(fileSystem->Create(fileName, maxPageNum*PageSize)) {
			backingStoreFile = fileSystem->Open(fileName);
		}
		else {

		}
	}
	
}

BackingStore::~BackingStore(){

}

void 
BackingStore::PageOut(AddrSpace *obeyer, int virtualPageNum){
	int phyPageNum = obeyer->getPageTable()[virtualPageNum].physicalPage;
	//DEBUG('c',"Evit Physical Page: %d.\n", phyPageNum);
	bool dirty = (obeyer->getPageTable())[virtualPageNum].dirty;
	if(phyPageNum != -1) {
		if(dirty) { //needs to be write to backingstore
			int i = 0;
			for(i = 0; i < maxPageNum; ++i) {
				if(lookUpTable[i].space == NULL) { 
					break;
				}
			}
			ASSERT(i < 100); //not run out of file size
			backingStoreFile->WriteAt(&(machine->mainMemory[phyPageNum]), PageSize, i*PageSize);
			(obeyer->getPageTable())[virtualPageNum].dirty = FALSE;
		}
		TheMemoryManager->FreePage(phyPageNum);
		(obeyer->getPageTable())[virtualPageNum].valid = FALSE; //set the page to invalid
		(obeyer->getPageTable())[virtualPageNum].physicalPage = -1; //set its physical page number to -1
	}
}
void
BackingStore::PageOut(AddrSpace *demander) {
	//backingLock->Acquire();
	DEBUG('c', "Pagging out...\n");
	bool found = false;
	int addressCount = 0;
	for(int i = 0; i < maxAddressSpaceNum; ++i) {
		if(addrspaceList[i] != NULL) {
			addressCount += 1;
			if(addrspaceList[i] == demander) { // is detecting current thread
				int numOfValidPage = 0;
				for(int j = 0; j < (int)addrspaceList[i]->getNumOfPages(); ++j) {
					if((addrspaceList[i]->getPageTable())[j].valid) {
						numOfValidPage += 1;
					}
				}
				if(numOfValidPage <=1 ){
					DEBUG('c', "numOfValidPage is less than 2.\n");
					continue; //if we choose to page out the page from current address space who
								//has only one valid page, just change to another one.
				}
				else {
					for(int j = 0; j < (int)addrspaceList[i]->getNumOfPages(); ++j) {
						if((addrspaceList[i]->getPageTable())[j].valid) {//if it's valid so it must hold a page
							found = true;
							DEBUG('c',"Find a page in space %d to evit: VA: %d, PA: %d.\n", i, j, addrspaceList[i]->getPageTable()[j].physicalPage);
							PageOut(addrspaceList[i], j);
							break;
						}
					}
					if(found) break;
				}
				
			}
			else {
				//DEBUG('c', "The %d address space will evit a page.\n", i);
				for(int j = 0; j < (int)addrspaceList[i]->getNumOfPages(); ++j) {
					if((addrspaceList[i]->getPageTable())[j].valid) {//if it's valid so it must hold a page
						found = true;
						DEBUG('c',"Find a page in space %d to evit: VA: %d, PA: %d.\n", i, j, addrspaceList[i]->getPageTable()[j].physicalPage);
						PageOut(addrspaceList[i], j);
						break;
					}
				}
				if(found) break;
			}
		}
	}
	if(!found) {
		DEBUG('c', "Error! Can not find a page to evit.\n");
	}
	DEBUG('c', "Number of addressspace is: %d\n", addressCount);
	//backingLock->Release();
}
void 
BackingStore::PageIn(AddrSpace *demander, int virtualPageNum){
	backingLock->Acquire();
	DEBUG('c', "Pagging in...\n");
	int k = 0;
	for(k = 0; k < maxAddressSpaceNum; ++k) {
		if(demander == addrspaceList[k]) {
			DEBUG('c', "Addrespace %d is demanding page.\n", k);
		}
	}
	int phyPageNum = TheMemoryManager->AllocPage();
	if(phyPageNum == -1) {
		//no enough pages, we need to page out another page
		ASSERT(usedPage < maxPageNum);
		PageOut(demander);
		phyPageNum = TheMemoryManager->AllocPage();

	}
	DEBUG('c', "allocate Phy: %d to Virtual: %d\n", phyPageNum, virtualPageNum);
	(demander->getPageTable())[virtualPageNum].physicalPage = phyPageNum;
	int i = 0;
	for(i = 0; i < maxPageNum; ++i) { //check if it's in the backing store file
		if(lookUpTable[i].space == demander && lookUpTable[i].virtualPageNum == virtualPageNum) { // have stored in the backingStoreFile before
			break;
		}
	}
	if(i < maxPageNum) {
		int filePageNum = lookUpTable[i].storePageNum;
		//load data from backing store
		backingStoreFile->ReadAt(&(machine->mainMemory[phyPageNum]),PageSize, filePageNum*PageSize);
	}
	else {
		//load data from executable file
		demander->LoadFromExec(virtualPageNum);
	}
	backingLock->Release();
}
bool 
BackingStore::addAddrSpace(AddrSpace *space){
	backingLock->Acquire();
	int i = 0;
	for(i = 0; i < maxAddressSpaceNum; ++i) {
		if(addrspaceList[i] == NULL) {
			addrspaceList[i] = space;
			DEBUG('c',"Find an empty address space slot.\n");
			break;
		}
	}
	if(i >= maxAddressSpaceNum) {
		backingLock->Release();
		return false;
	}
	else {
		DEBUG('c', "Adding space to backing store list [%d]\n", i);
		backingLock->Release();
		return true;
	}
}
bool 
BackingStore::removeAddrSpace(AddrSpace *space){
	backingLock->Acquire();
	int i = 0;
	for(i = 0; i < maxAddressSpaceNum; ++i) {
		if(addrspaceList[i] == space) { //remove space from address space list
			addrspaceList[i] = NULL;
			break;
		}
	}
	if(i >= maxAddressSpaceNum) {
		DEBUG('c', "Error! Can not remove this space from backing store list.\n");
		backingLock->Release();
		return false;
	}
	else {
		for(int j = 0; j < maxPageNum; ++j) { // remove space record from look up table
			if(lookUpTable[j].space == space) {
				lookUpTable[j].space = NULL;
				lookUpTable[j].virtualPageNum = -1;
				lookUpTable[j].storePageNum = -1;
			}
		}
		DEBUG('c', "removing space off backing store list [%d]\n", i);
		backingLock->Release();
		return true;
	}
}