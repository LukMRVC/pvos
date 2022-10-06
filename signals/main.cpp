#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <csignal>
#include <sys/types.h>
#include <cerrno>
#include <cstring>

void ctrlc(int t_sig) {
    printf("Caught signal %d\n", t_sig);
}

int main() {

    struct sigaction l_sa;
    l_sa.sa_handler = ctrlc;
    l_sa.sa_flags = 0; // SA_RESTART
    sigemptyset(&l_sa.sa_mask);

    sigaction(SIGALRM, &l_sa, nullptr);
    char buf[512];
    int l_counter = 0;
    while (true) {
        printf("Counter: %d\n", l_counter++);
        alarm(10);
        ssize_t bytes_read = read(STDIN_FILENO, buf, sizeof (buf));
        if (bytes_read < 0) {
            printf("Read fault: %d %s\n", errno, strerror(errno));
        } else {
            write(STDOUT_FILENO, buf, bytes_read);
        }
    }

    return 0;
}
