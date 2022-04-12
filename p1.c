#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct args {
	int start;
	long long int bytes_per_thread;
	int size;
	char *file_name;
};

const int MAX_THREADS = 100;
int num_read_arr[100];

void *th_func(void *args) {
	struct args *a;
	a = (struct args *)args;
	int arr_size = a->size/MAX_THREADS + MAX_THREADS;
	int content[arr_size];
	FILE *fp;


	fp = fopen(a->file_name, "r");
	fseek(fp, a->start * a->bytes_per_thread, SEEK_SET);
	long long int fp_end = ftell(fp) + a->bytes_per_thread;

	if(a->start == 0){
	} else {
		while (fgetc(fp) != ' ')
			;
	}

	int num_read = 0;

	for (int i = 0; i < arr_size; i++) {
		fscanf(fp, "%d", &content[i]);
		num_read++;

		if (ftell(fp) >= fp_end || feof(fp)) {
			break;
		}
		
	}

	for (int i = 0; i < num_read; i++) {
		printf("%d ", content[i]);
	}

	num_read_arr[a->start] = num_read;

	return 0;
}

int main(int argc, char *argv[]) {
	FILE *fp;
	long long int size;
	pthread_t threads[MAX_THREADS];

	if (argc != 3) {
		printf("Provide number of numbers and filename");
		exit(-1);
	}

	size = atoi(argv[1]);
	char *file_name = argv[2];
	int th_start_factor = size / MAX_THREADS;
	struct args args_array[MAX_THREADS];

	fp = fopen(file_name, "r");
	fseek(fp, 0, SEEK_END);
	long long int file_size = ftell(fp);

	for (int i = 0; i < MAX_THREADS; i++) {
		struct args a;
		a.start = i;
		a.bytes_per_thread = file_size / MAX_THREADS + 1;
		a.file_name = file_name;
		a.size = size;

		args_array[i] = a;
	}

	for (int i = 0; i < MAX_THREADS; i++) {
		pthread_create(&threads[i], NULL, th_func, &args_array[i]);
	}

	for (int i = 0; i < MAX_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	return 0;
}
