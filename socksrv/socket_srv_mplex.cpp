//***************************************************************************
//
// Program example for labs in subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2017
//
// Example of socket server.
//
// This program is example of socket server and it allows to connect and serve
// the only one client.
// The mandatory argument of program is port number for listening.
//
//***************************************************************************

#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <cstdarg>
#include <poll.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <vector>

#define STR_CLOSE   "close"
#define STR_QUIT    "quit"

//***************************************************************************
// log messages

#define LOG_ERROR               0       // errors
#define LOG_INFO                1       // information and notifications
#define LOG_DEBUG               2       // debug messages

// debug flag
int g_debug = LOG_INFO;

void log_msg(int t_log_level, const char *t_form, ...) {
    const char *out_fmt[] = {
            "ERR: (%d-%s) %s\n",
            "INF: %s\n",
            "DEB: %s\n"};

    if (t_log_level && t_log_level > g_debug) return;

    char l_buf[1024];
    va_list l_arg;
    va_start(l_arg, t_form);
    vsprintf(l_buf, t_form, l_arg);
    va_end(l_arg);

    switch (t_log_level) {
        case LOG_INFO:
        case LOG_DEBUG:
            fprintf(stdout, out_fmt[t_log_level], l_buf);
            break;

        case LOG_ERROR:
            fprintf(stderr, out_fmt[t_log_level], errno, strerror(errno), l_buf);
            break;
    }
}

//***************************************************************************
// help

void help(int t_narg, char **t_args) {
    if (t_narg <= 1 || !strcmp(t_args[1], "-h")) {
        printf(
                "\n"
                "  Socket server example.\n"
                "\n"
                "  Use: %s [-h -d] port_number\n"
                "\n"
                "    -d  debug mode \n"
                "    -h  this help\n"
                "\n", t_args[0]);

        exit(0);
    }

    if (!strcmp(t_args[1], "-d"))
        g_debug = LOG_DEBUG;
}

int read_data_from_client(int fd);

//***************************************************************************

