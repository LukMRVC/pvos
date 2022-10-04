#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>


int main() {
    int pid = 0;
    int child_status = 0;
    int processes_created = 1;
    do {
        pid = fork();
        processes_created += 1;
    } while (pid > 0);

    if (pid == 0) {
        sleep(10);
    } else {
        printf("Created %d processes with forking\n", processes_created);
        while (wait(&child_status) >= 0);
    }

    return 0;
}
