#include "syscall.h"

int
main(int argc, char *argv[]){
	Write(*argv, 2, ConsoleOutput);	
	Exit(3);
}