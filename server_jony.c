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
#include "aux.h"

#define PORT "58011"
char port[5]="58011";
int udp_fd;
int tcp_fd;
ssize_t n;
socklen_t udp_addrlen;
struct addrinfo udp_hints, *udp_res;
struct sockaddr_in udp_addr;
char buffer[128];
char cmds_udp[][4] = {"LIN", "LOU", "UNR", "LMA", "LMB", "LST", "SRC"};
int verbose_mode=0;

void send_reply_udp(char buffer[], size_t n){
    if (sendto(udp_fd, buffer, n, 0, (struct sockaddr *)&udp_addr, udp_addrlen) == -1) {
        perror("sendto");
        exit(1);
    }
}

int createLogin(char *uid){
    uid[strcspn(uid, "\n")] = 0;
    char login_name[35];
    FILE *fp;
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
    if(strlen(uid)!=6){
        puts("strlen");
        return 0;
    }

    sprintf(uid_dirname, "USERS/%6s", uid);

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


// funcao para ver se a pass que esta no _pass.txt é a mesma introduzida pelo user
int uid_pass_match(char uid[], char password[]){
    char pass_name[35], pass_registered[10];
    FILE *fp;

    sprintf(pass_name, "USERS/%s/%s_pass.txt", uid, uid);
    fp=fopen(pass_name, "r");
    fgets(pass_registered, 10, fp);

    return !strcmp(pass_registered, password);

}


/////////////////   UDP   ////////////////////////////////////////////////////////////////////

void login_cmd(char uid[], char password[]){
    struct stat stats;
    char path[13];
    sprintf(path, "USERS/%s", uid);
    stat(path, &stats);

    if (S_ISDIR(stats.st_mode)){ //ver se a diretoria com do uid existe
        // existe, temos de ver se o login já está feito
        char login_name[35];
        sprintf(login_name, "USERS/%s/%s_login.txt", uid, uid);
        if(uid_pass_match(uid, password)){   
            if (access(login_name, F_OK) == -1){
             
                if(createLogin(uid)){
                    send_reply_udp("RLI OK\n", 8);
                    return;
                }
            } else { //o login já tava feito , mandamos ok na mesma? o tejo manda ok na mes
                send_reply_udp("RLI OK\n", 9);
                return;
            } 
        } else { //pass e uid nao combinam
            send_reply_udp("RLI NOK\n", 9);
            return;
        }
    } else { // temos de criar a diretoria para o uid
        if(registerUid(uid, password)){
            send_reply_udp("RLI REG\n", 9);
            return;
        }
    }     
    send_reply_udp("RLI ERR\n", 10); //sei que é só err, mas deixei o rli para sabermos de onde vem
}

void logout_cmd(char uid[], char password[]){
    struct stat stats;
    char path[13];
    sprintf(path, "USERS/%s", uid);
    stat(path, &stats);

    if (S_ISDIR(stats.st_mode)){ //ver se a diretoria com do uid existe
        // existe, temos de ver se o login já está feito
        char login_name[35];
        sprintf(login_name, "USERS/%s/%s_login.txt", uid, uid);
        if(uid_pass_match(uid, password)){   
            if (access(login_name, F_OK) == -1){ // login nao está feito
                send_reply_udp("RLO NOK\n", 9);
                return;
                
            } else { //o login tava feito , fazemos logout

                if (remove(login_name) == 0) {
                    send_reply_udp("RLO OK\n", 9);
                    return;
                } else {
                    send_reply_udp("RLO ERR\n", 10); // Erro ao remover o arquivo
                    return;
                }
            } 
        }
    } else { // user nao existe 
        send_reply_udp("RLO UNR\n", 9);
        return;
        
    }     
    send_reply_udp("RLO ERR\n", 10);
}

void unregister_cmd(char uid[], char password[]){
    struct stat stats;
    char path[13];
    sprintf(path, "USERS/%s", uid);
    stat(path, &stats);

    if (S_ISDIR(stats.st_mode)){ //ver se a diretoria com do uid existe
        // existe, temos de ver se o login já está feito
        char login_name[35], pass[35];
        sprintf(login_name, "USERS/%s/%s_login.txt", uid, uid);
        sprintf(pass, "USERS/%s/%s_pass.txt", uid, uid);
        if(uid_pass_match(uid, password)){   
            if (access(login_name, F_OK) == -1){ // login nao está feito
                send_reply_udp("UNR NOK\n", 9);
                return;
                
            } else { //o login tava feito , fazemos logout

                if (remove(login_name) == 0 && remove(pass) == 0) {
                    send_reply_udp("UNR OK\n", 9);
                    return;
                } else {
                    send_reply_udp("UNR ERR\n", 10); // Erro ao remover o arquivo
                    return;
                }
            } 
        }
    } else { // user nao existe 
        send_reply_udp("UNR UNR\n", 9);
        return;
        
    }     
    send_reply_udp("UNR ERR\n", 10);
}

void myauctions_cmd(char uid[]){
    send_reply_udp("RMA TEST\n", 10);
}

void mybids_cmd(char uid[]){
    send_reply_udp("RMB TEST\n", 10);
}

void list_cmd(){
    send_reply_udp("RLS TEST\n", 10);
}

void showrecord_cmd(char aid[]){
    send_reply_udp("RRC TEST\n", 10);
}




/////////////////   TCP   ////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////



void udp_message(char buffer[]){
    char command[6];
    sscanf(buffer, "%s", command);
    if(buffer[strlen(buffer)-1]!='\n'){
        send_reply_udp("ERR\n", 5);
    } 
    for (size_t i = 0; i < sizeof(cmds_udp) / sizeof(cmds_udp[0]); i++) {
        if(strncmp(cmds_udp[i], command, 3)==0){
            char uid[8], password[10], aid[5];
            switch (i) {
                case 0: //login
                    if(verify_login_input(buffer)){
                        sscanf(buffer, "%s\t%s\t%s", command, uid, password);
                        login_cmd(uid, password);
                    }else{
                        send_reply_udp("ERR\n", 5);
                    }// ou NOK ou assim tenho de ver
                    
                    break;
                case 1: //logout
                    if(verify_logout_input(buffer)){
                        sscanf(buffer, "%s\t%s\t%s", command, uid, password);
                        logout_cmd(uid, password);
                    }else{
                        send_reply_udp("ERR\n", 5);
                    }// ou NOK ou assim tenho de ver
                    
                    break;
                case 2: //unregister
                    if(verify_login_input(buffer)){
                        sscanf(buffer, "%s\t%s\t%s", command, uid, password);
                        unregister_cmd(uid, password);
                    }else{
                        send_reply_udp("ERR\n", 5);
                    }// ou NOK ou assim tenho de ver
                    
                    break;
                case 3: //myauctions
                    sscanf(buffer, "%s %s\n", command, uid);
                    myauctions_cmd(uid);
                    break;
                case 4: //mybids
                    sscanf(buffer, "%s %s\n", command, uid);
                    mybids_cmd(uid);
                    break;
                case 5: //list
                    list_cmd();
                    break;
                case 6: //show_record
                    sscanf(buffer, "%s %s\n", command, aid);
                    showrecord_cmd(aid);
                    break;
                default:
                    send_reply_udp("ERR\n", 5);
            }
        }
    }

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
            
            udp_message(buffer);
            printf("[UDP] Received: %.*s\n", (int)n, buffer);

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

void test(){
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
    // test();

    udp_setup();
    tcp_setup();
    server();
    return 0;
}
