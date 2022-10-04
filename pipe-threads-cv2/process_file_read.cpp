#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc <= 1) {
        printf("Program must have at least 1 argument!\n");
    }
    int child_status = 0;
    char buf[512];
    int fd = -1;
    int pipes[2];
    pipe(pipes);
    int i;
    int pid = 0;
    for (i = 1; i < argc; ++i) {
        pid = fork();
        if (!pid) {
            // break child out
            break;
        }
    }

    // parent close read end of pipe
    char byte[1] = {0};
    if (pid > 0) {
        close(pipes[0]);
        write(pipes[1], byte, 1);
        close(pipes[1]);
    }

    if (pid == 0) {
        read(pipes[0], &byte, 1);
        close(pipes[0]);
        char * filename = argv[i];
        fd = open(filename, O_RDONLY);
        if (fd < 0) {
            perror("Open: ");
            return -1;
        }
        while (read(fd, buf, sizeof(buf)) > 0) {
            printf("%s", buf);
        }
        close(fd);
        write(pipes[1], byte, 1);
        close(pipes[1]);
    } else {
        // parent while for children
        while(wait(&child_status) >= 0);
    }
    return 0;
}
