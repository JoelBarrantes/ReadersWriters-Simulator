
#include <time.h>
#define SEM_NAME "hmmm"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)


	struct Memory {
		int limit;
		int status;
		int pid[256];
		time_t date_time[256];
	};
