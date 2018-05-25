
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

	printf("RUN THE PROGRAM WITH THE FOLLOWING FORMAT:\n");
	printf("initializer <memory lines>. The max number for \"memory lines\" is 1024\n");
	printf("Example: initializer 256\n");
	
}

int main(int argc , char *argv[]) {
	
	sem_t *sem = sem_open(SEM_NAME, O_CREAT , 0666, 1);
    sem_t *semr1 = sem_open(SEM_READW, O_CREAT , 0666, 1);
	 sem_t *semr2 = sem_open(SEM_READR, O_CREAT , 0666, 1);
    sem_t *sema = sem_open(SEM_ACCESS, O_CREAT , 0666, 1);
    sem_t *semf = sem_open(SEM_FILE, O_CREAT , 0666, 1);
    

    int ShmID;
	int mem_q;
	struct Memory *ShmPTR;
	
	if (argc < 2 ){
		display_help();
        return 1;
    }     

	mem_q = atoi(argv[1]);
	key_t ShmKEY = ftok(KEY, VAL);

    ShmID = shmget(ShmKEY, sizeof(struct Memory), IPC_CREAT | 0666);
    if (ShmID < 0) {
    	printf("Memory could not be allocated.\n");
    	exit(1);
    }
    printf("Shared memory allocated.\n");
	
	ShmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
    printf("Shared memory attached.\n");

	ShmPTR -> limit = mem_q;
	ShmPTR -> status = 1;
    ShmPTR -> consecutive_r = 0;

	for(int i = 0; i < mem_q; i++){
		ShmPTR -> pid[i] = 0;
	}	

	/*while(1){
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
	*/
    FILE *file1 = fopen(FILE_NAME,"a");
    fprintf(file1,"------------------------------------------------------------------------------------------\n");
    fclose(file1);

	sem_close(sem);
	sem_close(sema);
    sem_close(semf);
    sem_close(semr1);
    sem_close(semr2);
	return 0;

}
