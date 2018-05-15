
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <termios.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h> 
#include <semaphore.h> 
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "memory.h"


struct Memory  *ShmPTR;
int PID = 0;
int worker_ID = 0;

void *run_writer(void *args){

    sem_t *sem = args;
	time_t t;
	int i = 0;
	int id = worker_ID;
	worker_ID++;
	printf("Worker %d ready\n", id );
	
	while(1){
		sem_wait(sem);
		if (ShmPTR -> status == -1) break;
		printf("Worker %d writing...\n", id);		
		t = time(NULL);		
		ShmPTR -> status = 1; 
		ShmPTR -> pid[i] = PID;
		PID++;
		ShmPTR -> date_time[i] = t;
		sleep(1);
		i++;		
		if ( i == ShmPTR -> limit) ShmPTR -> status = -1; 
	
		sem_post(sem);
		pthread_yield();
	}

	return NULL;

};

int main() {


	int num_writers = 4;
	key_t          ShmKEY;
	int            ShmID;
	pthread_t writers[num_writers];	
	sem_t *sem = sem_open(SEM_NAME, O_RDWR);	

	ShmKEY = ftok(".", 'x');
	ShmID = shmget(ShmKEY, sizeof(struct Memory), 0666);
	if (ShmID < 0) {
		printf("*** shmget error (client) ***\n");
		exit(1);
	}
	printf("Writers: Shared memory allocated\n");

	ShmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
	if ((int) ShmPTR == -1) {
		printf("*** shmat error (client) ***\n");
		exit(1);
	}
	printf("Writers: Shared memory attached\n");

	int t;
	
	for(t = 0; t < num_writers; t++){
		int rc = pthread_create(&writers[t], NULL, run_writer, sem);
	}
	
	for (t = 0; t < num_writers; t++) {
        pthread_join(writers[t], NULL);
    }	

	shmdt((void *) ShmPTR);
	
    printf("Writers: memory deattached\n");
    printf("Closing writers\n");
	return 0;
 

}
