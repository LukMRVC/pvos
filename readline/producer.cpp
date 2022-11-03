#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>
#include <fcntl.h>
#include <cerrno>
#include <cstring>

const char *pipe_name = "./piiiipeeee";


int read_line_select(int fd, void *t_data, int t_len, int t_out_ms) {
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(fd, &read_set);

    timeval tout = {0, t_out_ms * 1000};
    int data_move = 0;
    char *data = (char *) t_data;
    while (true) {
        int sel = select(fd + 1, &read_set, nullptr, nullptr, &tout);
        if (sel == 0) {
            errno = ETIME;
            return -1;
        }
        if (data_move >= t_len) return data_move;
        int ret = read(fd, data + data_move, 1);
        if (ret == 0) {
            if (data_move == 0) return 0;
            return data_move;
        }
        if (ret < 0) return ret;
        if (data[data_move++] == '\n') {
            return data_move;
        }
    }
}

int run_producer() {
    int ret = mkfifo(pipe_name, 0600);
    if (ret < 0) {
        if (errno != EEXIST) {
            perror("mkfifo");
            return 1;
        }
    }
    srand(time(nullptr));
    int pfd = open(pipe_name, O_WRONLY);
    if (pfd < 0) {
        perror("open");
        return -1;
    }
    char buffer[512];
    std::memset(buffer, 0, 512);
    unsigned int len_of_pid = 0;
    int n = getpid();
    int base = 10;
    do {
        ++len_of_pid;
        n /= base;
    } while (n);
    while (true) {
        int sum = getpid();
        sprintf(buffer, "%d ", getpid());
        int num_of_digits = rand() % 30;
        for (int i = 0; i < num_of_digits; ++i) {
            int rnd = rand() % 10;
            sum += rnd;
            sprintf(buffer + len_of_pid + (i * 2) + 1, "%d ", rnd);
        }

        int falsy = rand() % 100 + 1;
        if (falsy > 60) {
            sum += 1;
        }
        int cur_len = strlen(buffer);
        sprintf(buffer + len_of_pid + num_of_digits * 2 + 1, "%d\n", sum);
//        *(buffer + len_of_pid + num_of_digits * 2 + 1) = '\n';
//        write(1, buffer, strlen(buffer));
        write(pfd, buffer, strlen(buffer));
        std::memset(buffer, 0, 512);
    }

}

int run_consumer() {
    int pfd = open(pipe_name, O_RDONLY);
    if (pfd < 0) {
        perror("open");
        return 1;
    }
    char buffer[512];
    while (true) {
        std::memset(buffer, 0, 512);
        int _read = read_line_select(pfd, buffer, sizeof(buffer), 4000);
        if (_read < 0) {
            printf("Consumer timed out!");
        } else {
            printf("Consumer getting: %s", buffer);
            int sum = 0;
            int current_num;
            int offset = 0;
            int bytes_read;
            while (sscanf(buffer + offset, "%d%n", &current_num, &bytes_read) > 0) {
                sum += current_num;
                offset += bytes_read;
                if (bytes_read <= 0) {
                    break;
                }
            }
            // remove last num
            sum -= current_num;
            if (sum != current_num) {
                fprintf(stderr, "Sum is not correct! %d != %d\n", sum, current_num);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "Choose role [producer|consumer]!");
        return 1;
    }
    if (std::strcmp(argv[1], "producer") == 0) {
        return run_producer();
    }
    if (std::strcmp(argv[1], "consumer") == 0) {
        return run_consumer();
    }

    return 0;
}