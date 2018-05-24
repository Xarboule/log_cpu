#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

extern int errno;

int main(int argc, char** argv[]){
	
	if (argc != 2){
		printf("Usage : %s command\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	const int nproc = get_nprocs();
	printf("Monitoring %d CPUs.\n", nproc);
	pid_t pid = fork();

	if (pid == -1){
		perror("fork failed");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0){ // Processus de creation des logs
		int errnum;
		FILE* proc_freq_files[nproc];
		
		
		for (int i=0; i<nproc; i++){
			char path[40];
			char* number;
			sprintf(number, "%d", i);
			strcpy(path, "/sys/devices/system/cpu/cpu");
			strcat(path, number);
			strcat(path, "/cpufreq/scaling_cur_freq");
			printf("Opening file %s\n", path);
			proc_freq_files[i] = fopen(path, "r");
		}
	
		printf("Beginning logs of CPU infos\n");
		int cur_measure;

		

		while(42){ // Enregistrement des mesures periodiquement
			sleep(1);
			char result[100] = "";
			printf("Measure !\n");
			for(int i=0; i<nproc; i++){
				char atomic_result[10] = "";
				fscanf(proc_freq_files[i], "%d", &cur_measure);	
				sprintf(atomic_result, "%d", cur_measure);
				printf("CPU %d : %d\n", i, cur_measure);
				if (i == (nproc-1)){
					strcat(atomic_result, "\n");
					printf("\n");	
				}
				else {
					strcat(atomic_result, ", ");
				}
				strcat(result, atomic_result);
			}
			printf(result);
			FILE* log_file = fopen("cpu_freq.csv", "a");
			if(log_file == NULL){
				errnum = errno;
				fprintf(stderr, "Error opening log file : %s\n", strerror(errnum));
				exit(EXIT_FAILURE);
			}

			//int test = fprintf(log_file, "%s", result);

			int test = fputs(result, log_file);
			
			if(test == EOF){
				errnum = errno;
				fprintf(stderr, "Error opening log file : %s\n", strerror(errnum));
				exit(EXIT_FAILURE);
			}
			fclose(log_file);
		}
	}
	else { // Processus d'execution de l'application
		printf("Beginning the application %s ...\n", argv[1][0]);
		
		int status = system(argv[1][0]);

		//sleep(10);
		kill(pid, SIGKILL);
		printf("End of experimentation.\n");
	}
	return(EXIT_SUCCESS); 
}
