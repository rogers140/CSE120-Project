#include "syscall.h"

int
main(int argc, char *argv[]){
	char buffer[80];
	int i = 0;
	while(1){
		buffer[i] = argv[0][i];
		//Write(buffer[i], 1, ConsoleOutput);
		i += 1;
		if(buffer[i-1]=='\0'){
			break;
		}
	}
	
	Exit(3);
}