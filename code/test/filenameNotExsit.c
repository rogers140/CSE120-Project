#include "syscall.h"

int main(){
	int result = 1000;
	result = Exec("../test/tom",0,0,0);
	Exit(result);
}