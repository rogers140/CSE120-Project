#include "syscall.h"

int
main()
{
	int result = 1000;
	result = Exec("../test/IllegalInstrException",0,0,0);
	Exit(result);
}
