/* 
 * exittest.c
 *
 *	Simple program to test exit system call.
 */

#include "syscall.h"

int
main()
{
	int result1 = 1000;
	result1 = Exec("../test/joinee",0,0,0);
	Join(result1);
    Exit(123);
}
