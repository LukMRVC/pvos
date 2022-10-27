#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>
#include <fcntl.h>
#include <cerrno>

int main() {
    int pfd = mkfifo("./piiiipeeee", 0600);
    if (pfd < 0) {
        if (errno != EEXIST) {
            perror("mkfifo");
            return 1;
        }
    }
    srand(time(nullptr));


    return 0;
}