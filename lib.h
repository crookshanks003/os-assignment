#include <pthread.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

int read_status_from_mem(int process);
void write_status_to_mem(int process, int status);
