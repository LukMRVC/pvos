#include <cstdio>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <cstring>
#include <sys/mman.h>
#include <ctime>
#include <cstdlib>
#define MAX_CRATE_ITEM_SIZE 20

struct crate {
    uint capacity;
    uint state;
    char *data[MAX_CRATE_ITEM_SIZE];
};

int main() {
    const char * items[5] = {"one", "two", "three", "four", "five"};
    srand(time(nullptr));
    int shmfd = shm_open("/crate", O_RDWR, 0600);
    if (shmfd < 0) {
        perror("shm_open shmfd");
        return -1;
    }
    uint crate_struct_len = sizeof (uint) * 2;
    crate * crate_ptr = (crate * ) mmap(nullptr, crate_struct_len, PROT_WRITE, MAP_SHARED, shmfd, 0);


    // create semaphores Shared mem
    int semfd = shm_open("/semaphores", O_RDWR, 0600);
    if (semfd < 0) {
        perror("shm_open semaphore");
        return -1;
    }
    // map semaphores, 0 - consumer sem, 1 - memory access sem, 2 - producer sem
    sem_t* semptr = (sem_t * ) mmap(nullptr, sizeof (sem_t) * 2, PROT_WRITE, MAP_SHARED, semfd, 0);
    // memory access binary semaphore
    sem_t * memsem_ptr = semptr + sizeof (sem_t);
    // producer semaphore
    sem_t * producer_sem = semptr + sizeof (sem_t) * 2;

    // now do the producing!!
    int sleep_time = rand() % 2 + 1;
    char buf[128];
    while (true) {
        // wait to unlock producing
        sem_wait(producer_sem);
        // wait to get memory access
        sem_wait(memsem_ptr);
        printf("Producer %d producing!\n", getpid());
        auto written = sprintf(buf, "(%d) %s,", getpid(), items[crate_ptr->state]);
        std::memcpy(crate_ptr->data + crate_ptr->state * MAX_CRATE_ITEM_SIZE, buf, written);
        crate_ptr->state += 1;
        if (crate_ptr->state == crate_ptr->capacity - 1) {
            // unlock consumer
            sem_post(semptr);
        }
        // unlock memory access
        sem_post(memsem_ptr);
        sleep(sleep_time);
    }

    return 0;
}
