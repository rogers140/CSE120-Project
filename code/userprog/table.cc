#include "table.h"
Table::Table(int size) {
	tableSize = size;
	array = new void*[tableSize];
	tableLock = new Lock("TableLock");
	int i = 0;
	for(i = 0; i < tableSize; ++i) {
		array[i] = NULL;
	}
	Thread* t = new Thread("dummy");
	array[0] = (void*)t; //incase return 0 as space id.
}
Table::~Table() {
	delete [] array;
	delete tableLock;
}
int 
Table::Alloc(void *object) {
	tableLock->Acquire();
	int i = 0;
	for(i = 0; i <tableSize; ++i) {
		if(array[i] == NULL) {
			array[i] = object;
			tableLock->Release();
			return i;
		}
	}
	tableLock->Release();
	return -1; // did not find any empty slot
}
void * 
Table::Get(int index) {//do we need a lock here?
	return array[index];
}

void 
Table::Release(int index) {
	tableLock->Acquire();
	array[index] = NULL;
	tableLock->Release();
}

bool 
Table::Isempty(){
	for(int i = 0; i < tableSize; ++i) {
		if(array[i] != NULL){
			return 0;
		}
		
	}

	return 1;
}

int
Table::EntryExist(void *object){
	tableLock->Acquire();
	int i = 0;
	for(i = 0; i <tableSize; ++i) {
		if(array[i] == object) {			
			tableLock->Release();
			return i; // return the index of the entry
		}
	}
	tableLock->Release();
	return -1; // entry not found

}