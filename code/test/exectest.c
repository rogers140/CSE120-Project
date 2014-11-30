#include "syscall.h"

int
main()
{
	int result = 1000;
	char *argv[] = {"to", "mar", "john", "lijun", "yiqiu"};
	result = Exec("../test/argExecTest", 5, *argv, 0);
	// result = Exec("../test/exittest",0,0,0);
	// result = Exec("../test/sort",0,0,0);
	// result = Exec("../test/snake",0,0,0);
	// result = Exec("../test/array",0,0,0);
	Exit(result);
}