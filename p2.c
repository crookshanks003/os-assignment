#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

long long int sum = 0;
pthread_mutex_t lock;
int *numbers;
int size;

int main(int argc, char *argv[]) {
	key_t key = ftok("numbers", 65);
	int shmid = shmget(key, sizeof(int) * 1000, 0666 | IPC_CREAT);

	numbers = (int *)shmat(shmid, 0, 0);
	//first member of array is size
	size = numbers[0];

	for (int i = 1; i < size; i++) {
		printf("%d ", numbers[i]);
	}

	shmdt(numbers);

	shmctl(shmid, IPC_RMID, NULL);
}
