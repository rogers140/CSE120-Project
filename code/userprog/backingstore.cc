#include "copyright.h"
#include "backingstore.h"
#include "system.h"
#include "machine.h"
#include "memorymanager.h"
extern MemoryManager *TheMemoryManager;
BackingStore::BackingStore(int pagingAlgorithm){
	backingLock = new Lock("backingStore");
	usedPage = 0;
	indexOfVictim = 0;
	algorithm = pagingAlgorithm;
	char *fileName = "backingstore";
	backingStoreFile = fileSystem->Open(fileName);
	if(backingStoreFile == NULL) {
		DEBUG('f', "No backing file! Creating...\n");
		if(fileSystem->Create(fileName, maxPageNum*PageSize)) {
			backingStoreFile = fileSystem->Open(fileName);
			if(backingStoreFile ==  NULL) {
				DEBUG('f', "Open file failed.\n");
			}
			else {
				DEBUG('f', "Open file succeeded.\n");
			}
		}
		else {
			DEBUG('f', "Create file failed.\n");
		}
	}
	else {
		DEBUG('f', "Open file succeeded.\n");
	}
	for(int i = 0; i < maxPageNum; ++i) {
		lookUpTable[i].space = NULL;
		lookUpTable[i].virtualPageNum = -1;
		lookUpTable[i].storePageNum = -1;
	}
	for(int i = 0; i < maxAddressSpaceNum; ++i) {
		addrspaceList[i] = NULL;
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
			DEBUG('c', "The page VPN: %d is dirty\n", virtualPageNum);
			int i = 0;
			for(i = 0; i < maxPageNum; ++i) { //find history
				if(lookUpTable[i].space == obeyer && lookUpTable[i].virtualPageNum == virtualPageNum) { 
					break;
				}
			}
			if(i >= maxPageNum) {//not written into the file before
				DEBUG('c', "First time writing into the backing file.\n");
				int j = 0;
				for(j = 0; j < maxPageNum; ++j) {
					if(lookUpTable[j].space ==  NULL) {//finding a empty spot
						break;
					}
				}
				if(j >= maxPageNum) {
					DEBUG('c', "Error! No enough file space!\n");
				}
				else {
					lookUpTable[j].space = obeyer;
					lookUpTable[j].virtualPageNum = virtualPageNum;
					lookUpTable[j].storePageNum = j;
					DEBUG('c', "VPN: %d is Writing into file at: %d\n",virtualPageNum, j);
					backingStoreFile->WriteAt(&(machine->mainMemory[phyPageNum * PageSize]), PageSize, (lookUpTable[j].storePageNum)*PageSize);
					(obeyer->getPageTable())[virtualPageNum].dirty = FALSE;
				}
			}
			else {//have been written into the file before.
				DEBUG('c', "Writing into file at: %d\n", lookUpTable[i].storePageNum);
				backingStoreFile->WriteAt(&(machine->mainMemory[phyPageNum * PageSize]), PageSize, (lookUpTable[i].storePageNum)*PageSize);
				(obeyer->getPageTable())[virtualPageNum].dirty = FALSE;
			}
			stats->numPageOuts += 1;
			
		}
		else {
			DEBUG('c', "The page VPN: %d is not dirty\n", virtualPageNum);
		}
		TheMemoryManager->FreePage(phyPageNum);
		(obeyer->getPageTable())[virtualPageNum].valid = FALSE; //set the page to invalid
		(obeyer->getPageTable())[virtualPageNum].physicalPage = -1; //set its physical page number to -1
	}
}
void
BackingStore::RandomPageOut(AddrSpace *demander) {//could be optimized, because it has some useless loops
	DEBUG('c', "Randomly paging out...\n");
	int victim = Random() % NumPhysPages;
	for(int i = 0; i < maxAddressSpaceNum; ++i) {
		if(addrspaceList[i] != NULL) {
			for(int j = 0; j < (int)addrspaceList[i]->getNumOfPages(); ++j) {
				if(addrspaceList[i]->getPageTable()[j].physicalPage == victim) {
					DEBUG('c',"Find a page in space %d to evit: VA: %d, PA: %d.\n", i, j, addrspaceList[i]->getPageTable()[j].physicalPage);
					PageOut(addrspaceList[i], j);
					break;
				}
			}
		}
	}
}
void
BackingStore::FIFOPageOut(AddrSpace *demander) {
	DEBUG('c', "FIFO paging out...\n");
	for(int i = 0; i < maxAddressSpaceNum; ++i) {
		if(addrspaceList[i] != NULL) {
			for(int j = 0; j < (int)addrspaceList[i]->getNumOfPages(); ++j) {
				if(addrspaceList[i]->getPageTable()[j].physicalPage == indexOfVictim) {
					DEBUG('c',"Find a page in space %d to evit: VA: %d, PA: %d.\n", i, j, addrspaceList[i]->getPageTable()[j].physicalPage);
					PageOut(addrspaceList[i], j);
					break;
				}
			}
		}
	}
	indexOfVictim = (indexOfVictim + 1) % NumPhysPages;
}

