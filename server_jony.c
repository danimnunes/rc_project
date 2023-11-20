#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <sys/select.h>

#define PORT "58001"

int udp_fd;
int tcp_fd;
ssize_t n;
socklen_t udp_addrlen;
struct addrinfo udp_hints, *udp_res;
struct sockaddr_in udp_addr;
char buffer[128];

void udp_server() {

    if (socket(AF_INET, SOCK_DGRAM, 0) == -1) {
        exit(1);
    }

    memset(&udp_hints, 0, sizeof udp_hints);

    udp_hints.ai_family = AF_INET;
    udp_hints.ai_socktype = SOCK_DGRAM;
    udp_hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, PORT, &udp_hints, &udp_res) != 0) {
        exit(1);
    }

    if (bind(udp_fd, udp_res->ai_addr, udp_res->ai_addrlen) == -1) {
        exit(1);
    }

    fd_set all_fds_read;
    int max_fd_udp;

    while (1) {
        FD_ZERO(&all_fds_read);
        FD_SET(STDIN_FILENO, &all_fds_read); 
        FD_SET(udp_fd, &all_fds_read);
        FD_SET(tcp_fd, &all_fds_read);
        max_fd_udp = udp_fd;

        if (select(max_fd_udp + 1, &all_fds_read, NULL, NULL, NULL) < 0) {
            exit(1);
        }

        if (FD_ISSET(udp_fd, &all_fds_read)) {
            udp_addrlen = sizeof(udp_addr);

            if (recvfrom(udp_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&udp_addr, &udp_addrlen) == -1) {
                exit(1);
            }
            
            printf("[UDP] Received: %.*s\n", (int)n, buffer);

            if (sendto(udp_fd, buffer, n, 0, (struct sockaddr *)&udp_addr, udp_addrlen) == -1) {
                exit(1);
            }
        }

        // Placeholder for handling TCP connections
        if (FD_ISSET(tcp_fd, &all_fds_read)) {
            // Handle TCP input here
            // You will need to implement TCP connection handling
        }

        // Placeholder for handling stdin
        if (FD_ISSET(STDIN_FILENO, &all_fds_read)) {
            // Handle stdin input here
        }
    }
    freeaddrinfo(udp_res);
    close(udp_fd);
}

int main() {
    udp_server();
    return 0;
}
