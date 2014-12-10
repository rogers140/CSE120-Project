#include "copyright.h"
#include "backingstore.h"
#include "system.h"
#include "machine.h"
#include "memorymanager.h"
extern MemoryManager *TheMemoryManager;
BackingStore::BackingStore(){
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
BackingStore::PageOut() {
	bool found = false;
	for(int i = 0; i < maxAddressSpaceNum; ++i) {
		if(addrspaceList[i] != NULL) {
			//DEBUG('c', "The %d address space will evit a page.\n", i);
			for(int j = 0; j < addrspaceList[i]->getNumOfPages(); ++j) {
				if((addrspaceList[i]->getPageTable())[j].valid) {//if it's valid so it must hold a page
					found = true;
					//DEBUG('c',"Find a page to evit: VA: %d, PA: %d.\n", j, addrspaceList[i]->getPageTable()[j].physicalPage);
					PageOut(addrspaceList[i], j);
					
					break;
				}
			}
			if(found) break;
		}
	}
	if(!found) {
		//DEBUG('c', "Error! Can not find a page to evit.\n");
	}
}
void 
BackingStore::PageIn(AddrSpace *demander, int virtualPageNum){
	int phyPageNum = TheMemoryManager->AllocPage();
	if(phyPageNum == -1) {
		//no enough pages, we need to page out another page
		ASSERT(usedPage < maxPageNum);
		PageOut();
		phyPageNum = TheMemoryManager->AllocPage();
		//DEBUG('c', "allocate Phy: %d to Virtual: %d\n", phyPageNum, virtualPageNum);
	}
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
}
bool 
BackingStore::addAddrSpace(AddrSpace *space){
	int i = 0;
	for(i = 0; i < maxAddressSpaceNum; ++i) {
		if(addrspaceList[i] == NULL) {
			addrspaceList[i] = space;
			DEBUG('c',"Find an empty address space slot.\n");
			break;
		}
	}
	if(i >= maxAddressSpaceNum)
		return false;
	else 
		return true;

}
bool 
BackingStore::removeAddrSpace(AddrSpace *space){
	int i = 0;
	for(i = 0; i < maxAddressSpaceNum; ++i) {
		if(addrspaceList[i] == space) {
			addrspaceList[i] = NULL;
			break;
		}
	}
	if(i >= maxAddressSpaceNum)
		return false;
	else 
		return true;
}