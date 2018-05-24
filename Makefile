all: log_cpu_freq.c
	gcc -Wall -O3 -o log_cpu log_cpu_freq.c


clean:
	$(RM) log_cpu
