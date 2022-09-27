#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <ctime>

const int PARAM_NUM = 2;

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


int main(int argc, char **argv) {
    if (argc != PARAM_NUM) {
        printf("The number of params must be exactly: %d!\n", PARAM_NUM);
        printf("Params: [process_count]\n");
        exit(EXIT_FAILURE);
    }
    int process_count = std::stoi(argv[1]);
    int * child_pids = new int[process_count];
    printf("Process count to start: %d\n", process_count);
    int pid;
    for (int i = 0; i < process_count; ++i) {
        pid = fork();
        if (!pid) {
            // child process exiting loop
            break;
        }
        child_pids[i] = pid;
    }

    srand(getpid());
    // only child process gets into if branch
    if (!pid) {
        int rnd = rand() % 2000;
        sleep(2);
        do_random_exit(rnd, true);
        // just to be suer
        return 0;
    }

    while (true) {
        auto child_id = get_child_exit_status(0, true);
        if (child_id == -1 && errno == ECHILD) {
            break;
        }
    }

    printf("Now doing the second job, making a child process pool!\n");
    printf("IT'S A POOL PARTYYY!!!\n");

    const int max_party_threshold = 30;
    int running_childs = 0;
    int created = 0;
    int finished = 0;
    auto last_time = time(nullptr);
    while (true) {
        auto current_time = time(nullptr);
        running_childs++;
        created++;
        if (fork() == 0) {
            srand(getpid());
            int sleep_time = rand() % max_party_threshold + 7;
//            printf("Child %d sleeping for %d\n", getpid(), sleep_time);
            sleep(sleep_time);
            do_random_exit(sleep_time + rand() % 2000);
            return 0;
        }
        usleep(100000);
        int child_id = 0;
        while (true) {
            child_id = get_child_exit_status(WNOHANG);
            if (child_id <= 0) {
                break;
            }

            if (child_id > 0) {
                finished++;
                running_childs--;
            }
        }

        if (child_id < 0) {
            break;
        }
        if (difftime(current_time, last_time) >= 1) {
            last_time = time(nullptr);
            printf("Created %d processes and awaited %d processes, running %d\n", created, finished, running_childs);
            created = 0;
            finished = 0;
        }
    }

    return 0;
}
