#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "msr_utils.h"

extern int errno;

int main(int argc, char * argv[]){
	
	if (argc != 2){
		printf("Usage : %s command\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	const int nproc = get_nprocs();
	printf("Monitoring %d CPUs.\n", nproc);

	printf("Initializing MSR for uncore frequency reading...\n");


	int status = system("modprobe msr");
	if(status == -1 || WEXITSTATUS(status) != 0){
		exit(EXIT_FAILURE);
		
	}
	//printf("WEXITSTATUS = %d\n", status);
	status = system("wrmsr 0x391 0x20000000");
	if(status == -1 || WEXITSTATUS(status) != 0){
		exit(EXIT_FAILURE);
		
	}
	//printf("WEXITSTATUS = %d\n", status);
	if(system("wrmsr 0x394 0x400000") != 0){
		exit(EXIT_FAILURE);
		
	}


	//Fork

	pid_t pid = fork();

	if (pid == -1){
		perror("fork failed");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0){ // Measuring process
		int errnum;
		FILE* proc_freq_files[nproc];
		
		int uncore_freq_file = open("/dev/cpu/0/msr", O_RDONLY);
		if(uncore_freq_file == -1){
			errnum = errno;
			fprintf(stderr, "Error opening msr file : %s\n", strerror(errnum));
			exit(EXIT_FAILURE);
		}


		for (int i=0; i<nproc; i++){
			char path[40];
			char number[11] = "";
			sprintf(number, "%d", i);
			strcpy(path, "/sys/devices/system/cpu/cpu");
			strcat(path, number);
			strcat(path, "/cpufreq/scaling_cur_freq");
			printf("Opening file %s\n", path);
			proc_freq_files[i] = fopen(path, "r");
		}
	
		printf("Beginning logs of CPU infos\n");
		int cur_measure;

		uint64_t uncore_freq;

		while(42){ // Periodic measures
			static uint64_t previous_uncore_clk;
			previous_uncore_clk = read_msr(0, 0x395);
			sleep(1);
			char result[100] = "";
			
			printf("Measure !\n");
			//MSR measure for uncore freq
			uint64_t cur_uncore_clk = read_msr(0, 0x395);
			previous_uncore_clk = cur_uncore_clk;
			uncore_freq = (cur_uncore_clk - previous_uncore_clk)/1000;
			printf("Uncore frequency : %" PRId64 "\n", uncore_freq);
			char uncore_result[20] = "";
			sprintf(uncore_result, "%" PRId64 ", ", uncore_freq);
			strcat(result, uncore_result);

			for(int i=0; i<nproc; i++){
				char atomic_result[10] = "";
				fseek(proc_freq_files[i], 0, SEEK_SET);
				fscanf(proc_freq_files[i], "%d", &cur_measure);	
				fflush(proc_freq_files[i]);
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

			int test = fputs(result, log_file);
			
			if(test == EOF){
				errnum = errno;
				fprintf(stderr, "Error writing into log file : %s\n", strerror(errnum));
				exit(EXIT_FAILURE);
			}
			fclose(log_file);
			close(uncore_freq_file);
		}
	}
	else { // Application process 
		printf("Beginning the application %s ...\n", argv[1]);
	
		int errnum;

		int status = system(argv[1]);

		if (status == -1){
			errnum = errno;
			fprintf(stderr, "Error executing the command : %s\n", strerror(errnum));
			kill(pid, SIGKILL);
			exit(EXIT_SUCCESS);
		}

		kill(pid, SIGKILL);
		printf("End of experimentation.\n");
	}
	return(EXIT_SUCCESS); 
}
