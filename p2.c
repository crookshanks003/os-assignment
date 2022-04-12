#include <bits/types.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

struct args {
	int start;
	int end;
};

const int MAX_THREAD = 100;

long long int sum = 0;
pthread_mutex_t lock;
int *numbers;
int size;

void *th_function(void *args){
	struct args *a;
	a = (struct args *)args;

	long long int local_sum = 0;
	
	for (int i = a->start; i < a->end; i++){
		local_sum += numbers[i];
	}

	pthread_mutex_lock(&lock);
	sum += local_sum;
	pthread_mutex_unlock(&lock);

	return 0;
}

int main() {
	pthread_mutex_init(&lock, NULL);

	key_t key = ftok("numbers", 65);
	int shmid = shmget(key, sizeof(int) * 1000, 0666 | IPC_CREAT);

	numbers = (int *)shmat(shmid, 0, 0);
	//first member of array is size
	size = numbers[0];

	int num_per_thread = size/MAX_THREAD;

	for (int i = 0; i < MAX_THREAD; i++){

	}

	for (int i = 0; i < MAX_THREAD; i++){

	}

	pthread_mutex_destroy(&lock);
	shmdt(numbers);

	shmctl(shmid, IPC_RMID, NULL);
}
