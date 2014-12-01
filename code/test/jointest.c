#include "syscall.h"

int main(){
	int result1 = 1000;
	result1 = Exec("../test/exittest",0,0,1);
	Exit(result1);
}