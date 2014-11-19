#include "syscall.h"

int
main()
{
	int result = 1000;
	result = Exec("../test/exittest",0,0,0);
	Exit(result);
}