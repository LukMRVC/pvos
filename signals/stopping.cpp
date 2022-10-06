#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>


int main() {
    const int children = 10;
    bool stopped[children];
    unsigned int child_pids[children];
    int p_pid;
    int i;
    int pressed = 0;

    for (i = 0; i < children; ++i) {
        stopped[i] = false;
        p_pid = fork();
        if (p_pid == 0) {
            break;
        }
        child_pids[i] = p_pid;
    }
    setvbuf(stdout, nullptr, _IONBF, 0);

    if (p_pid == 0) {
        while (true) {
            sleep(1 + 1 * i);
            printf("Child %d reporting\n", i);
        }
    } else {
        int child_status;
        pressed = getchar();
        // consume newline
        getchar();
        while (pressed != 'q') {
            pressed = pressed - '0';
            if (stopped[pressed]) {
                printf("SIGCONT children %d\n", child_pids[pressed]);
                kill(child_pids[pressed], SIGCONT);
                stopped[pressed] = false;
            } else {
                printf("SIGSTOP children %d\n", child_pids[pressed]);
                kill(child_pids[pressed], SIGSTOP);
                stopped[pressed] = true;
            }
            pressed = getchar();
            // consume newline
            getchar();
        }
        for (i = 0; i < children; ++i) {
            kill(child_pids[i], SIGKILL);
        }
        while (wait(&child_status) > 0);
    }

    return 0;
}