#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <sys/sem.h>
#include <sys/shm.h>
#define MAX_CRATE_ITEM_SIZE 20
#define MIN(a, b) a < b ? a : b


struct crate {
    uint capacity;
    uint state;
    char *data[MAX_CRATE_ITEM_SIZE];
};

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "Start with mode [producer|consumer]");
        return 1;
    }
    bool is_producer = strcmp(argv[1], "producer") == 0;
    auto key = ftok("/vcrate", 0xCAFE);
    const char * items[9] = {"one", "two", "three", "four", "five", "six", "seven", "eight", "nine"};
    srand(time(nullptr));
    int shmfdid = shmget(key, sizeof (uint) * 2, 0);
    crate * crate_ptr = (crate *) shmat(shmfdid, nullptr, 0);
    if (*((int *)(crate_ptr)) < 0) {
        perror("shmat");
        return -1;
    }
    uint crate_struct_data_len = sizeof (uint) * 2 + crate_ptr->capacity * MAX_CRATE_ITEM_SIZE;

    // connect semaphores
    auto semkey = ftok("/vsem", 0xCAFE);
    auto semid = semget(semkey, 3, 0);
    if (semid < 0) {
        perror("semget");
        return -1;
    }

    sembuf sem_consumer_up = { 0, 1, 0 };

    sembuf sem_memory_up = { 1, 1, 0 };

    sembuf producer_mem_down[2] = {{ 2, -1, 0 }, { 1, -1, 0 }};
    sembuf consumer_mem_down[2] = {{ 0, -1, 0 }, { 1, -1, 0 }};
    sembuf producer_mem_up[2] = {{ 2, (short)crate_ptr->capacity, 0 }, { 1, 1, 0 }};

    // now do the producing!!
    int sleep_time = rand() % 2 + 1;
    char buf[128];
    if (is_producer) {
        printf("Producer sleep time: %ds\n", sleep_time);
        while (true) {
            // wait to unlock producing and memory
            semop(semid, producer_mem_down, 2);
            printf("Producer %d producing!\n", getpid());
            auto written = sprintf(buf, "(%d) %s", getpid(), items[crate_ptr->state]);
            std::memcpy(crate_ptr->data + crate_ptr->state * MAX_CRATE_ITEM_SIZE, buf, MIN(written, MAX_CRATE_ITEM_SIZE));
            crate_ptr->state += 1;
            if (crate_ptr->state == crate_ptr->capacity) {
                // unlock consumer
                semop(semid, &sem_consumer_up, 1);
            }
            // unlock memory access
            semop(semid, &sem_memory_up, 1);
            sleep(sleep_time);
        }
    } else {
        while (true) {
            // wait for consumer unlock and memory access
            semop(semid, consumer_mem_down, 2);
            printf("Consuming crate data!\n\n");
            for (uint i = 0; i < crate_ptr->capacity; ++i) {
                std::memcpy(buf, crate_ptr->data + i * MAX_CRATE_ITEM_SIZE, MAX_CRATE_ITEM_SIZE);
                printf("Data: %s\n", buf);
                // increase producer semaphore to maximum capacity
            }
            crate_ptr->state = 0;
            // increase producer sem by capacity and unlock memory access
            semop(semid, producer_mem_up, 2);
        }
    }


    return 0;
}
