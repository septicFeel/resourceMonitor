#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

//for terminal: cat /proc/stat
double readCpuUsage() {
    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL) {
        perror("Error opening /proc/stat");
        return 1;
    }

    char line[256];
    if (fgets(line, sizeof(line), fp)) {
        double user, nice, system, idle, iowait, irq, softirq;
        sscanf(line, "cpu %lf %lf %lf %lf %lf %lf %lf", &user, &nice, &system, &idle, &iowait, &irq, &softirq);

        double totalCpuTime = user + nice + system + idle + iowait + irq + softirq;
        double nonIdleCpuTime = user + nice + system + irq + softirq;
        double cpuUsage = nonIdleCpuTime / totalCpuTime * 100.0;
        fclose(fp);
        return cpuUsage;
    }

    fclose(fp);
    return 0;
}

//for terminal: cat /proc/meminfo
double readRamUsage() {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp == NULL) {
        perror("Error opening /proc/meminfo");
        return 1;
    }

    double totalMem, freeMem, availableMem, buffers, cached;
    if (fscanf(fp, "MemTotal: %lf kB\nMemFree: %lf kB\nMemAvailable: %lf kB\nBuffers: %lf kB\nCached: %lf kB\n",
               &totalMem, &freeMem, &availableMem, &buffers, &cached) == 5) {
        double usedMem = totalMem - freeMem - cached;
        double ramUsage = usedMem / totalMem * 100.0;
        fclose(fp);
        return ramUsage;
    }

    fclose(fp);
    return 2;
}

void trimExcessiveSpaces(char *str) {
    int i, j;
    bool space_found = false;

    for (i = 0, j = 0; str[i] != '\0'; i++) {
        if (!isspace(str[i])) {
            space_found = false;
            str[j++] = str[i];
        } else if (!space_found) {
            space_found = true;
            str[j++] = ' ';
        }
    }

    while (j > 0 && isspace(str[j - 1])) {
        j--;
    }

    str[j] = '\0';
}
 

void readSensorsData() {

    FILE *fp = popen("sensors", "r");
    if (fp == NULL) {
        perror("Error running 'sensors'");
        return;
    }
  
    char line[256];
    char aux[256];

    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, "CPU") != NULL) {
            strcpy(aux, line);
            trimExcessiveSpaces(aux);
            printf("Temperature for %s\n", aux);
        }
        if (strstr(line, "fan1") != NULL) {
            strcpy(aux, line);
            trimExcessiveSpaces(aux);
            printf("CPU %s\n", aux);
        }
        if (strstr(line, "GPU") != NULL) {
            strcpy(aux, line);
            trimExcessiveSpaces(aux);
            printf("Temperature for %s\n", aux);
        }
    }
    pclose(fp);
}

int main() {
    while (1) {

        double cpuUsage = readCpuUsage();
        double ramUsage = readRamUsage();
        
        if (cpuUsage >= 0 || ramUsage >= 0) {
            printf("CPU Usage: %.2f%%\n", cpuUsage);
            printf("RAM Usage: %.2f%%\n", ramUsage);
        } else {
            printf("Error reading usage information.\n");
        }

        readSensorsData();

        sleep(1);
    }

    return 0;
}