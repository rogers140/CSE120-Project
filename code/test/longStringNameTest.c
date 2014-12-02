#include "syscall.h"

int main(){
	int result = 1000;
	result = Exec("../test/supersupersupersupersupersupersupersupersupersupersupersupersupersupersupersupersupersupersupersuperlong",0,0,0);
	Exit(result);
}