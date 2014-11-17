//memorymanager.c
#include "copyright.h"
#include "memorymanager.h"

//MemoryMananger::memorymanager
MemoryManager::MemoryManager(int numPages)
{
	bitMap = new BitMap(numPages);
	safeLock = new Lock("safeLock");

}
MemoryManager::~MemoryManager()
{
	delete bitMap;
	delete safeLock;
}
int MemoryManager::AllocPage()
{
	safeLock->Acquire();
	if((bitMap->NumClear) <1)
	{
		safeLock->Release();
		return -1;
	}
	int pageNum = bitMap->Find();
	bitMap->Mark(pageNum);
	safeLock->Release();
	return pageNum;
}
void MemoryManager::FreePage(int physPageNum)
{
	safeLock->Acquire();
	
	bitMap->Clear(physPageNum);

	safeLock->Release();
}
bool MemoryManager::PageIsAllocated(int physPageNum)
{
	safeLock->Acquire();
	if(bitMap->Test(physPageNum)
		safeLock->Release();
		return TRUE;
	else
		safeLock->Release();
		return FALSE;
}
}