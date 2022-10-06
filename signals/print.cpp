#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>


int main () {
    int l_counter = 0;

    // with O_CREAT always add mode to file           here < 0644
    open("./novy_soubor", O_CREAT | O_RDWR, 0644);

    setvbuf(stdout, nullptr, _IONBF, 0);
    while ( true ) {
        printf("%d ", l_counter++);
        usleep(10000);
    }
}
