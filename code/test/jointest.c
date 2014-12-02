#include "syscall.h"

int main(){
	int result = 1000;
	result = Exec("../test/joinee",0,0,1);
	Join(result);
	Exit(result);
}