#include "syscall.h"

int main(){
	int result = 1000;
	result = Exec("Œ",0,0,0);
	Exit(result);
}