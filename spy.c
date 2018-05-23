
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

void show_writers(struct Memory* ShmPTR) {
	//sem_t *sem = sem_open(SEM_NAME, O_RDWR);  	
	int i = 0;	
	//sem_wait(sem);
	int lim = ShmPTR -> limit_w;	
	for(i = 0; i < lim; i++){
		Agent writer = ShmPTR -> writers[i];	
		if(writer.status == SLEEPING){
			printf("PID: %d | Status: Sleeping\n", writer.pid);
		} else if(writer.status == LOCKED){
			printf("PID: %d | Status: Locked\n", writer.pid);
		} else if(writer.status == OPERATING){
			printf("PID: %d | Status: Operating\n", writer.pid);
		}

		
		
	}
	//sem_post(sem);
	//sem_close(sem);
}

void show_mem(struct Memory* ShmPTR) {
	//sem_t *sem = sem_open(SEM_NAME, O_RDWR);  	
	int i = 0;	
	//sem_wait(sem);
	int lim = ShmPTR -> limit;	
	for(i = 0; i < lim; i++){
		time_t t = ShmPTR -> date_time[i];

		char buff[20];
		strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
		
		printf("Line: %d | PID: %d | Date Time: ", i, ShmPTR -> pid[i]);
		printf("%s\n",buff);
		
	}
	//sem_post(sem);
	//sem_close(sem);
}


void show_readers(struct Memory* ShmPTR) {
	//sem_t *sem = sem_open(SEM_NAME, O_RDWR);  	
	int i = 0;	
	//sem_wait(sem);
	int lim = ShmPTR -> limit_r;	
	for(i = 0; i < lim; i++){
		Agent reader = ShmPTR -> readers[i];	
		if(reader.status == SLEEPING){
			printf("PID: %d | Status: Sleeping\n", reader.pid);
		} else if(reader.status == LOCKED){
			printf("PID: %d | Status: Locked\n", reader.pid);
		} else if(reader.status == OPERATING){
			printf("PID: %d | Status: Operating\n", reader.pid);
		}

		
		
	}
	//sem_post(sem);
	//sem_close(sem);
}

int main(int argc , char *argv[]){

	//THIS CODE IS REUSABLE	
	int ShmID;
	struct Memory *ShmPTR;
	
	key_t ShmKEY = ftok(KEY, VAL);

	ShmID = shmget(ShmKEY, sizeof(struct Memory), 0666);
	if (ShmID < 0) {
		printf("*** shmget error (client) ***\n");
		exit(1);
	}

	ShmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
	printf("Spy: shared memory attached.\n");
	//THIS CODE IS REUSABLE




	printf("---------------MEMORY DUMP---------------\n");
	show_mem(ShmPTR);	
	printf("---------------MEMORY DUMP---------------\n\n");

	printf("-------------WRITERS STATUS--------------\n");
	show_writers(ShmPTR);	
	printf("-------------WRITERS STATUS--------------\n\n");

    printf("-------------READERS STATUS--------------\n");
	show_readers(ShmPTR);	
	printf("-------------READERS STATUS--------------\n\n");


	//THIS CODE IS REUSABLE
	shmdt((void *) ShmPTR);	
    printf("Spy: shared memory deattached.\n");
	return 0;
 	//THIS CODE IS REUSABLE
 

	return 0;

}
