#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>

int main(int argc, char** argv){

	int i = 0;
	char l;
	while(1){
			i++;
			printf("%d\n", i);
			sleep(2);
	}

	return 0;
}