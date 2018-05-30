#include <stdio.h>
# pragma GCC optimize("O0")
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "msr_utils.h"

//define XEON

extern int errno;

void write_msr_config(){

	int status = system("modprobe msr");
	if(status == -1 || WEXITSTATUS(status) != 0){
		exit(EXIT_FAILURE);
		
	}
	write_msr(0,0x391,1<<29);
	write_msr(0,0x394,1<<22);
}

void write_msr_config_xeon(){

	int status = system("modprobe msr");
	if(status == -1 || WEXITSTATUS(status) != 0){
		exit(EXIT_FAILURE);
		
	}
	write_msr(0,0x703,1<<22);
}



int main(int argc, char * argv[]){
	
	if (argc != 2){
		printf("Usage : %s command\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	const int nproc = get_nprocs();
	printf("Monitoring %d CPUs.\n", nproc);

	printf("Initializing MSR for uncore frequency reading...\n");

#ifdef XEON
	write_msr_config_xeon();
#else
	write_msr_config();
#endif

	//Fork

	pid_t pid = fork();

	if (pid == -1){
		perror("fork failed");
		exit(EXIT_FAILURE);
	}
	else if (pid == 0){ // Measuring process
		int errnum;
		FILE* proc_freq_files[nproc];
		
		//int uncore_freq_file = open("/dev/cpu/0/msr", O_RDONLY);
		/*if(uncore_freq_file == -1){
			errnum = errno;
			fprintf(stderr, "Error opening msr file : %s\n", strerror(errnum));
			exit(EXIT_FAILURE);
		}*/


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
		
#ifdef XEON
		int addr_msr_read = 0x704;
#else
		int addr_msr_read = 0x395;
#endif

		int cur_measure;

		uint64_t uncore_freq;

		
		uint64_t previous_uncore_clk, cur_uncore_clk;

		struct timespec res1, res2;
		while(42){ // Periodic measures
			res2.tv_sec = 0;
			clock_gettime(CLOCK_MONOTONIC, &res1);
			previous_uncore_clk = read_msr(0, addr_msr_read);
			usleep(1000);	
			cur_uncore_clk = read_msr(0, addr_msr_read);
			clock_gettime(CLOCK_MONOTONIC, &res2);
			sleep(1);
			previous_uncore_clk &= ((1uL<<48)-1);
		       	cur_uncore_clk &= ((1uL<<48)-1);
			
			char result[300] = "";
			printf("Measure !\n");
			//MSR measure for uncore freq
			uint64_t elapsed = ((res2.tv_sec-res1.tv_sec)*1000000000 + (res2.tv_nsec - res1.tv_nsec))/1000000;
			uncore_freq = (cur_uncore_clk - previous_uncore_clk)/elapsed; // KHz
			printf("Uncore Frequency: %li\n", uncore_freq);
			previous_uncore_clk = cur_uncore_clk;
			char uncore_result[20] = "";
			sprintf(uncore_result, "%li, ", uncore_freq);
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
			//close(uncore_freq_file);
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
