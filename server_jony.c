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
#include <sys/stat.h>

#define PORT "58011"
char port[5]="58011";
int udp_fd;
int tcp_fd;
ssize_t n;
socklen_t udp_addrlen;
struct addrinfo udp_hints, *udp_res;
struct sockaddr_in udp_addr;
char buffer[128];
int verbose_mode=0;


int createLogin(char *uid){
    uid[strcspn(uid, "\n")] = 0;
    char login_name[35];
    FILE *fp;
    printf("uid::::%s\tstrlen::::%ld\n", uid, strlen(uid));
    if(strlen(uid)!=6){
        puts("strlen");
        return 0;
    }

    sprintf(login_name, "USERS/%s/%s_login.txt", uid, uid);
    fp=fopen(login_name, "w");
    if(fp==NULL){
        puts("fp");
        return 0;
    }
    fprintf(fp, "Logged in\n");
    fclose(fp);
    return 1;
}

int eraseLogin(char *uid){
    char login_name[35];

    if(strlen(uid)!=6){
        return 0;
    }

    sprintf(login_name, "USERS/%s/%s_login.txt", uid, uid);
    unlink(login_name);
    return 1;
}

int registerUid(char *uid, char *password){
    uid[strcspn(uid, "\n")] = 0;
    char login_name[35], pass_name[35];
    char uid_dirname[17];
    int ret;
    FILE *fp, *fp_pass;
    printf("uid::::%s\tstrlen::::%ld\n", uid, strlen(uid));
    if(strlen(uid)!=6){
        puts("strlen");
        return 0;
    }

    sprintf(uid_dirname, "USERS/%06s", uid);

    ret = mkdir(uid_dirname, 0700);
    if(ret == -1){
        puts("mkdir1");
        return 0;
    }

    sprintf(login_name, "USERS/%s/%s_login.txt", uid, uid);
    fp=fopen(login_name, "w");
    if(fp==NULL){
        puts("fp when registering");
        return 0;
    }
    fprintf(fp, "Logged in\n");
    puts("just wrote");

    sprintf(pass_name, "USERS/%s/%s_pass.txt", uid, uid);
    fp_pass=fopen(pass_name, "w");
    if(fp_pass==NULL){
        puts("fp");
        return 0;
    }
    fprintf(fp_pass, "%s", password);

    fclose(fp);
    fclose(fp_pass);
    return 1;
}

int unregisterUid(char *uid){
    char login_name[35], pass_name[35], uid_dirname[17];
    int ret;

    if(strlen(uid)!=6){
        return 0;
    }

    sprintf(login_name, "USERS/%s/%s_login.txt", uid, uid);
    unlink(login_name);
    sprintf(pass_name, "USERS/%s/%s_pass.txt", uid, uid);
    unlink(pass_name);
    
    sprintf(uid_dirname, "USERS/%s", uid);

    ret = rmdir(uid_dirname);
    if(ret == -1){
        puts("mkdir1");
        return 0;
    }

    return 1;
}

int checkAssetFile(char *fname){
    struct stat filestat;
    int ret_stat;

    ret_stat=stat(fname, &filestat);

    if(ret_stat==-1 || filestat.st_size==0)
        return 0;
    return (filestat.st_size);
}

int createAuction(int aid){
    char aid_dirname[15];
    char bids_dirname[20];
    int ret;
    printf("aid:::%d\n", aid);
    if (aid < 1 || aid > 999)
    {
        puts("aid");
        return 0;
    }

    sprintf(aid_dirname, "AUCTIONS/%03d", aid);

    ret = mkdir(aid_dirname, 0700);
    if(ret == -1){
        puts("mkdir1");
        return 0;
    }

    sprintf(bids_dirname, "AUCTIONS/%03d/BIDS", aid);

    ret = mkdir(bids_dirname, 0700);
    if(ret==-1){
        rmdir(aid_dirname);
        puts("mkdir2");
        return 0;
    }
    return 1;
    
}

void udp_setup(){
    // UDP setup
    if ((udp_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    memset(&udp_hints, 0, sizeof udp_hints);

    udp_hints.ai_family = AF_INET;
    udp_hints.ai_socktype = SOCK_DGRAM;
    udp_hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port, &udp_hints, &udp_res) != 0) {
        perror("getaddrinfo");
        exit(1);
    }

    if (bind(udp_fd, udp_res->ai_addr, udp_res->ai_addrlen) == -1) {
        perror("bind");
        exit(1);
    }

    freeaddrinfo(udp_res);
}

