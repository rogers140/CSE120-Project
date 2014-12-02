#include "syscall.h"

int
main()
{
	int a = 100;
    void (*foo)(int);
    foo(a);
	Exit(a);

}


