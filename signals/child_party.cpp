#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <sys/wait.h>
#include <unistd.h>

int running_childs = 0;
int created = 0;

void do_random_exit(int seed, bool with_ls = false) {
    if (seed < 600) {
        if (with_ls) {
            execlp("ls", "/tmp", nullptr);
        } else {
            char *p = nullptr;
            p[520] = 480;
            exit(EXIT_FAILURE);
        }
    } else if (seed >= 600 && seed < 1100) {
        if (with_ls) {
            execlp("ls", "/trv/nothing/eever", nullptr);
        } else {
            exit(EXIT_SUCCESS);
        }
    } else {
        float x = 5 / 0;
        printf("%d\n", x);
    }
}

int get_child_exit_status(int options, bool verbose = false) {
    int pidstatus = 0;
    pid_t child_pid = waitpid(-1, &pidstatus, options);
    if (child_pid == -1) {
        perror("WAITPID");
        return child_pid;
    }

    if (WIFEXITED(pidstatus) != 0 && verbose) {
        printf("Child process %d exited normally with status %d\n", child_pid, WEXITSTATUS(pidstatus));
    }

    if (WIFSIGNALED(pidstatus) != 0 && verbose) {
        printf("Child process %d terminated by signal %d\n", child_pid, WTERMSIG(pidstatus));
    }

    if (WIFSTOPPED(pidstatus) != 0 && verbose) {
        printf("Child process %d stopped by signal %d\n", child_pid, WSTOPSIG(pidstatus));
    }
    return child_pid;
}

void siging_handler(int t_sig) {
    static int collected = 0;
    static int signals_caught = 0;
    signals_caught++;
    int child_status;
    while (wait(&child_status) > 0) {
        running_childs--;
        collected++;
    }
    printf("Caught %d signals and collected %d child proceses\n", signals_caught, collected);
}


int main() {
    const int max_party_threshold = 30;
    int finished = 0;
    auto last_time = time(nullptr);

    struct sigaction l_sa;
    l_sa.sa_handler = siging_handler;
    l_sa.sa_flags = 0; // SA_RESTART
    sigemptyset(&l_sa.sa_mask);

    sigaction(SIGCHLD, &l_sa, nullptr);

    while (true) {
        running_childs++;
        created++;
        if (fork() == 0) {
            srand(getpid());
            int sleep_time = rand() % max_party_threshold + 7;
            sleep(sleep_time);
            do_random_exit(sleep_time + rand() % 2000);
            return 0;
        }
        usleep(100000);
        if (created % 10 == 0) {
            printf("Created %d processes, currently running %d\n", created, running_childs);
        }
    }
    return 0;
}