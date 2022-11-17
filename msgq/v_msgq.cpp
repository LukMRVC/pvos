#include <cstdio>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <cerrno>
#include <cstring>

#define MAX_MSG_SIZE 30

int open_create_queue(key_t key, int msgflag) {
    int ident;
// open the queue
    ident = msgget(key, msgflag);
    // create queue if not exists
    if (ident < 0 && errno == ENOENT) {
        ident = msgget(key, msgflag | IPC_CREAT);
    }
    return ident;
}


int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "Enter mode as argument! [consumer|producer]\n");
        return 1;
    }

    bool is_producer = strcmp(argv[1], "producer") == 0;
    int capacity = 5;
    const char *items[8] = {"jedna", "dva", "tri", "ctyri", "pet", "sest", "sedm", "osm"};

    int qflags = 0600;
    if (argc == 3 && strcmp(argv[2], "init") == 0) {
        qflags |= O_TRUNC;
    }
    int qcrate = open_create_queue(0xcafe, qflags);
    unsigned int consumer_type = 1;
    unsigned int producer_type = 2;
    unsigned int load_type = 3;
    int curidx;

    struct message {
        long type;
        char text[MAX_MSG_SIZE];
    } msg{};

    if (argc == 3) {
        if (strcmp(argv[2], "init") == 0) {
            printf("Sending initial message\n");
            msg.type = producer_type;
            sprintf(msg.text, "%d %d", (int) capacity, 0);
            // send initial message that is between producers only
            msgsnd(qcrate, (void *)&msg, sizeof (msg.text), 0);
        }
    }

    // one queue is for producers to send messages between each other
    // another queue is for producers - consumers to know when consumer should awaken
    // last queue is the crate itself, where the data is stored

    if (is_producer) {
        const int sleep_time = 2;
        while (true) {
            printf("waiting for producers message\n");
            ssize_t received = msgrcv(qcrate, (void *)&msg, sizeof(msg.text), producer_type, 0);
            if (received < 0) {
                perror("msgrcv");
                return -1;
            }
            sscanf(msg.text, "%d %d", &capacity, &curidx);
            printf("Read from producers queue: %d %d\n", capacity, curidx);
            sprintf(msg.text, "(%d): %s", getpid(), items[curidx++]);
            printf("writing to crate: %s\n", msg.text);
//            int s = mq_send(crate, msgbuf, sizeof(msgbuf), msgprio);
            // send message to crate
            msg.type = load_type;
            int s = msgsnd(qcrate, (void *)&msg, sizeof (msg.text), 0);
            if (s < 0) {
                perror("mq_send");
                return -1;
            }
            if (curidx == capacity) {
                msg.type = consumer_type;
                sprintf(msg.text, "%d", capacity);
                // send msq to consumer
                printf("Sending data to consumer!\n");
                msgsnd(qcrate, (void *)&msg, sizeof (msg.text), 0);
            } else {
                msg.type = producer_type;
                sprintf(msg.text, "%d %d", capacity, curidx);
                msgsnd(qcrate, (void *)&msg, sizeof (msg.text), 0);
            }
            sleep(sleep_time);
        }
    } else {
        while (true) {
            printf("Receiving data from consumer queue\n");
            ssize_t received = msgrcv(qcrate, (void *)&msg, sizeof(msg.text), consumer_type, 0);
            if (received < 0) {
                perror("msgrcv");
                return -1;
            }
            sscanf(msg.text, "%d", &capacity);
            printf("Received: %d, reading crate\n", capacity);
            for (int i = 0; i < capacity; ++i) {
                msgrcv(qcrate, (void *)&msg, sizeof(msg.text), load_type, 0);
                printf("%s\n", msg.text);
            }
            printf("Sending to producers\n");
            msg.type = producer_type;
            sprintf(msg.text, "%d %d", capacity, 0);
            msgsnd(qcrate, (void *)&msg, sizeof (msg.text), 0);
        }
    }
    return 0;
}
//
// Created by lukas on 10.11.22.
//
