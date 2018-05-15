
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


void display_help(){
	printf("# OF LINES MISSING\n");
	
}

void show_mem(struct Memory* ShmPTR, sem_t *sem) {
	int i = 0;	
	sem_wait(sem);
	int lim = ShmPTR -> limit;	
	for(i = 0; i < lim; i++){
		time_t t = ShmPTR -> date_time[i];

		char buff[20];
		strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
		
		printf("%d | ", ShmPTR -> pid[i]);
		printf("%s\n",buff);
		
	}
	sem_post(sem);
}

int main(int argc , char *argv[]) {
	int sema;
	sem_t *sem = sem_open(SEM_NAME, O_CREAT , 0666, 1);
	sem_getvalue(sem, &sema);
	printf("%d\n", sema );

	key_t ShmKEY;
    int ShmID;
	int mem_q;
	struct Memory *ShmPTR;
	
	if (argc < 2 ){
		display_help();
        return 1;
    }     

	mem_q = atoi(argv[1]);

	ShmKEY = ftok(".", 'x');

    ShmID = shmget(ShmKEY, sizeof(struct Memory), IPC_CREAT | 0666);
    if (ShmID < 0) {
    	printf("Memory could not be allocated.\n");
    	exit(1);
    }
    printf("Shared memory allocated.\n");
	
	ShmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
    if ((int) ShmPTR == -1) {
    	printf("*** shmat error (server) ***\n");
    	exit(1);
    }
    printf("Shared memory attached.\n");
	ShmPTR -> limit = mem_q;
	ShmPTR -> status = 0;
	
	while(1){
		if (ShmPTR -> status == 1) {
			sleep(3);
			show_mem(ShmPTR, sem);
			printf("------------------------\n\n");
		}
		if (ShmPTR -> status == -1){
			printf("exiting...\n");
			sleep(3);			
			break;
		}
	}

	show_mem(ShmPTR, sem);
    shmdt((void *) ShmPTR);
	
	printf("Shared memory deattached.\n");
    shmctl(ShmID, IPC_RMID, NULL);
	printf("Shared memory deallocated.\n");
	int sem_close(sem_t *sem);
	return 0;

}
