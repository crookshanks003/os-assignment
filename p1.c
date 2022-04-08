#include <stdio.h>
#include <stdlib.h>

//single threaded function to read file
int main(int argc, char *argv[]) {
	FILE *fp;
	int size;

	if (argc != 3) {
		printf("Provide number of numbers and filename");
		exit(-1);
	}

	size = atoi(argv[1]);
	int content [size];
	fp = fopen(argv[2], "r");

	for (int i = 0; i < size; i++){
		fscanf(fp, "%d", &content[i]);
	}

	printf("file loaded");
	return 0;
}
