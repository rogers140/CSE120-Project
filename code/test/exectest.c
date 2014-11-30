#include "syscall.h"

int
main()
{
	int result = 1000;
	char* argv[] = {"tom", "mary", "jon"};
	result = Exec("../test/argExecTest",2,argv,0);
	// result = Exec("../test/exittest",0,0,0);
	// result = Exec("../test/exittest",0,0,0);
	// result = Exec("../test/exittest",0,0,0);
	Exit(result);
}