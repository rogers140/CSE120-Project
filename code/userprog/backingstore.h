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
	BackingStore();
	void PageOut(AddrSpace *obeyer, int virtualPageNum); //used for active yield of a page
	void PageOut(AddrSpace *demander); //used for passive yield 
	void PageIn(AddrSpace *demander, int virtualPageNum);
	bool addAddrSpace(AddrSpace *space);
	bool removeAddrSpace(AddrSpace *space);
	~BackingStore();
private:
	OpenFile *backingStoreFile;
	int usedPage; //number of pages that all process use
	ShadowPageEntry lookUpTable[maxPageNum]; // look up the stored entry
	AddrSpace *addrspaceList[maxAddressSpaceNum]; //store initialized addrspace
	Lock* backingLock; //use lock to ensure thread-safe

};


#endif // BACKINGSTORE_H