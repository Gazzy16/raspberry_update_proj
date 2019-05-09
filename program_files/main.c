#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <errno.h>

int main(int argc, char** argv){

	int i = 0;
	char l;

	printf("2v\n" );

	while(i < 100){
			i++;
			//printf("%d\n", i);
			sleep(1);
	}

	return 0;
}