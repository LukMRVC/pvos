#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pthread.h>
#include <cstdlib>
#include <cstring>

void *read_file(void * filename) {
    char * file = (char *) filename;
    int fd = open(file, O_RDONLY);
    if (fd < 0) {
        perror("Open: ");
        return nullptr;
    }

    int content_size = 1028;
    char * content = new char [content_size];
    ssize_t total_read = 0;
    char buf[512];
    ssize_t bytes_read = 0;
    do {
        bytes_read = read(fd, buf, sizeof(buf));
        if (total_read + bytes_read > content_size) {
            content_size *= 3;
            content = (char *) realloc(content, content_size);
        }
        memcpy(content + total_read, buf, bytes_read);
        total_read += bytes_read;
    } while (bytes_read > 0);
    // terminate with 0
    content[total_read + 1] = 0;
    return (void *)content;
}

int main(int argc, char **argv) {
    if (argc <= 1) {
        printf("Program must have at least 1 argument!\n");
    }

    char * file_content;
    pthread_t join_handle;
    auto *handles = new pthread_t [argc - 1];
    for (int i = 1; i < argc; ++i) {
        char * filename = argv[i];
        pthread_create(&join_handle, nullptr, read_file, (void *)filename);
        handles[i - 1] = join_handle;
    }

    for (int i = 0; i < argc - 1; ++i ) {
        pthread_join(handles[i], (void **)&file_content);
        printf("%s", file_content);
        delete [] file_content;
    }

    delete [] handles;
}

