#include <cstdlib>
#include <cstdio>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <cerrno>


struct FdBufferPair {
    int fd;
    char * buffer = nullptr;
    int offset = 0;
    size_t buffer_len = 0;
    ssize_t read_bytes = 0;

    void reset() {
        offset = 0;
        read_bytes = 0;
        std::memset(buffer, 0, buffer_len);
    }

    ~FdBufferPair() {
        if (buffer != nullptr) {
            delete [] buffer;
            buffer = nullptr;
        }
    }
};


ssize_t readline_tout(int fd, char * out_buffer, size_t count, int t_out_ms = 2000) {
    fd_set read_set;
    FD_ZERO(&read_set);
    static std::vector<FdBufferPair*> fd_pairs;
    errno = 0;
    const int L_BUFFER_SIZE = 128;
    FdBufferPair * fd_pair = nullptr;

    for (auto & pair: fd_pairs) {
        if (fd == pair->fd) {
            fd_pair = pair;
            break;
        }
    }
    std::memset(out_buffer, 0, count);

    if (fd_pair != nullptr && out_buffer == nullptr) {
        fd_pair->reset();
        return 0;
    }

    if (fd_pair == nullptr) {
        auto *p = new FdBufferPair { fd, nullptr, -1, L_BUFFER_SIZE };
        p->buffer = new char[L_BUFFER_SIZE];
        std::memset(p->buffer, 0, L_BUFFER_SIZE);
        fd_pair = p;
        fd_pairs.emplace_back(p);
    }
    FD_SET(fd, &read_set);
    timeval tout = {0, t_out_ms * 1000};

    ssize_t ret;
    if (fd_pair->offset == -1) {
        int sel = select(fd + 1, &read_set, nullptr, nullptr, &tout);
        if (sel == 0) {
            errno = ETIME;
            return -1;
        }

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

            copied += fd_pair->offset - starting_offset;
            fd_pair->reset();
            // read from fildes into inner buffer
            int sel = select(fd + 1, &read_set, nullptr, nullptr, &tout);
            if (sel == 0) {
                errno = ETIME;
                return -1;
            }
            ret = read(fd_pair->fd, fd_pair->buffer, fd_pair->buffer_len);
            if (ret <= 0) {
                if (line_len > 0) {
                    errno = EAGAIN;
                    return -line_len;
                }
                return ret;
            }
            // increment current out_buffer offset - copied
            // reset offset, starting offset and set read_bytes
            fd_pair->read_bytes = ret;
            starting_offset = 0;
        }

        if (fd_pair->buffer[fd_pair->offset++] == '\n') {
            line_len++;
            break;
        }
        line_len++;
    }

    if (line_len >= count) {
        errno = EAGAIN;
        return -line_len;
    }

    // copy into out_buffer from pair out_buffer offset - starting offset, including newline
    std::memcpy(out_buffer + copied, fd_pair->buffer + starting_offset, fd_pair->offset - starting_offset);
    out_buffer[copied + fd_pair->offset] = 0;
    return line_len;
}



int main() {
    const int timeout_ms = 2000;
    char line[128];
    setvbuf(stdout, nullptr, _IONBF, 0);
    printf("timeout is %dms\n", timeout_ms);
    while (true) {
        ssize_t _read = readline_tout(STDIN_FILENO, line, sizeof (line), 2000);
        if (_read <= 0) {
            if (errno == ETIME) {
                printf("Timeouted\n");
                continue;
            }

            if (errno == EAGAIN) {
                printf("%s\n", line);
                continue;
            }

            printf("returned %d, breaking out\n", _read);
//            perror("readline");
            break;
        }
        printf("Read: %d bytes: %s", _read, line);
    }
//    ssize_t _read = readline_tout(STDIN_FILENO, line, sizeof (line));
//    printf("Read: %d bytes: %s\n", _read, line);
//    _read = readline_tout(STDIN_FILENO, line, sizeof(line));
//    printf("Read: %d bytes: %s\n", _read, line);
//    _read = readline_tout(STDIN_FILENO, line, sizeof(line));
//    printf("Read: %d bytes: %s\n", _read, line);
    return 0;
}
