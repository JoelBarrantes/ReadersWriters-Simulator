
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

pthread_mutex_t m;//controla acceso al dato.
int num_readers = 0; 

int worker_ID = 0;
int idx = 0;

void *run_reader(void *args){

	struct args *arguments = args;
	
	int reader_t = arguments -> op_t;
	int sleep_t = arguments -> sleep_t;

	//printf("reader_t %d",reader_t);
	sem_t *sem = sem_open(SEM_NAME, O_RDWR);
    sem_t *semf = sem_open(SEM_FILE, O_RDWR);
    sem_t *semr1 = sem_open(SEM_READW, O_RDWR);    
	sem_t *semr2 = sem_open(SEM_READR, O_RDWR);  
  
	pid_t tid = syscall(SYS_gettid);

	pthread_mutex_lock(&m);
	Agent *agent = &(ShmPTR -> readers[worker_ID]);
	worker_ID++;
	pthread_mutex_unlock(&m);

	agent -> status = LOCKED;
	agent -> pid = tid;

    	
	while(1){
		agent -> status = LOCKED;
        //printf("Aqui 1\n");
        

        
        sem_wait(semr1);   
        sem_wait(semr2);     
        pthread_mutex_lock(&m);
 
        num_readers++;
        if( num_readers == 1){
            sem_wait(sem);
        }
	    pthread_mutex_unlock(&m);
        sem_post(semr2);
        sem_post(semr1);
                				
        

        ////////////////////////////////
        
        //printf("Aqui 2\n");
        time_t t; 
        agent -> status = OPERATING;
        ShmPTR -> consecutive_r = 0;
        int PID = 0;  
        int local_index;
        while(PID == 0){
       
		    int status = ShmPTR -> status;
		           
            if (status == CLOSED){
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
        
        int status = ShmPTR -> status;
		if (status == CLOSED){
			sem_post(sem);
			break;	
		}
		 
        char buff[20];
		strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
        printf("Reader with pid %d reading the line %d\n", agent -> pid, local_index);
		sleep(reader_t);
		printf("Reader with pid %d: Read message \"Line: %d | PID: %d | Date Time: ", agent -> pid , local_index,  PID);
		printf("%s\"\n",buff);

        sem_wait(semf);
            FILE *file = fopen(FILE_NAME,"a");

            t = time(NULL);	
            char bufflog[20];
		    strftime(bufflog, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
            
    
	        fprintf(file,"%s:- Reader with pid %d reading the line %d\n", bufflog, agent -> pid, local_index);
            fprintf(file,"%s:- Reader with pid %d: Read message \"Line: %d | PID: %d | Date Time: ", bufflog, agent -> pid, local_index, PID );	
            fprintf(file,"%s\".\n",buff);
            fclose(file);
        sem_post(semf);
            
        ////////////////////////////////

					
		
        pthread_mutex_lock(&m);
        
        num_readers--;
        if (num_readers == 0){
            sem_post(sem);
        }

        pthread_mutex_unlock(&m);
        

        
        printf("Reader with pid %d sleeping...\n", agent -> pid);   

        sem_wait(semf);
            FILE *file1 = fopen(FILE_NAME,"a");
            
            t = time(NULL);	
            char bufflog2[20];
		    strftime(bufflog2, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
            
            
	        fprintf(file1,"%s:- Reader with pid %d sleeping...\n", bufflog2, agent -> pid);	
            fclose(file1);
        sem_post(semf);

		agent -> status = SLEEPING;
		sleep(sleep_t);
        
    
        status = ShmPTR -> status;
		if (status == CLOSED){
			sem_post(sem);
			break;	
		}
		
	}

	sem_close(sem);
    sem_close(semr1);    
    sem_close(semr2);  
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
	
	struct args *arguments = malloc(sizeof(struct args));//asignación memoria tam= de la estructura de argumentos q recibe
		
	if (argc < 4 ){
		display_help();
        return 1; // si son menos de 3 argumentos  manda el help
    }     
	
	int a_q = atoi(argv[1]); //cantidad lectores
	int w_t = atoi(argv[2]); //duracion de lectura
	int s_t = atoi(argv[3]); //duracion de sueño

	pthread_t readers[a_q];	
	
	// punteros
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
	printf("Readers: shared memory attached.\n");

	ShmPTR -> limit_r = a_q;

	int t;
    srand(time(NULL));
	
	for(t = 0; t < a_q; t++){
		int rc = pthread_create(&readers[t], NULL, run_reader, arguments);
        int secs = rand()%4;
        sleep(secs);
	}
	
	for (t = 0; t < a_q; t++) {
        pthread_join(readers[t], NULL);
    }	
	
	shmdt((void *) ShmPTR);
	
    printf("Readers: shared memory deattached.\n");
    printf("Killing Readers...\n");
	return 0;
 

}
