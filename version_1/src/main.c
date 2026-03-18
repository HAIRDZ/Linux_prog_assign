#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sched.h>

#define freqFile "data/freq.txt"
#define logFile  "data/timeAndInterval.txt"

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

long periodNs = 1000000;
struct timespec currentTime;
int isUpdated = 0;

void normalizeTime(struct timespec *t) {
    while (t->tv_nsec >= 1000000000) {
        t->tv_nsec -= 1000000000;
        t->tv_sec += 1;
    }
}

void* sampleThread(void* arg) {
    struct sched_param param;
    param.sched_priority = 80;
    sched_setscheduler(0, SCHED_FIFO, &param);

    struct timespec ts;
    struct timespec nextTime;

    clock_gettime(CLOCK_MONOTONIC, &nextTime);

    while (1) {
        nextTime.tv_nsec += periodNs;
        normalizeTime(&nextTime);

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &nextTime, NULL);

        clock_gettime(CLOCK_REALTIME, &ts);

        pthread_mutex_lock(&lock);
        currentTime = ts;
        isUpdated = 1;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
    }
}

void* inputThread(void* arg) {
    long newPeriod;

    while (1) {
        FILE *f = fopen(freqFile, "r");
        if (f) {
            if (fscanf(f, "%ld", &newPeriod) == 1) {
                pthread_mutex_lock(&lock);
                periodNs = newPeriod;
                pthread_mutex_unlock(&lock);
            }
            fclose(f);
        }
        sleep(1);
    }
}

void* loggingThread(void* arg) {
    FILE *f = fopen(logFile, "w");
    struct timespec prev = {0};

    while (1) {
        pthread_mutex_lock(&lock);

        while (!isUpdated)
            pthread_cond_wait(&cond, &lock);

        struct timespec now = currentTime;
        isUpdated = 0;

        pthread_mutex_unlock(&lock);

        long interval = 0;
        if (prev.tv_sec != 0) {
            interval = (now.tv_sec - prev.tv_sec) * 1000000000L +
                       (now.tv_nsec - prev.tv_nsec);
        }

        fprintf(f, "%ld.%09ld , %ld ns\n",
                now.tv_sec, now.tv_nsec, interval);
        fflush(f);

        prev = now;
    }
}

int main() {
    pthread_t sampleTid, inputTid, loggingTid;

    pthread_create(&sampleTid, NULL, sampleThread, NULL);
    pthread_create(&inputTid, NULL, inputThread, NULL);
    pthread_create(&loggingTid, NULL, loggingThread, NULL);

    sleep(300);

    return 0;
}
