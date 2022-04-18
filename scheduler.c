#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

#include "lib.h"

int child[2];
int process_2 = 0;
int task2_complete = 0;
int task_complete = 0;

pthread_cond_t cond_shm2;
pthread_mutex_t mutex_shm2;

//thread for watching the task status of process 1 and 2
void *watchman_task(void *args) {

	int status, pid;
	for (int i = 0; i < 2; i++) {
		pid = wait(&status);
		if (pid == child[0]) {
			child[0] = -1;
		} else if (pid == child[1]) {
			child[1] = -1;
		}
		printf("Process %d has exited\n", pid);
	}

	printf("Task complete");
	task_complete = 1;
	return 0;
}

void *process2_task(void *args) {

	pthread_mutex_lock(&mutex_shm2);
	if (process_2 == 0) {
		printf("Process 2 is sleeping");
		pthread_cond_wait(&cond_shm2, &mutex_shm2);
	}
	pthread_mutex_unlock(&mutex_shm2);

	unsigned long int sum = 0;
	for (int i = 0; i < 500; i++) {
		sum += i;
		pthread_mutex_lock(&mutex_shm2);
		if (process_2 == 0) {
			printf("Process 2 is sleeping");
			pthread_cond_wait(&cond_shm2, &mutex_shm2);
		}
		pthread_mutex_unlock(&mutex_shm2);
		usleep(1);
	}

	task2_complete = 1;
	return 0;
}

int main() {
	pthread_t watchman;
	int tq = 1*1000; //time-quant in microseconds

	child[0] = fork();

	if (child[0] == 0) {
		char *argv[] = {"./p1.out", "1000", "./input/thousand.txt", NULL};
		execv(argv[0], argv);
	} else {
		child[1] = fork();

		if (child[1] == 0) {
			printf("Child 2\n");
			/* pthread_t task_thread; */
			/* pthread_mutex_init(&mutex_shm2, NULL); */
			/* pthread_cond_init(&cond_shm2, NULL); */
			/* pthread_create(&task_thread, NULL, process2_task, NULL); */

			/* while (task2_complete == 0) { */
			/* 	while (process_2 == 0) { */
			/* 		usleep(1); */
			/* 	} */
			/* 	pthread_mutex_lock(&mutex_shm2); */
			/* 	pthread_cond_signal(&cond_shm2); */
			/* 	pthread_mutex_unlock(&mutex_shm2); */
			/* } */
			/* pthread_join(task_thread, NULL); */
			/* pthread_mutex_destroy(&mutex_shm2); */
			/* pthread_cond_destroy(&cond_shm2); */

		} else {
			pthread_create(&watchman, NULL, watchman_task, NULL);
			printf("Scheduling...\n");
			for (long long i = 0; task_complete == 0; i++) {
				if (child[i % 2] == -1) continue;

				write_status_to_mem(0, i % 2 == 0);
				write_status_to_mem(1, i % 2 == 1);
				usleep(tq);
			}

			pthread_join(watchman, NULL);
		}
	}

	return 0;
}
