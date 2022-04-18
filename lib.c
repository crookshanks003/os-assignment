#include "lib.h"
#include <stdio.h>

void write_status_to_mem(int process, int status) {
	key_t key = ftok("scheduler.c", 65);
	int shmid = shmget(key, sizeof(int) * 2, 0666 | IPC_CREAT);
	int *p_status = (int *)shmat(shmid, 0, 0);

	p_status[process] = status;

	shmdt(p_status);
}

int read_status_from_mem(int process) {
	key_t key = ftok("scheduler.c", 65);
	int shmid = shmget(key, sizeof(int) * 2, 0666 | IPC_CREAT);
	int *p_status = (int *)shmat(shmid, 0, 0);
	int ret = p_status[process];

	shmdt(p_status);
	return ret;
}
