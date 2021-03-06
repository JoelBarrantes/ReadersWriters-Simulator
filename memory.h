

//THIS FILE DEFINES THE SHARED MEMORY SEGMENT STRUCTURE AND THE SEMAPHORE


#include <time.h>
#define SEM_NAME "se_q"
#define SEM_FILE "se_w"
#define SEM_READW "se_e"
#define SEM_READR "se_t"
#define SEM_ACCESS "se_r"

//Estados de la memoria
#define FILLED 0
#define AVAILABLE 1
#define CLOSED -1

//Estado del agente
#define LOCKED 0
#define SLEEPING -1
#define OPERATING 1
#define IDLE 2

#define MAX_MEM_SIZE 1024
#define MAX_WRITERS 32
#define MAX_READERS 32
#define MAX_S_READERS 32
#define KEY "."
#define VAL 'x'

#define FILE_NAME "Bitacora.txt"

typedef struct {
	int pid;
	int status;
} Agent;



struct Memory {

	int limit;

	int limit_w;
	int limit_r;
	int limit_sr;

	int status;
   
    int consecutive_r;    

	int pid[MAX_MEM_SIZE];
	time_t date_time[MAX_MEM_SIZE];	

	Agent writers[MAX_WRITERS];
	Agent readers[MAX_READERS];
	Agent sreaders[MAX_S_READERS];

};

struct args{
	int agents_q;
	int op_t;
	int sleep_t;
};





