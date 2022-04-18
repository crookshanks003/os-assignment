#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>

struct args {
	int index;
	long int start;
	long int end;
	long long int bytes_this_thread;
	int size;
	char *file_name;
};

const int MAX_THREADS = 10;
int tot_num_read = 0;
int *numbers; // store numbers read
long ind = 1; // store the current index of number
long int file_size;
int bytes_per_thread;

pthread_mutex_t lock;

void *th_func(void *args) {
	struct args *a;
	a = (struct args *)args;
	int arr_size = a->size / MAX_THREADS + MAX_THREADS;
	int content[arr_size];
	FILE *fp;

	fp = fopen(a->file_name, "r");
	fseek(fp, a->start, SEEK_SET);

	if (a->index == 0) {
	} else {
		while (fgetc(fp) != ' ')
			;
	}

	int num_read = 0;

	for (int i = 0; i < arr_size; i++) {
		fscanf(fp, "%d", &content[i]);
		num_read++;

		if (ftell(fp) >= a->end || ftell(fp) == file_size) {
			break;
		}
	}

	pthread_mutex_lock(&lock);
	for (int j = 0; j < num_read; j++) {
		numbers[ind] = content[j];
		ind++;
	}
	tot_num_read += num_read;
	pthread_mutex_unlock(&lock);

	return 0;
}

int main(int argc, char *argv[]) {
	clock_t t;
	t = clock();
	FILE *fp;
	long long int size;
	pthread_t threads[MAX_THREADS];

	if (argc != 3) {
		printf("Provide number of numbers and filename");
		exit(-1);
	}

	pthread_mutex_init(&lock, NULL);

	size = atoi(argv[1]);
	char *file_name = argv[2];
	struct args args_array[MAX_THREADS];

	// put the numbers array into shared memory
	key_t key = ftok("p1.c", 65);
	int shmid = shmget(key, sizeof(int) * size, 0666 | IPC_CREAT);
	numbers = (int *)shmat(shmid, 0, 0);
	// first member of array is size of numbers
	numbers[0] = size;

	fp = fopen(file_name, "r");
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	bytes_per_thread = file_size / MAX_THREADS;
	int rem = file_size % MAX_THREADS;

	long int curr_fp = 0;
	int total_bytes = 0;
	for (int i = 0; i < MAX_THREADS; i++) {
		struct args a;
		a.index = i;
		a.start = curr_fp;
		a.bytes_this_thread = bytes_per_thread + (i < rem);
		a.end = curr_fp + a.bytes_this_thread;
		a.file_name = file_name;
		a.size = size;

		total_bytes += a.bytes_this_thread;
		curr_fp += a.bytes_this_thread;
		args_array[i] = a;
	}

	for (int i = 0; i < MAX_THREADS; i++) {
		pthread_create(&threads[i], NULL, th_func, &args_array[i]);
	}

	for (int i = 0; i < MAX_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	printf("\n");
	shmdt(numbers);
	pthread_mutex_destroy(&lock);

	printf("File loaded into memory\n");
	t = clock() - t;
	double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds

	printf("%f seconds to execute \n", time_taken);

	return 0;
}
