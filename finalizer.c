
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
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>


#include "memory.h"

void show_mem(struct Memory* ShmPTR) {
	sem_t *sem = sem_open(SEM_NAME, O_RDWR);  	
	int i = 0;	
	sem_wait(sem);
	int lim = ShmPTR -> limit;	
	for(i = 0; i < lim; i++){
		time_t t = ShmPTR -> date_time[i];

		char buff[20];
		strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
		
		printf("Line: %d | PID: %d | Date Time: ", i, ShmPTR -> pid[i]);
		printf("%s\n",buff);
		
	}
	sem_post(sem);
	sem_close(sem);
}

int main(int argc , char *argv[]){

	int ShmID;
	struct Memory *ShmPTR;
	
	key_t ShmKEY = ftok(KEY, VAL);
	ShmID = shmget(ShmKEY, sizeof(struct Memory), 0666);
	if (ShmID < 0) {
		printf("*** shmget error (client) ***\n");
		exit(1);
	}

	ShmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
	printf("Finalizer: shared memory attached.\n");

	ShmPTR -> status = -1;
	
	show_mem(ShmPTR);

	shmdt((void *) ShmPTR);	
	shmctl(ShmID, IPC_RMID, NULL);
	printf("Finalizer: shared memory deallocated.\n");
	sem_unlink(SEM_NAME);
    sem_unlink(SEM_FILE);
    sem_unlink(SEM_ACCESS);	
    sem_unlink(SEM_READW);
    sem_unlink(SEM_READR);
    

	return 0;   

}
