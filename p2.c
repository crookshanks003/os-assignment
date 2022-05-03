#include <bits/types.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "lib.h"

struct args {
	int start;
	int end;
};

const int MAX_THREAD = 100;

long long int sum = 0;
int *numbers;
int size;

int can_run = 0;
int task_completed = 0;

pthread_cond_t cond;
pthread_mutex_t cond_lock;
pthread_mutex_t lock;

void *th_function(void *args) {

	pthread_mutex_lock(&cond_lock);
	if (can_run == 0) {
		pthread_cond_wait(&cond, &cond_lock);
	}
	pthread_mutex_unlock(&cond_lock);

	struct args *a;
	a = (struct args *)args;

	long int local_sum = 0;

	for (int i = a->start; i <= a->end; i++) {
		local_sum += numbers[i];
		pthread_mutex_lock(&cond_lock);
		if (can_run == 0) {
			pthread_cond_wait(&cond, &cond_lock);
		}
		pthread_mutex_unlock(&cond_lock);
	}

	// add to the sum safely
	pthread_mutex_lock(&lock);
	sum += local_sum;
	task_completed += 1;
	pthread_mutex_unlock(&lock);

	return 0;
}

int main() {
	clock_t t;
	t = clock();

	pthread_mutex_init(&cond_lock, NULL);
	pthread_cond_init(&cond, NULL);
	pthread_mutex_init(&lock, NULL);

	key_t key = ftok("p1.c", 65);
	int shmid = shmget(key, sizeof(int) * 100, 0666 | IPC_CREAT);

	numbers = (int *)shmat(shmid, 0, 0);
	// first member of array is size
	size = numbers[0];

	int rem = size % MAX_THREAD;
	int num_per_thread = size / MAX_THREAD;

	pthread_t threads[MAX_THREAD];
	struct args th_args[MAX_THREAD];

	for (int i = 0; i < MAX_THREAD; i++) {
		struct args arg;
		arg.start = i * num_per_thread + 1;
		arg.end = (i + 1) * num_per_thread + (int)(i < rem);

		th_args[i] = arg;
	}

	for (int i = 0; i < MAX_THREAD; i++) {
		pthread_create(&threads[i], NULL, th_function, &th_args[i]);
	}

	double wait_time = 0;
	while (1) {
		can_run = read_status_from_mem(1);
		while (can_run == 0) {
			t = clock();
			can_run = read_status_from_mem(1);
			usleep(1);
			t = clock() - t;
			double time_taken = ((double)t) / CLOCKS_PER_SEC;
			wait_time += time_taken;
		}
		pthread_mutex_lock(&cond_lock);
		pthread_cond_broadcast(&cond);
		pthread_mutex_unlock(&cond_lock);

		if (task_completed == MAX_THREAD) break;
	}

	for (int i = 0; i < MAX_THREAD; i++) {
		pthread_join(threads[i], NULL);
	}

	printf("sum is: %lld\n", sum);

	pthread_mutex_destroy(&lock);
	pthread_mutex_destroy(&cond_lock);
	pthread_cond_destroy(&cond);

	shmdt(numbers);
	shmctl(shmid, IPC_RMID, NULL);

	t = clock() - t;
	double time_taken = ((double)t) / CLOCKS_PER_SEC; // in seconds

	printf("wait time for 2 is %f\n", wait_time);
	printf("%f seconds to execute \n", time_taken);
}
