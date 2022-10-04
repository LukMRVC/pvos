#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>


void *thread_sleep(void * sleep_time) {
    sleep(*(int *)sleep_time);
    return (void *)0;
}

void *placeholder(void *s) {}

const int N = 68000;
int main(void) {
    pthread_t threads[N] = {};
    pthread_t join_handle = 0;
    const int sleep_time = 5;
    int i = 0;

    printf("Creating threads - no detach...\n");
    while (true) {
        int suc = pthread_create(&join_handle, nullptr, thread_sleep, (void *)&sleep_time);
        if (suc != 0) {
            printf("Created %d threads without Detach\n", i);
            perror("Pthread create: ");
            break;
        }
        threads[i++] = join_handle;
    }

    printf("Cleaning up...\n\n\n\n");
    void * retval = new int;
    for (int j = 0; j < i; j++) {
        pthread_join(threads[j], &retval);
    }

    // reset thread counter
    i = 0;
    printf("Creating threads - WITH DETACH...\n");
    while (true) {
        int suc = pthread_create(&join_handle, nullptr, thread_sleep, (void *)&sleep_time);
        if (suc != 0) {
            printf("Created %d threads WITH Detach\n", i);
            perror("Pthread create: ");
            break;
        }
        suc = pthread_detach(join_handle);
        i += 1;
        if (suc != 0) {
            perror("Detach: ");
            break;
        }
    }
    printf("Cleaning up...\n\n\n\n");
    sleep(sleep_time);
    // reset thread counter
    i = 0;
    printf("Creating threads - no code...\n");
    while (true) {
        int suc = pthread_create(&join_handle, nullptr, placeholder, nullptr);
        if (suc != 0) {
            printf("Created %d threads without Detach\n", i);
            perror("Pthread create: ");
            break;
        }
        if (i + 2 >= N) {
            threads[i++] = join_handle;
            printf("Breaking out of loop!\n");
            break;
        }
        threads[i++] = join_handle;
    }

    printf("Cleaning up...\n\n\n\n");
    for (int j = 0; j < i; j++) {
        pthread_join(threads[j], &retval);
    }

    return 0;
}