void tcp_setup(){
    // TCP setup
    if ((tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in tcp_addr;
    memset(&tcp_addr, 0, sizeof(tcp_addr));
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_addr.s_addr = INADDR_ANY;
    tcp_addr.sin_port = htons(atoi(port));

    if (bind(tcp_fd, (struct sockaddr*)&tcp_addr, sizeof(tcp_addr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(tcp_fd, SOMAXCONN) == -1) {
        perror("listen");
        exit(1);
    }
}

void server() {

    fd_set all_fds_read;
    int max_fd;

    while (1) {
        FD_ZERO(&all_fds_read);
        FD_SET(udp_fd, &all_fds_read);
        FD_SET(tcp_fd, &all_fds_read);
        max_fd = (udp_fd > tcp_fd) ? udp_fd : tcp_fd; // o chat diz pra fazer como está, mas antes tinhamos : udp_fd + tcp_fd;

        if (select(max_fd + 1, &all_fds_read, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(1);
        }

        if (FD_ISSET(udp_fd, &all_fds_read)) {
            udp_addrlen = sizeof(udp_addr);

            if ((n = recvfrom(udp_fd, buffer, sizeof(buffer), 0, (struct sockaddr *)&udp_addr, &udp_addrlen)) == -1) {
                perror("recvfrom");
                exit(1);
            }
            
            printf("[UDP] Received: %.*s\n", (int)n, buffer);

            if (sendto(udp_fd, buffer, n, 0, (struct sockaddr *)&udp_addr, udp_addrlen) == -1) {
                perror("sendto");
                exit(1);
            }
        }

        if (FD_ISSET(tcp_fd, &all_fds_read)) {
            struct sockaddr_in client_addr;
            socklen_t client_addrlen = sizeof(client_addr);
            int tcp_socket = accept(tcp_fd, (struct sockaddr*)&client_addr, &client_addrlen);
            
            if (tcp_socket == -1) {
                perror("accept");
                exit(1);
            }

            n = read(tcp_socket,buffer, 128);
            if(n==-1)exit(1);

            // Aqui você pode adicionar código para processar as mensagens recebidas do cliente TCP
            // Provavelmente, você quer criar uma função ou chamada específica para lidar com as operações de login, logout, etc.
            // Certifique-se de adicionar o novo socket à sua lista de sockets TCP, se necessário.

            close(tcp_socket); // Certifique-se de fechar o socket após o processamento.
        }

        // Placeholder for handling TCP messages
        // You need to implement the code for handling data from connected TCP sockets
        // Loop through your list of connected TCP sockets and check FD_ISSET for each

    }

    // Close the UDP socket
    close(udp_fd);
    close(tcp_fd);
}

int main(int argc, char *argv[]) {
    if (argc == 3){
        if (strcmp(argv[1], "-p") == 0){
            strcpy(port, argv[2]);
            if(atoi(port) < 0 || atoi(port) > 65535){
                exit(1);
            }
        } else {
        puts("Something went wrong with input.");
        exit(1);
        }
    } else if (argc == 2){
        if (strcmp(argv[1], "-v") == 0){
            verbose_mode=1;
        } else {
        puts("Something went wrong with input.");
        exit(1);
        }
    } else if (argc == 4){
        if (strcmp(argv[1], "-p") == 0 && strcmp(argv[3], "-v")){
            strcpy(port, argv[2]);
            if(atoi(port) < 0 || atoi(port) > 65535){
                exit(1);
            }
            verbose_mode=1;
        } else {
        puts("Something went wrong with input.");
        exit(1);
        }
    }
    char inputtt[128];
    memset(inputtt, 0, sizeof(inputtt));
    // Use fgets to read a line from stdin
    if (fgets(inputtt, sizeof(inputtt), stdin) == NULL) {
        perror("fgets");
        exit(EXIT_FAILURE);
    }
    char uid[10], password[10];
    sscanf(inputtt, "%s %s\n", uid, password);
    if(unregisterUid(uid)) {
        puts("well done");
    } else { puts("dumb.");}
    udp_setup();
    tcp_setup();
    server();
    return 0;
}