int main(int t_narg, char **t_args) {
    if (t_narg <= 1) help(t_narg, t_args);

    int l_port = 0;

    // parsing arguments
    for (int i = 1; i < t_narg; i++) {
        if (!strcmp(t_args[i], "-d"))
            g_debug = LOG_DEBUG;

        if (!strcmp(t_args[i], "-h"))
            help(t_narg, t_args);

        if (*t_args[i] != '-' && !l_port) {
            l_port = atoi(t_args[i]);
            break;
        }
    }

    if (l_port <= 0) {
        log_msg(LOG_INFO, "Bad or missing port number %d!", l_port);
        help(t_narg, t_args);
    }

    log_msg(LOG_INFO, "Server will listen on port: %d.", l_port);

    // socket creation
    int l_sock_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (l_sock_listen == -1) {
        log_msg(LOG_ERROR, "Unable to create socket.");
        exit(1);
    }

    in_addr l_addr_any = {INADDR_ANY};
    sockaddr_in l_srv_addr;
    l_srv_addr.sin_family = AF_INET;
    l_srv_addr.sin_port = htons(l_port);
    l_srv_addr.sin_addr = l_addr_any;

    // Enable the port number reusing
    int l_opt = 1;
    if (setsockopt(l_sock_listen, SOL_SOCKET, SO_REUSEADDR, &l_opt, sizeof(l_opt)) < 0)
        log_msg(LOG_ERROR, "Unable to set socket option!");

    // assign port number to socket
    if (bind(l_sock_listen, (const sockaddr *) &l_srv_addr, sizeof(l_srv_addr)) < 0) {
        log_msg(LOG_ERROR, "Bind failed!");
        close(l_sock_listen);
        exit(1);
    }

    // listenig on set port
    if (listen(l_sock_listen, 1) < 0) {
        log_msg(LOG_ERROR, "Unable to listen on given port!");
        close(l_sock_listen);
        exit(1);
    }

    log_msg(LOG_INFO, "Enter 'quit' to quit server.");

    std::vector<pollfd> clients;
    pollfd s_in;
    s_in.fd = STDIN_FILENO;
    s_in.events = POLLIN;

    pollfd sock_srv_poll;
    sock_srv_poll.fd = l_sock_listen;
    sock_srv_poll.events = POLLIN;
    clients.emplace_back(s_in);
    clients.emplace_back(sock_srv_poll);

    // go!
    while (true) {
        int l_sock_client = -1;

        while (true) // wait for new client
        {
            // select from fds
            int l_poll = poll(clients.data(), clients.size(), -1);

            if (l_poll < 0) {
                log_msg(LOG_ERROR, "Function poll failed!");
                exit(1);
            }

            if (clients[0].revents & POLLIN) { // data on stdin
                char buf[128];
                int len = read(STDIN_FILENO, buf, sizeof(buf));
                if (len < 0) {
                    log_msg(LOG_DEBUG, "Unable to read from stdin!");
                    exit(1);
                }

                log_msg(LOG_DEBUG, "Read %d bytes from stdin");
                // request to quit?
                if (!strncmp(buf, STR_QUIT, strlen(STR_QUIT))) {
                    log_msg(LOG_INFO, "Request to 'quit' entered.");
                    close(l_sock_listen);
                    exit(0);
                }
            }

            if (clients[1].revents & POLLIN) { // new client?
                sockaddr_in l_rsa;
                int l_rsa_size = sizeof(l_rsa);
                // new connection
                l_sock_client = accept(l_sock_listen, (sockaddr *) &l_rsa, (socklen_t *) &l_rsa_size);
                if (l_sock_client == -1) {
                    log_msg(LOG_ERROR, "Unable to accept new client.");
                    close(l_sock_listen);
                    exit(1);
                }
                uint l_lsa = sizeof(l_srv_addr);
                // my IP
                getsockname(l_sock_client, (sockaddr *) &l_srv_addr, &l_lsa);
                log_msg(LOG_INFO, "My IP: '%s'  port: %d",
                        inet_ntoa(l_srv_addr.sin_addr), ntohs(l_srv_addr.sin_port));
                // client IP
                getpeername(l_sock_client, (sockaddr *) &l_srv_addr, &l_lsa);
                log_msg(LOG_INFO, "Client IP: '%s'  port: %d",
                        inet_ntoa(l_srv_addr.sin_addr), ntohs(l_srv_addr.sin_port));

                pollfd n_client;
                n_client.fd = l_sock_client;
                n_client.events = POLLIN;
                clients.emplace_back(n_client);
            }

            for (int i = 2; i < clients.size(); ++i) {
                if (clients[i].revents & POLLIN) {
                    // data on some client
                    if (read_data_from_client(clients[i].fd) < 0) {
                        clients.erase(clients.begin() + i);
                    }
                    break;
                }
            }

        } // while wait for client
    } // while ( 1 )

    return 0;
}

int read_data_from_client(int fd) {
    static char l_buf[512];
    int l_len = read(fd, l_buf, sizeof(l_buf));
    if (!l_len) {
        log_msg(LOG_DEBUG, "Client closed socket!");
        close(fd);
        return -1;
    } else if (l_len < 0) {
        log_msg(LOG_DEBUG, "Unable to read data from client.");
    } else {
        log_msg(LOG_DEBUG, "Read %d bytes from client.", l_len);
        log_msg(LOG_DEBUG, "Data: %s", l_buf);
    }

    // close request?
    if (!strncasecmp(l_buf, "close", strlen(STR_CLOSE))) {
        log_msg(LOG_INFO, "Client sent 'close' request to close connection.");
        close(fd);
        log_msg(LOG_INFO, "Connection closed. Waiting for new client.");
        return -1;
    }

    int sum = 0;
    int current_num;
    int offset = 0;
    int bytes_read;
    while (sscanf(l_buf + offset, "%d%n", &current_num, &bytes_read) > 0) {
        sum += current_num;
        offset += bytes_read;
        if (bytes_read <= 0) {
            return -1;
        }
    }
    // remove last num
    sum -= current_num;
    sprintf(l_buf, "%s\n", sum == current_num ? "OK" : "ERR");
    l_len = strlen(l_buf);
    // write data to client
    l_len = write(fd, l_buf, l_len);
    if (l_len < 0) {
        log_msg(LOG_ERROR, "Unable to write data to stdout.");
    }
    memset(l_buf, 0, sizeof(l_buf));
    return 0;
}