#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "buffer.h"

static pthread_mutex_t mutex;
sem_t full;
sem_t empty;
buffer_item buffer[BUFFER_SIZE];
int count;
int sigint_flag = 0;

void sig_handler(int sig_num) {

    if (sig_num == SIGINT)
        sigint_flag = 1;

    fprintf(stdout, " (SIGINT) caught\n");
}

int insert_item(buffer_item item) {

    int retVal = 0;

    if (sem_wait(&empty) == -1)
        fprintf(stderr, "%s\n", strerror(errno));
    if (pthread_mutex_lock(&mutex) != 0)
        fprintf(stderr, "%s\n", strerror(errno));

    if (count < BUFFER_SIZE) {
        buffer[count] = item;
        count++;
        printf("producer produced %d\n", item);
        // fprintf(stdout, "Inventory = %d\n", count); // debug
    }
    else
        retVal = -1;

    pthread_mutex_unlock(&mutex);
    sem_post(&full); // increment full so it can be consumed by a consumer

    return retVal;
}

int remove_item(buffer_item *item) {

    int retVal = 0;     // return value
    sem_wait(&full);    // wait until full > 0

    if (pthread_mutex_lock(&mutex) != 0)    // lock the mutex so count stays accurate, only a single thread can access
        fprintf(stderr, "%s\n", strerror(errno));

    if (count > 0) {                // make sure the count > 0, otherwise something went wrong
        *item = buffer[count - 1];  // set item to the last number inserted into the buffer
        count--;                    // decrement the count
        fprintf(stdout, "consumer consumed %d\n", *item);
        // fprintf(stdout, "Inventory = %d\n", count); // debug
    } else
        retVal = -1;

    pthread_mutex_unlock(&mutex);   // unlock the mutex
    sem_post(&empty);               // increment empty so a producer can produce

    return retVal;
}

static void *producer(void * param) {
    buffer_item item;

    while (1) {
        int sleepVal = 1 + (rand() % 6);
        sleep((unsigned int) sleepVal);
        item = rand();

        if (sigint_flag == 1) {
            break;
        }

        if (insert_item(item) == -1) {
            fprintf(stderr, "Error condition\n");
        }
    }
}

static void *consumer(void * param) {
    buffer_item item;

    while (1) {
        int sleepVal = 1 + (rand() % 6);
        sleep((unsigned int) sleepVal);

        if (sigint_flag == 1) {
            break;
        }
        if (remove_item(&item) == -1) {
            fprintf(stderr, "Error condition\n");
        }
    }
}

int main(int argc, char *argv[]) {

    if (argc != 4) {
        fprintf(stdout, "Usage: %s [how long to run for] [# of producer threads] [# of consumer threads]\n", argv[0]);
        exit(1);
    }

    if (sem_init(&empty, 0, BUFFER_SIZE) == -1) {   // initalize semaphore empty, starts with a value of 5
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }

    if (sem_init(&full, 0, 0) == -1) {              // initalize semaphore full, starts with a value of 0
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }

    long sleepTime = strtol(argv[1], NULL, 0);   // the sleep time from user input
    long numProducer = strtol(argv[2], NULL, 0);                        // number of producers from user input
    long numConsumer = strtol(argv[3], NULL, 0);                        // number of consumers from user input
    count = 0;                                                          // set global variable count to zero
    void *res;

    if ((sleepTime == 0) || (numConsumer == 0) || (numProducer == 0)) {
        if (sleepTime == 0) {
            fprintf(stderr, "Enter an integer for the sleep time (>0)\n");
            exit(1);
        } else if (numConsumer == 0) {
            fprintf(stderr, "Enter an integer for the number of consumers (>0)\n");
            exit(1);
        } else if (numProducer == 0) {
            fprintf(stderr, "Enter an integer for the number of producers (>0)\n");
            exit(1);
        }
    }

    if ((sleepTime < 0) || (numConsumer < 0) || (numProducer < 0)) {
        sleepTime = abs(sleepTime);
        numConsumer = abs(numConsumer);
        numProducer = abs(numProducer);
        fprintf(stderr, "Negative inputs detected. Using absolute values: %ld, %ld, %ld\n", sleepTime, numConsumer,numProducer);
    }

    pthread_t threads[numConsumer + numProducer];   // array of pthreads that are the producers and consumers
    if (pthread_mutex_init(&mutex, NULL) != 0) {    // initalize the pthread mutex
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }

    for (int ii = 0; ii < numProducer; ii++) {      // create the producers threads
        if (pthread_create(&threads[ii], NULL, producer, &ii) != 0)
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }
    for (int ii = (int) numProducer; ii < numConsumer + numProducer; ii++) {    //create consumer threads
        if (pthread_create(&threads[ii], NULL, consumer, &ii) != 0)
            fprintf(stderr, "ERROR: %s\n", strerror(errno));
    }

    if (signal(SIGINT, sig_handler) == SIG_ERR)     // signal handler to catch SIGINT
        fprintf(stderr, "ERROR: %s\n", strerror(errno));

    sleep(sleepTime);                               // sleep for specified amount of time

    pthread_mutex_unlock(&mutex);                   // make sure the mutex is unlocked

    /*
     * cancel all of the threads for deferred cancellation, this will cause the threads to quit
     * when they are at certain function/system calls such as sleep. After the threads are cancelled
     * join all of the threads so they all exit at the same time/
     */
    for (int ii = 0; ii < numConsumer + numProducer; ii++) {
        if (pthread_cancel(threads[ii]) != 0) {
            fprintf(stdout, "ERROR: %s\n", strerror(errno));
        }
        if (pthread_join(threads[ii], &res) != 0) {
            fprintf(stdout, "ERROR: %s\n", strerror(errno));
        }
        if (res == PTHREAD_CANCELED) {
            // fprintf(stdout, "thread %d canceled\n", ii);
        }
    }
    printf("All threads (consumers/producers) closed.\n");
    // destroy the mutex and semaphores
    pthread_mutex_destroy(&mutex);
    sem_destroy(&full);
    sem_destroy(&empty);

    return 0;
}