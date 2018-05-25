
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

int num_sreaders = 0;
pthread_mutex_t m;
int worker_ID = 0;


void *run_sreader(void *args){

	struct args *arguments = args;
	
	
	int sreader_t = arguments -> op_t;
	int sleep_t = arguments -> sleep_t;

	sem_t *sem = sem_open(SEM_NAME, O_RDWR);
    sem_t *semr = sem_open(SEM_READR, O_RDWR);
    sem_t *semf = sem_open(SEM_FILE, O_RDWR);
	sem_t *sema = sem_open(SEM_ACCESS, O_RDWR);	
	time_t t;
	
	pid_t tid = syscall(SYS_gettid);

	pthread_mutex_lock(&m);
	Agent *agent = &(ShmPTR -> sreaders[worker_ID]);
	worker_ID++;
	pthread_mutex_unlock(&m);

	agent -> status = LOCKED;
	agent -> pid = tid;
    srand(time(NULL));
	
	while(1){
        
     
		if (ShmPTR -> status == CLOSED){
			sem_post(sem);
			break;	
		}


        agent -> status = LOCKED;
        if (ShmPTR -> consecutive_r > 2 ){
            continue;            
        }
		
		
        pthread_mutex_lock(&m);
    

        num_sreaders++;
        if(num_sreaders == 1){
            sem_wait(semr);
        }            

        pthread_mutex_unlock(&m);            



	    sem_wait(sem);
	    agent -> status = OPERATING;
        ShmPTR -> consecutive_r++;
        
        int index = rand();
        index = index % (ShmPTR -> limit);
        
        

	    printf("Selfish reader with pid %d reading line %d.\n", agent -> pid, index);	
        

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

    
        
        
        int PID = ShmPTR -> pid[index];      
            

        if(PID != 0){

            //READERS EGOISTAS --- CONTADORES

            t = ShmPTR -> date_time[index];
 	        char buff[20];
            strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
       
            printf("Selfish reader with pid %d: Read message \"Line: %d | PID: %d | Date Time: ", agent -> pid , index,  agent -> pid);
            printf("%s\"\n",buff);
            
            //////////LOG///////////
            sem_wait(semf);
            FILE *file1 = fopen(FILE_NAME,"a");
            
            t = time(NULL);	
            char bufflog1[20];
            strftime(bufflog1, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
            
            
            fprintf(file1,"%s:- Selfish reader with pid %d: Read message \"Line: %d | PID: %d | Date Time: ", bufflog1 ,agent -> pid , index ,agent -> pid);	
            fprintf(file1,"%s\".\n",buff);
            fclose(file1);
            sem_post(semf);
            //////////LOG///////////


            sleep(sreader_t);


            printf("Selfish reader with pid %d: Deleting message \"Line: %d | PID: %d | Date Time: ", agent -> pid , index,  PID);
            printf("%s\"\n",buff);
            ShmPTR -> pid[index] = 0;	

            //////////LOG///////////
            sem_wait(semf);
            FILE *file3 = fopen(FILE_NAME,"a");
            
            t = time(NULL);	
            char bufflog2[20];
            strftime(bufflog2, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
            
            
            fprintf(file3,"%s:- Selfish reader with pid %d: Deleting message \"Line: %d | PID: %d | Date Time: ", bufflog2 ,agent -> pid , index ,agent -> pid);	
            fprintf(file3,"%s\".\n",buff);
            fclose(file3);
            sem_post(semf);
            //////////LOG///////////

        }

            

	    printf("Selfish reader with pid %d sleeping...\n", tid);

        //////////LOG///////////            
        sem_wait(semf);
        FILE *file2 = fopen(FILE_NAME,"a");
        
        t = time(NULL);	
        char bufflog3[20];
        strftime(bufflog3, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
        
    
        fprintf(file2,"%s:- Selfish reader with pid %d sleeping...\n",bufflog3, tid);
        fclose(file2);            
        sem_post(semf);
        //////////LOG///////////			


	    sem_post(sem);
        
        pthread_mutex_lock(&m);
        num_sreaders--;
        if (num_sreaders == 0){
            sem_post(semr);
        }
        
        pthread_mutex_unlock(&m);

	    agent -> status = SLEEPING;
	    sleep(sleep_t);
	
	

	    if (ShmPTR -> status == CLOSED){
		    sem_post(sem);
		    break;	
	    }
	
    }

    sem_close(sem);
    sem_close(semr);
sem_close(sema);
    sem_close(semf);
    return NULL;

};

void display_help(){
	printf("RUN THE PROGRAM WITH THE FOLLOWING FORMAT:\n");
	printf("writers <number of writers> <write duration> <sleep duration>\n");
	printf("Example: sreaders 5 3 2\n");
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

	pthread_t sreaders[a_q];	
	
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
	printf("Selfish readers: shared memory attached.\n");

	ShmPTR -> limit_sr = a_q;

	int t;
	
	for(t = 0; t < a_q; t++){
		int rc = pthread_create(&sreaders[t], NULL, run_sreader, arguments);
	}
	
	for (t = 0; t < a_q; t++) {
        pthread_join(sreaders[t], NULL);
    }	
	
	shmdt((void *) ShmPTR);
	
    printf("Selfish readers: shared memory deattached.\n");
    printf("Killing selfish readers...\n");
	return 0;
 

}
