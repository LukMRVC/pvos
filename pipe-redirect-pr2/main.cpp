#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdio>
#include <cstdlib>
#include <cctype>

int main() {
    int pipes[2];
    pipe(pipes);
    if (fork() == 0) {
        // child
        close(pipes[0]);

        // pipe is duplicated onto STDOUT
        dup2(pipes[1], STDOUT_FILENO);
        close(pipes[1]);
        execlp("ls", "ls", nullptr);

        printf("If here, something got wrong!...\n");
        return 0;
    }

    if (fork() == 0) {
        // child 2
        // close unused end of pipe
        close(pipes[1]);
        dup2(pipes[0], STDIN_FILENO);
        close(pipes[0]);
        execlp("tr", "tr", "a-z", "A-Z", nullptr);
        printf("If here, something is wrong...\n");
        /*while (true) {
            char buf[128];
            // read from pipe
            int len = read(pipes[0], buf, sizeof (buf));
            if (len <= 0) {
                break;
            }
            for (int i = 0; i < len; ++i) {
                buf[i] = toupper(buf[i]);
            }
            buf[len] = 0;
            // write to STDOUT
            write(STDOUT_FILENO, buf, len);
        }
        // close pipe end
        close(pipes[0]);*/
        return 0;
    }

    // parent
    close(pipes[0]);
    close(pipes[1]);

    wait(nullptr);
    wait(nullptr);
    close(pipes[0]);

    return 0;
}
