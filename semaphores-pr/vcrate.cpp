#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <string>
#include <sys/shm.h>
#include <sys/sem.h>
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
    auto key = ftok("/vcrate", 0xCAFE);
    uint crate_struct_data_len = MAX_CRATE_ITEM_SIZE * capacity;
    // create shared memory
//    create_shared_mem:
    int shmfdid = shmget(key, sizeof (uint) * 2 + crate_struct_data_len, IPC_CREAT | 0600);
    if (shmfdid < 0) {
        perror("shmget crate");
//        shmfdid = shmget(key, sizeof (uint) * 2 + crate_struct_data_len, 0);
//        if (errno == EINVAL) {
//            if (shmctl(shmfdid, IPC_RMID, nullptr) < 0) {
//                perror("shmctl: ");
//                return 1;
//            }
//            goto create_shared_mem;
//        }
        return 1;
    }


    crate * crate_ptr = (crate *) shmat(shmfdid, nullptr, 0);
    if (*((int *)(crate_ptr)) < 0) {
        perror("shmat");
        return -1;
    }
    // initialize values
    memset(crate_ptr, 0, crate_struct_data_len);
    crate_ptr->capacity = capacity;
    crate_ptr->state = 0;


    // create semaphores
    auto semkey = ftok("/vsem", 0xCAFE);
    auto semid = semget(semkey, 3, 0600 | IPC_CREAT);
    if (semid < 0) {
        perror("semget");
        return -1;
    }
    // initialize semaphores
    // initialize consumer semaphore
    semctl(semid, 0, SETVAL, 0);
    // initialize memory access semaphore
    semctl(semid, 1, SETVAL, 1);
    // initialize producers semaphore
    semctl(semid, 2, SETVAL, capacity);
    return 0;
}
