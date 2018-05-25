
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
int num_writers = 0;

int worker_ID = 0;
int idx = 0;

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
    sem_t *semr = sem_open(SEM_READ, O_RDWR);
    sem_t *semf = sem_open(SEM_FILE, O_RDWR);
	sem_t *sema = sem_open(SEM_ACCESS, O_RDWR);
	time_t t;
	
	pid_t tid = syscall(SYS_gettid);

	pthread_mutex_lock(&m);
	Agent *agent = &(ShmPTR -> writers[worker_ID]);
	worker_ID++;
	pthread_mutex_unlock(&m);

	agent -> status = LOCKED;
	agent -> pid = tid;

	
	while(1){
		
		
	
		if (ShmPTR -> status == AVAILABLE){
			
            pthread_mutex_lock(&m);
            agent -> status = LOCKED;
            num_writers++;
            if(num_writers == 1){
                sem_wait(semr);
            }            
    
            pthread_mutex_unlock(&m);            

			sem_wait(sem);
			agent -> status = OPERATING;
			ShmPTR -> consecutive_r = 0;
               
            int PID = -1;
            int local_index;
            while(PID != 0){
                 
                //CHECK IF MEMORY IS FULL
			    full = get_empty_line(ShmPTR -> pid, ShmPTR -> limit);
			    if ( full == -1) {
                    ShmPTR -> status = 0;
                    sem_post(sem);
                }
                    
		        
                if (ShmPTR -> status == CLOSED){
			        sem_post(sem);
			        break;	
		        }            
            
                pthread_mutex_lock(&m);
                PID = ShmPTR -> pid[idx];      
                t = ShmPTR -> date_time[idx];
                local_index = idx;
                
                idx++;
                if (idx == ShmPTR -> limit){
                    idx = 0;
                }
                 
                pthread_mutex_unlock(&m) ;  
             
            }   
        

            int index = local_index;
		    
			printf("Writer with pid %d writing in line %d.\n", agent -> pid, index);	
            

            //////////LOG///////////
            sem_wait(semf);
            FILE *file = fopen(FILE_NAME,"a");

            t = time(NULL);	
            char bufflog[20];
		    strftime(bufflog, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));


	        fprintf(file,"%s:- Writer with pid %d writing in line %d.\n", bufflog, agent -> pid, index);	
            fclose(file);
            sem_post(semf);
            //////////LOG///////////

			t = time(NULL);					
			ShmPTR -> pid[index] = agent -> pid;		
			ShmPTR -> date_time[index] = t;
			sleep(write_t);		
            	
            char buff[20];
		    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));

           
            printf("Writer with pid %d wrote: \"Line: %d | PID: %d | Date Time: ",  agent -> pid , index ,agent -> pid);	
            printf("%s\".\n",buff);

            //////////LOG///////////
            sem_wait(semf);
            FILE *file1 = fopen(FILE_NAME,"a");
            
            t = time(NULL);	
            char bufflog2[20];
		    strftime(bufflog2, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
            
            
	        fprintf(file1,"%s:- Writer with pid %d wrote: \"Line: %d | PID: %d | Date Time: ", bufflog2 ,agent -> pid , index ,agent -> pid);	
            fprintf(file1,"%s\".\n",buff);
            fclose(file1);
            sem_post(semf);
            //////////LOG///////////

			//CHECK IF MEMORY IS FULL
			index = get_empty_line(ShmPTR -> pid, ShmPTR -> limit);
			if ( index == -1) ShmPTR -> status = 0;
			printf("Writer with pid %d sleeping...\n", tid);

            sem_wait(semf);
            FILE *file2 = fopen(FILE_NAME,"a");
            
            t = time(NULL);	
            char bufflog3[20];
		    strftime(bufflog3, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
            
        
            fprintf(file2,"%s:- Writer with pid %d sleeping...\n",bufflog3, tid);
            fclose(file2);            
            sem_post(semf);
			
			sem_post(sem);


            pthread_mutex_lock(&m);
            num_writers--;
            if (num_writers == 0){
                sem_post(semr);
            }
            
            pthread_mutex_unlock(&m);
            
            sched_yield();
			agent -> status = SLEEPING;
			sleep(sleep_t);
		
		}

		if (ShmPTR -> status == CLOSED){
			sem_post(sem);
			break;	
		}
		
	}

	sem_close(sem);
    sem_close(sema);
    sem_close(semr);
    sem_close(semf);
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
