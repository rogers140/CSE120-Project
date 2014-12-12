#ifndef BACKINGSTORE_H
#define BACKINGSTORE_H

#include "copyright.h"
#include "addrspace.h"
#include "filesys.h"
#include "machine.h"
#include "synch.h"

#define maxPageNum 100
#define maxAddressSpaceNum 50
struct ShadowPageEntry {
	AddrSpace *space;
	int virtualPageNum;
	int storePageNum; //the #page stored on the backing store file
};
class BackingStore{
public:
	BackingStore(int pagingAlgorithm);
	void PageOut(AddrSpace *obeyer, int virtualPageNum); //basic page out action.
	//void PageOut(AddrSpace *demander);
	void RandomPageOut(AddrSpace *demander); //random paging out algorithm
	void FIFOPageOut(AddrSpace *demander); //FIFO paging out algorithm
	void LRUClockPageOut(AddrSpace *demander); //LRU paging out algorithm
	void PageIn(AddrSpace *demander, int virtualPageNum);
	bool addAddrSpace(AddrSpace *space);
	bool removeAddrSpace(AddrSpace *space);
	char* getAlgorithm();
	~BackingStore();
private:
	OpenFile *backingStoreFile;
	ShadowPageEntry lookUpTable[maxPageNum]; // look up the stored entry
	AddrSpace *addrspaceList[maxAddressSpaceNum]; //store initialized addrspace
	Lock* backingLock; //use lock to ensure thread-safe
	int usedPage;
	int indexOfVictim; // used for FIFO
	int algorithm; //0: Random (default)
				   //1: FIFO
				   //2:	LRU

};


#endif // BACKINGSTORE_H