void
BackingStore::LRUClockPageOut(AddrSpace *demander) {
	DEBUG('c', "LRUClock paging out...\n");
	bool found = false;
	while(!found){
		for(int i = 0; i < maxAddressSpaceNum; ++i) {
			if(addrspaceList[i] != NULL) {
				for(int j = 0; j < (int)addrspaceList[i]->getNumOfPages(); ++j) {
					if(addrspaceList[i]->getPageTable()[j].physicalPage == indexOfVictim) {
						if(addrspaceList[i]->getPageTable()[j].use == TRUE){
							addrspaceList[i]->getPageTable()[j].use = false;
							break;							
						}else{
							DEBUG('c',"Find a page in space %d to evit: VA: %d, PA: %d.\n", i, j, addrspaceList[i]->getPageTable()[j].physicalPage);
							PageOut(addrspaceList[i], j);
							found = true;
							break;
						}
						
					}
				}
			}
		}
		indexOfVictim = (indexOfVictim + 1) % NumPhysPages;	//go to next page
	}
}


void 
BackingStore::PageIn(AddrSpace *demander, int virtualPageNum){
	backingLock->Acquire();
	stats->numPageIns += 1;
	int k = 0;
	for(k = 0; k < maxAddressSpaceNum; ++k) {
		if(demander == addrspaceList[k]) {
			DEBUG('c', "Addrespace %d is demanding page.\n", k);
		}
	}
	DEBUG('c', "Pagging in...\n");
	int phyPageNum = TheMemoryManager->AllocPage();
	if(phyPageNum == -1) {
		//no enough pages, we need to page out another page
		while(phyPageNum == -1) { //in case the pageout failed
			if(algorithm == 1) {
				FIFOPageOut(demander);
			}
			else if(algorithm == 2) {
				LRUClockPageOut(demander);
			}
			else {//default: Random
				RandomPageOut(demander);
			}
			
			phyPageNum = TheMemoryManager->AllocPage();
		}
		// RandomPageOut(demander);
		// phyPageNum = TheMemoryManager->AllocPage();
		// if(phyPageNum == -1)
		// DEBUG('c', "page in failed\n");
	}
	else {
		usedPage += 1;
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
		DEBUG('c', "Loading data from backing store file from: %d.\n",filePageNum);
		backingStoreFile->ReadAt(&(machine->mainMemory[phyPageNum * PageSize]), PageSize, filePageNum*PageSize);
	}
	else {
		//load data from executable file
		DEBUG('c', "Loading data from executable file.\n");
		demander->LoadFromExec(virtualPageNum);
	}
	(demander->getPageTable())[virtualPageNum].valid = TRUE;
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
char* 
BackingStore::getAlgorithm(){
	if(algorithm == 0) {
		return "Random Algorithm";
	}
	else if(algorithm == 1) {
		return "FIFO Algorithm";
	}
	else {
		return "LRU Algorithm";
	}
}