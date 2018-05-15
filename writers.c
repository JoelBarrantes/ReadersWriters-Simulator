
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
#include <sys/syscall.h>  

#include "memory.h"


struct Memory  *ShmPTR;

pthread_mutex_t m;
int worker_ID = 0;

int get_empty_line(int pid[MAX_MEM_SIZE], int limit){

	for(int i = 0; i < limit; i++){
			
		int val = pid[i];
		if (val == 0){
			return i;
		}

	}
	return -1;
}


void *run_writer(void *args){

	struct args *arguments = args;
	
	int write_t = arguments -> op_t;
	int sleep_t = arguments -> sleep_t;

	sem_t *sem = sem_open(SEM_NAME, O_RDWR);    
	time_t t;
	
	pid_t tid = syscall(SYS_gettid);

	pthread_mutex_lock(&m);
	Agent *agent = &(ShmPTR -> writers[worker_ID]);
	worker_ID++;
	pthread_mutex_unlock(&m);

	agent -> status = LOCKED;
	agent -> pid = tid;

	
	while(1){
		agent -> status = LOCKED;
		sem_wait(sem);
		int status = ShmPTR -> status;
		sem_post(sem);
				
		if (status == AVAILABLE){
			

			sem_wait(sem);
			agent -> status = OPERATING;
			int index = get_empty_line(ShmPTR -> pid, ShmPTR -> limit);
		 				
			printf("Process with pid %d writing in line %d.\n", tid, index);		
			t = time(NULL);					
			ShmPTR -> pid[index] = tid;		
			ShmPTR -> date_time[index] = t;
			sleep(write_t);			

			//CHECK IF MEMORY IS FULL
			index = get_empty_line(ShmPTR -> pid, ShmPTR -> limit);
			if ( index == -1) ShmPTR -> status = 0;
			printf("Process with pid %d sleeping...\n", tid);		
			sem_post(sem);
			agent -> status = SLEEPING;
			sleep(sleep_t);
		
		}

		if (status == CLOSED){
			sem_post(sem);
			break;	
		}
		
	}

	sem_close(sem);
	return NULL;

};

void display_help(){
	printf("RUN THE PROGRAM WITH THE FOLLOWING FORMAT:\n");
	printf("writers <number of writers> <write duration> <sleep duration>\n");
	printf("Example: writers 5 3 2\n");
}

int main(int argc , char *argv[]) {

	int ShmID;
	
	struct args *arguments = malloc(sizeof(struct args));
		
	if (argc < 4 ){
		display_help();
        return 1;
    }     
	
	int a_q = atoi(argv[1]); //number of agents
	int w_t = atoi(argv[2]); //write duration
	int s_t = atoi(argv[3]); //sleep turation

	pthread_t writers[a_q];	
	
	arguments -> agents_q = a_q;
	arguments -> op_t = w_t;
	arguments -> sleep_t = s_t;
	
	pthread_mutex_init(&m, NULL);

	key_t ShmKEY = ftok(KEY, VAL);	
	ShmID = shmget(ShmKEY, sizeof(struct Memory), 0666);
	if (ShmID < 0) {
		printf("*** shmget error (client) ***\n");
		exit(1);
	}
	ShmPTR = (struct Memory *) shmat(ShmID, NULL, 0);
	printf("Writers: shared memory attached.\n");

	ShmPTR -> limit_w = a_q;

	int t;
	
	for(t = 0; t < a_q; t++){
		int rc = pthread_create(&writers[t], NULL, run_writer, arguments);
	}
	
	for (t = 0; t < a_q; t++) {
        pthread_join(writers[t], NULL);
    }	
	
	shmdt((void *) ShmPTR);
	
    printf("Writers: shared memory deattached.\n");
    printf("Killing writers...\n");
	return 0;
 

}
