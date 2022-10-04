#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <errno.h>

int main() {
    char **args = new char * [2];
    args[0] = new char[5];
    args[1] = new char[12];
    strcpy(args[0], "ls");
    strcpy(args[1], "/home/luka");

    char result[258];

    auto ex = execvp(args[0], args);
    if (ex < 0) {
        sprintf(result, "Failure: %s\n", strerror(errno));
        printf("%s", result);
        perror("exec");
    }

}