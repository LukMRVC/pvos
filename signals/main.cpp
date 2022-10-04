#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <csignal>
#include <sys/types.h>

void sig_handler(int sig) {

}

int main() {

    int l_counter = 0;
    while (true) {
        printf("Counter: %d\n", l_counter++);
        sleep(1);
    }

    return 0;
}
