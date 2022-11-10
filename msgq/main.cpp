#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <cerrno>
#include <cstring>

#define MAX_MSG_SIZE 30

mqd_t open_create_queue(const char *name, int oflag, mode_t mode, struct mq_attr *attr) {
    mqd_t desc;
// open the queue
    desc = mq_open(name, oflag, mode, attr);
    // create queue if not exists
    if (desc < 0 && errno == ENOENT) {
        desc = mq_open(name, oflag | O_CREAT, mode, attr);
    }
    return desc;
}


int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "Enter mode as argument! [consumer|producer]\n");
        return 1;
    }

    bool is_producer = strcmp(argv[1], "producer") == 0;
    int capacity = 5;
    const char *items[8] = {"jedna", "dva", "tri", "ctyri", "pet", "sest", "sedm", "osm"};
    struct mq_attr init_attrs;
    init_attrs.mq_maxmsg = capacity;
    init_attrs.mq_msgsize = MAX_MSG_SIZE;
    init_attrs.mq_curmsgs = 0;
    init_attrs.mq_flags = 0;

    mqd_t crate = open_create_queue("/msgqueue", O_RDWR, 0600, nullptr);
    mqd_t producers;
    mqd_t consumer;
    producers = open_create_queue("/msgqproducer", O_RDWR, 0600, nullptr);
    consumer = open_create_queue("/msgqconsumer", O_RDWR, 0600, nullptr);

    mq_attr qattr;
    mq_getattr(crate, &qattr);
    printf("%ld\n", qattr.mq_msgsize);
    char msgbuf[qattr.mq_msgsize];
    unsigned int msgprio = 1;
    int curidx;

    if (argc == 3) {
        if (strcmp(argv[2], "init") == 0) {
            printf("Sending initial message\n");
            sprintf(msgbuf, "%d %d", (int)capacity, 0);
            // send initial message that is between producers only
            mq_send(producers, msgbuf, sizeof(msgbuf), msgprio);
        }
    }

    // one queue is for producers to send messages between each other
    // another queue is for producers - consumers to know when consumer should awaken
    // last queue is the crate itself, where the data is stored


    if (is_producer) {
        const int sleep_time = 2;
        while (true) {
            printf("waiting for producers message\n");
            int received = mq_receive(producers, msgbuf, sizeof(msgbuf), &msgprio);
            if (received < 0) {
                perror("mq_receive");
                return -1;
            }
            sscanf(msgbuf, "%d %d", &capacity, &curidx);
            printf("Read from producers queue: %d %d\n", capacity, curidx);
            sprintf(msgbuf, "(%d): %s", getpid(), items[curidx++]);
            printf("writing to crate: %s\n", msgbuf);
            int s = mq_send(crate, msgbuf, sizeof(msgbuf), msgprio);
            if (s < 0) {
                perror("mq_send");
                return -1;
            }
            if (curidx == capacity) {
                sprintf(msgbuf, "%d", capacity);
                // send msq to consumer
                printf("Sending data to consumer!\n");
                mq_send(consumer, msgbuf, sizeof(msgbuf), 1);
            } else {
                sprintf(msgbuf, "%d %d", capacity, curidx);
                mq_send(producers, msgbuf, sizeof(msgbuf), 1);
            }
            sleep(sleep_time);
        }
    } else {
        while (true) {
            printf("Receiving data from consumer queue\n");
            int received = mq_receive(consumer, msgbuf, sizeof (msgbuf), &msgprio);
            if (received < 0) {
                perror("mq_receive");
                return -1;
            }
            sscanf(msgbuf, "%d", &capacity);
            printf("Received: %d, reading crate\n", capacity);
            for (int i = 0; i < capacity; ++i) {
                mq_receive(crate, msgbuf, sizeof (msgbuf), &msgprio);
                printf("%s\n", msgbuf);
            }
            printf("Sending to producers\n");
            sprintf(msgbuf, "%d %d", capacity, 0);
            mq_send(producers, msgbuf, sizeof (msgbuf), 1);
        }
    }


    return 0;
}
