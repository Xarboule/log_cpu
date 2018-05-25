all: log_cpu_freq.o msr_utils.o 
	gcc -Wall -O3 -o log_cpu log_cpu_freq.o msr_utils.o

log_cpu_freq.o: log_cpu_freq.c
	gcc -Wall -O3 -o log_cpu_freq.o -c log_cpu_freq.c

msr_utils.o: msr_utils.h msr_utils.c
	gcc -Wall -O3 -o msr_utils.o -c msr_utils.c

clean:
	$(RM) log_cpu
	$(RM) *.o
