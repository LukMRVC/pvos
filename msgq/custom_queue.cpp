#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <string>
#include <ctime>
#include <cstring>


/**
 * This queue is sort of inspired by SysV msgbuf with types and text
 *
 */


struct Queue {
    uint front;
    uint rear;
    char data[1];
};

struct MQueue {
    size_t cap;
    size_t max_m_size;
    const char *qname = nullptr;
    sem_t *sem_full = nullptr;
    sem_t *sem_empty = nullptr;
    sem_t *sem_mutex = nullptr;

    int shmfd;
    Queue *msgdata;

    MQueue(const char *qname, size_t capacity, size_t max_msg_size) {
        cap = capacity;
        max_m_size = max_msg_size;
        this->qname = qname;
    }

    int open_shm(size_t mem_size) {
        int fd = shm_open(qname, O_RDWR, 0600);
        if (fd < 0 && errno == ENOENT) {
            fd = shm_open(qname, O_RDWR | O_TRUNC | O_CREAT, 0600);
            if (fd < 0) {
                perror("shm_open");
                return -1;
            }
            int res = ftruncate(fd, mem_size);
            if (res < 0) {
                perror("ftruncate");
                return -1;
            }
        }
        return fd;
    }

    int init_queue() {
        // 2 * sizeof for front and rear pointers
        shmfd = open_shm(cap * max_m_size + 2 * sizeof(uint));
        if (shmfd < 0) {
            return -1;
        }
        msgdata = (Queue *) mmap(nullptr, cap * max_m_size + 2 * sizeof(uint), PROT_READ | PROT_WRITE, MAP_SHARED,
                                 shmfd, 0);
        if (*((int *) msgdata) == -1) {
            return -1;
        }

        sem_full = sem_open((std::string(qname) + "_full").c_str(), O_RDWR, 0600, 1);
        if (sem_full == SEM_FAILED) {
            if (errno == ENOENT) {
                sem_full = sem_open((std::string(qname) + "_full").c_str(), O_RDWR | O_CREAT, 0600, 1);
                if (sem_full == SEM_FAILED) {
                    return -1;
                }
                if (sem_init(sem_full, 1, cap) < 0) {
                    return -1;
                }

                sem_empty = sem_open((std::string(qname) + "_empty").c_str(), O_RDWR | O_CREAT, 0600, 1);
                if (sem_empty == SEM_FAILED) {
                    return -1;
                }
                if (sem_init(sem_empty, 1, 0) < 0) {
                    return -1;
                }

                sem_mutex = sem_open((std::string(qname) + "_mutex").c_str(), O_RDWR | O_CREAT, 0600, 1);
                if (sem_mutex == SEM_FAILED) {
                    return -1;
                }
                if (sem_init(sem_mutex, 1, 1) < 0) {
                    return -1;
                }
            } else {
                return -1;
            }
        } else {
            sem_empty = sem_open((std::string(qname) + "_empty").c_str(), O_RDWR, 0600, 1);
            sem_mutex = sem_open((std::string(qname) + "_mutex").c_str(), O_RDWR, 0600, 1);
        }

        return 0;
    }

    void send(const char *msg) {
        sem_wait(sem_full);
        sem_wait(sem_mutex);
        // insert at queue_pos
        memcpy(msgdata->data + msgdata->rear * max_m_size, msg, max_m_size);
        msgdata->rear = (msgdata->rear + 1) % cap;
        // update_queue_pos
        sem_post(sem_mutex);
        sem_post(sem_empty);
    }

    void recv(char *msgbuf) {
        sem_wait(sem_empty);
        sem_wait(sem_mutex);
        memcpy(msgbuf, msgdata->data + msgdata->front * max_m_size, max_m_size);
        msgdata->front = (msgdata->front + 1) % cap;
        sem_post(sem_mutex);
        sem_post(sem_full);
    }
};


int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "Select one of: [producer|consumer]\n");
        return 1;
    }
    const char *items[8] = {"jedna", "dva", "tri", "ctyri", "pet", "sest", "sedm", "osm"};

    const int QUEUE_CAPACITY = 5;
    const int MAX_MSG_SIZE = 30;

    auto crate = MQueue("/msgqueue", QUEUE_CAPACITY, MAX_MSG_SIZE);
    if (crate.init_queue() < 0) {
        perror("init_queue");
        return 1;
    }

    auto producers = MQueue("/producers", QUEUE_CAPACITY, MAX_MSG_SIZE);
    if (producers.init_queue() < 0) {
        perror("init_queue");
        return 1;
    }

    auto consumers = MQueue("/consumers", QUEUE_CAPACITY, MAX_MSG_SIZE);
    if (consumers.init_queue() < 0) {
        perror("init_queue");
        return 1;
    }

    int curidx;
    int crate_capacity = 5;
    char msgbuf[MAX_MSG_SIZE];
    if (argc == 3) {
        if (strcmp(argv[2], "init") == 0) {
            printf("Sending initial message\n");
            sprintf(msgbuf, "%d %d", (int)crate_capacity, 0);
            producers.send(msgbuf);
        }
    }

    if (strcmp(argv[1], "producer") == 0) {
        const int sleep_time = 2;
        while (true) {
            printf("waiting for producers message\n");
            producers.recv(msgbuf);
            sscanf(msgbuf, "%d %d", &crate_capacity, &curidx);
            printf("Read from producers queue: %d %d\n", crate_capacity, curidx);
            sprintf(msgbuf, "(%d): %s", getpid(), items[curidx++]);
            printf("writing to crate: %s\n", msgbuf);
            crate.send(msgbuf);
            if (curidx == crate_capacity) {
                sprintf(msgbuf, "%d", crate_capacity);
                // send msq to consumer
                printf("Sending data to consumer!\n");
                consumers.send(msgbuf);
            } else {
                sprintf(msgbuf, "%d %d", crate_capacity, curidx);
                producers.send(msgbuf);
            }
            sleep(sleep_time);
        }
    } else if (strcmp(argv[1], "consumer") == 0) {
        while (true) {
            printf("Receiving data from consumer queue\n");
            consumers.recv(msgbuf);
            sscanf(msgbuf, "%d", &crate_capacity);
            printf("Received: %d, reading crate\n", crate_capacity);
            for (int i = 0; i < crate_capacity; ++i) {
                crate.recv(msgbuf);
                printf("%s\n", msgbuf);
            }
            printf("Sending to producers\n");
            sprintf(msgbuf, "%d %d", crate_capacity, 0);
            producers.send(msgbuf);
        }
    }

}