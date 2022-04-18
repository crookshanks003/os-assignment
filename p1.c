#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "lib.h"

struct args {
	int start;
	long long int bytes_per_thread;
	int size;
	char *file_name;
};

const int MAX_THREADS = 10;
int *numbers; // store numbers read
long ind = 1; // store the current index of numbers
int can_run = 0;
int task_completed = 0;

pthread_cond_t cond;
pthread_mutex_t cond_lock;
pthread_mutex_t lock;

void *th_func(void *args) {

	pthread_mutex_lock(&cond_lock);
	if (can_run == 0) {
		printf("Process 1 is sleeping\n");
		pthread_cond_wait(&cond, &cond_lock);
	}
	pthread_mutex_unlock(&cond_lock);

	struct args *a;
	a = (struct args *)args;
	int rem = a->size % MAX_THREADS;
	int arr_size = a->size / MAX_THREADS + (int)(a->start < rem);
	int content[arr_size];
	FILE *fp;

	fp = fopen(a->file_name, "r");
	fseek(fp, a->start * a->bytes_per_thread, SEEK_SET);
	long long int fp_end = ftell(fp) + a->bytes_per_thread;

	if (a->start == 0) {
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

		pthread_mutex_lock(&cond_lock);
		if (can_run == 0) {
			printf("Process 1 is sleeping in loop\n");
			pthread_cond_wait(&cond, &cond_lock);
		}
		pthread_mutex_unlock(&cond_lock);
	}

	pthread_mutex_lock(&lock);
	for (int j = 0; j < num_read; j++) {
		numbers[ind] = content[j];
		ind++;
		pthread_mutex_lock(&cond_lock);
		if (can_run == 0) {
			printf("Process 1 is sleeping in loop\n");
			pthread_cond_wait(&cond, &cond_lock);
		}
		pthread_mutex_unlock(&cond_lock);
	}
	task_completed += 1;
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

	size = atoi(argv[1]);
	char *file_name = argv[2];

	fp = fopen(file_name, "r");
	if (!fp) {
		printf("File does not exist");
		exit(-1);
	}

	pthread_mutex_init(&cond_lock, NULL);
	pthread_cond_init(&cond, NULL);
	pthread_mutex_init(&lock, NULL);

	struct args args_array[MAX_THREADS];

	// put the numbers array into shared memory
	key_t key = ftok("p1.c", 65);
	int shmid = shmget(key, sizeof(int) * size, 0666 | IPC_CREAT);
	numbers = (int *)shmat(shmid, 0, 0);
	/* numbers = (int *)malloc(sizeof(int)*size); */
	// first member of array is size of numbers
	numbers[0] = size;

	fseek(fp, 0, SEEK_END);
	long long int file_size = ftell(fp);
	int rem = file_size % MAX_THREADS;

	for (int i = 0; i < MAX_THREADS; i++) {
		struct args a;
		a.start = i;
		a.bytes_per_thread = file_size / MAX_THREADS + (int)(i < rem);
		a.file_name = file_name;
		a.size = size;

		args_array[i] = a;
	}

	for (int i = 0; i < MAX_THREADS; i++) {
		pthread_create(&threads[i], NULL, th_func, &args_array[i]);
	}

	double wait_time = 0;
	while (1) {
		can_run = read_status_from_mem(0);
		while (can_run == 0) {
			t = clock();
			can_run = read_status_from_mem(0);
			usleep(1);
			t = clock() - t;
			double time_taken = ((double)t) / CLOCKS_PER_SEC;
			wait_time += time_taken;
		}
		pthread_mutex_lock(&cond_lock);
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&cond_lock);

		if (task_completed == MAX_THREADS) break;
	}

	for (int i = 0; i < MAX_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	for (int i = 0; i < 1000; i++){
		printf("%d ", numbers[i]);
	}

	shmdt(numbers);
	pthread_mutex_destroy(&lock);
	pthread_mutex_destroy(&cond_lock);
	pthread_cond_destroy(&cond);

	printf("File loaded into memory\n");
	t = clock() - t;
	double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds


	printf("%f seconds to execute \n", time_taken);
	printf("wait time is %f", wait_time);
	return 0;
}
