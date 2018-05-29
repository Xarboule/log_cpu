#include "msr_utils.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>


extern int msr_dev;

void write_msr(int core, long addr, uint64_t msr_val)
{	
    if(msr_dev==0){
    char path[MAX_MSR_PATH];
 
    if (snprintf(path, MAX_MSR_PATH, "/dev/cpu/%d/msr", core) < 0) {
        perror("snprintf");
        exit(EXIT_FAILURE);
    }
 
    msr_dev = open(path, O_RDWR);
    if (msr_dev < 0) {
        perror("open");
        exit (EXIT_FAILURE);
    }}
 
    if (lseek(msr_dev, addr, SEEK_SET) < 0) {
        perror("lseek");
    }
 
    if (write(msr_dev, &msr_val, sizeof(msr_val)) != sizeof(msr_val)) {
        perror("write");
        exit(EXIT_FAILURE);
    }
 
}


uint64_t read_msr(int core, long addr)
{
    if(msr_dev == 0){
    char path[MAX_MSR_PATH];
 
    if (snprintf(path, MAX_MSR_PATH, "/dev/cpu/%d/msr", core) < 0) {
        perror("snprintf");
        exit(EXIT_FAILURE);
    }
 
    msr_dev = open(path, O_RDWR);
    if (msr_dev < 0) {
        perror("open");
        exit (EXIT_FAILURE);
    }}
 
    uint64_t msr_val;
    if (lseek(msr_dev, addr, SEEK_SET) < 0) {
        perror("lseek");
    }
 
    if (read(msr_dev, &msr_val, sizeof(msr_val)) != sizeof(msr_val)) {
        perror("read");
        exit(EXIT_FAILURE);
    }
 
 
    return msr_val;
}



