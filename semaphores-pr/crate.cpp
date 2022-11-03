#include <cstdio>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <cstring>
#include <string>
#include <sys/mman.h>
#define MAX_CRATE_ITEM_SIZE 20

struct crate {
    uint capacity;
    uint state;
    char *data[MAX_CRATE_ITEM_SIZE];
};

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "Add capacity parameter!\n");
        return -1;
    }
    uint capacity = (uint)std::atoi(argv[1]);
    int shmfd = shm_open("/crate", O_RDWR | O_TRUNC | O_CREAT, 0600);
    if (shmfd < 0) {
        perror("shm_open shmfd");
        return -1;
    }

    uint crate_struct_len = sizeof (uint) * 2 + capacity * MAX_CRATE_ITEM_SIZE;
    int res = ftruncate(shmfd, crate_struct_len);
    if (res < 0) {
        perror("ftruncate cfd");
        return -1;
    }

    crate * crate_ptr = (crate * ) mmap(nullptr, crate_struct_len, PROT_WRITE, MAP_SHARED, shmfd, 0);
    memset(crate_ptr, 0, crate_struct_len);
    crate_ptr->capacity = capacity;
    crate_ptr->state = 0;

    // create semaphores Shared mem
    int semfd = shm_open("/semaphores", O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (semfd < 0) {
        perror("shm_open semaphore");
        return -1;
    }

    res = ftruncate(semfd, sizeof (sem_t) * 2);
    if (res < 0) {
        perror("ftruncate semaphore semaphore");
        return -1;
    }

    sem_t* semptr = (sem_t * ) mmap(nullptr, sizeof (sem_t) * 3, PROT_WRITE, MAP_SHARED, semfd, 0);
    // initialize consumer semaphore
    sem_init(semptr, 1, 0);
    // initialize producers binary semaphore
    sem_init(semptr + sizeof (sem_t), 1, 1);
    // initialize producers capacity semaphore
    sem_init(semptr + sizeof (sem_t) * 2, 1, capacity);

    return 0;
}
