#include <cstdlib>
#include <cstdio>
#include <vector>
#include <cstring>
#include <unistd.h>


struct FdBufferPair {
    int fd;
    char * buffer = nullptr;
    int offset = 0;
    size_t buffer_len = 0;
    ssize_t read_bytes = 0;

    ~FdBufferPair() {
        if (buffer != nullptr) {
            delete [] buffer;
            buffer = nullptr;
        }
    }
};


ssize_t readline(int fd, char * out_buffer, size_t count) {
    static std::vector<FdBufferPair*> fd_pairs;
    const int L_BUFFER_SIZE = 5;
    FdBufferPair * fd_pair = nullptr;
    for (auto & pair: fd_pairs) {
        if (fd == pair->fd) {
            fd_pair = pair;
            break;
        }
    }

    if (fd_pair == nullptr) {
        auto *p = new FdBufferPair { fd, nullptr, -1, L_BUFFER_SIZE };
        p->buffer = new char[L_BUFFER_SIZE];
        std::memset(p->buffer, 0, L_BUFFER_SIZE);
        fd_pair = p;
        fd_pairs.emplace_back(p);
    }
    ssize_t ret;
    if (fd_pair->offset == -1) {
        ret = read(fd, fd_pair->buffer, L_BUFFER_SIZE);
        if (ret < 0) {
            perror("read");
            return ret;
        }
        fd_pair->read_bytes = ret;
        fd_pair->offset = 0;
    }

    int starting_offset = fd_pair->offset;
    ssize_t line_len = 0;
    ssize_t copied = 0;
    while (line_len < count) {
        // if offset is bigger than whats in the inner buffer
        if (fd_pair->offset >= fd_pair->read_bytes) {
            // copy into out_buffer current relevant content of inner buffer
            std::memcpy(out_buffer + copied, fd_pair->buffer + starting_offset, fd_pair->offset - starting_offset);
            // read from fildes into inner buffer
            ret = read(fd_pair->fd, fd_pair->buffer, fd_pair->buffer_len);
            if (ret < 0) {
                return ret;
            }
            // increment current out_buffer offset - copied
            copied += fd_pair->offset - starting_offset;
            // reset offset, starting offset and set read_bytes
            fd_pair->read_bytes = ret;
            fd_pair->offset = 0;
            starting_offset = 0;
        }

        if (fd_pair->buffer[fd_pair->offset++] == '\n') {
            line_len++;
            break;
        }
        line_len++;
    }
    // copy into out_buffer from pair out_buffer offset - starting offset, including newline
    std::memcpy(out_buffer + copied, fd_pair->buffer + starting_offset, fd_pair->offset - starting_offset);
    out_buffer[copied + fd_pair->offset] = 0;
    return line_len;
}



int main() {
    printf("Hello, world!\n");
    char line[128];
    setvbuf(stdout, nullptr, _IONBF, 0);
    ssize_t _read = readline(STDIN_FILENO, line, sizeof (line));
    printf("Read: %d bytes: %s\n", _read, line);
    _read = readline(STDIN_FILENO, line, sizeof(line));
    printf("Read: %d bytes: %s\n", _read, line);
    _read = readline(STDIN_FILENO, line, sizeof(line));
    printf("Read: %d bytes: %s\n", _read, line);
    return 0;
}
