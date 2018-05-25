#include "msr_utils.h"

void write_msr(int core, long addr, u_int64_t msr_val)
{
    FILE *msr_dev;
    char path[MAX_MSR_PATH];
 
    if (snprintf(path, MAX_MSR_PATH, "/dev/cpu/%d/msr", core) < 0) {
        perror("snprintf");
        exit(EXIT_FAILURE);
    }
 
    msr_dev = fopen(path, "w");
    if (msr_dev == NULL) {
        perror("fopen");
        exit (EXIT_FAILURE);
    }
 
    if (fseek(msr_dev, addr, SEEK_SET) < 0) {
        perror("fseek");
    }
 
    if (fwrite(&msr_val, sizeof(msr_val), 1, msr_dev) != 1) {
        perror("fwrite");
        exit(EXIT_FAILURE);
    }
 
    fclose(msr_dev);
}


u_int64_t read_msr(int core, long addr)
{
    FILE *msr_dev;
    u_int64_t msr_val;
    char path[MAX_MSR_PATH];
 
    if (snprintf(path, MAX_MSR_PATH, "/dev/cpu/%d/msr", core) < 0) {
        perror("snprintf");
        exit(EXIT_FAILURE);
    }
 
    msr_dev = fopen(path, "r");
    if (msr_dev == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
 
    if (fseek(msr_dev, addr, SEEK_SET) < 0) {
        perror("fseek");
    }
 
    if (fread(&msr_val, sizeof(msr_val), 1, msr_dev) != 1) {
        perror("fread");
        exit(EXIT_FAILURE);
    }
 
    fclose(msr_dev);
 
    return msr_val;
}



