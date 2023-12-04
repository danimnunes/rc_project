#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include "aux.h"


#define PORT "58011"
#define IP "193.136.138.142"
char port[5] = "58011", *ASIP = "tejo.tecnico.ulisboa.pt";
int fd, errcode;
ssize_t n;
socklen_t addrlen; // Tamanho do endereço
/*
hints - Estrutura que contém informações sobre o tipo de conexão que será estabelecida.
        Podem-se considerar, literalmente, dicas para o sistema operacional sobre como
        deve ser feita a conexão, de forma a facilitar a aquisição ou preencher dados.

res - Localização onde a função getaddrinfo() armazenará informações sobre o endereço.
*/
struct addrinfo hints, *res;
struct sockaddr_in addr; // buffer para onde serão escritos os dados recebidos do servidor
char current_uid[7] , aux_uid[7];
char current_uid_ps[9], aux_uid_ps[9];
char tcp_input[][11] = {"open", "close", "show_asset", "bid", "sa", "b"};
/*char input_l[][4] = {"login", "logout", "myauctions", "mybids", "list"};*/
char input_r[][4] = {"RLI", "RLO", "RUR", "RMA", "RMB", "RLS", "RRC", "ROA", "RCL", "RSA", "RBD"};
char cmds_sent[][4] = {"LIN", "LOU", "UNR", "LMA", "LMB", "LST", "SRC"};

void write_answer(char buffer[]){
    write(1, "echo: ", 6);
    write(1, buffer, strlen(buffer));
}

int analyse_answer(char status[], char buffer[]){
    char command[4], answer[4];
    sscanf(buffer, "%s %s", command, answer);
    return !(strcmp(answer, status));
}

int reply_matches(char sent[], char rcv[]){
    int matches=0;
    for(size_t i=0; i<sizeof(cmds_sent);i++){
        if(strcmp(sent, cmds_sent[i])){
            if(strcmp(rcv, input_r[i])){
                matches=1;
                break;
            }
        }
    }
    return matches;
}

void analyse_record(char buffer[]){
    char *token="", message[500000];
    token = strtok(buffer, " ");
    strcat(message, "Host UID-> ");

    strcat(message, token);

    strcat(message, "\nAuction name -> ");

    token = strtok(NULL, " ");
    strcat(message, token);
    strcat(message, "\nAsset file name -> ");

    token = strtok(NULL, " ");
    strcat(message, token);
    strcat(message, "\nStart value -> ");

    token = strtok(NULL, " ");
    strcat(message, token);
    strcat(message, "\nStart date -> ");
    token = strtok(NULL, " ");
    strcat(message, token);
    strcat(message, "\nStart time -> ");
    token = strtok(NULL, " ");
    strcat(message, token);
    strcat(message, "\nTime active -> ");

    token = strtok(NULL, " ");
    strcat(message, token);
    token = strtok(NULL, " ");

    int bid = 0;

    while (token != NULL) {
        // Process each token based on its type (e.g., 'RRC', 'B', 'E')

        if (strcmp(token, "B") == 0) {
            // Process bid information
            bid++;
            strcat(message, "\nBid info by UID: ");
        } else if(bid==1) {
            strcat(message, token);
            bid++;
        } else if(bid==2) {
            strcat(message, "\nbid_value->");
            strcat(message, token);
            bid++;
        } else if(bid==3) {
            strcat(message, " \tbid_date->");
            strcat(message, token);
            bid++;
        } else if(bid==4) {
            strcat(message, " \tbid_time->");
            strcat(message, token);
            bid++;
        } else if(bid==5) {
            strcat(message, " \tbid_sec_time->");
            strcat(message, token);
            bid++;
        } else if (strcmp(token, "E") == 0) {
            strcat(message, "\nAuction ending info: ");
            // Process end information
        } else {
            strcat(message, " ");
            strcat(message, token);
        }
        if(bid==6) bid=0;
        // Get the next token
        token = strtok(NULL, " ");
    }
    write_answer(message); 
}


void translate_answer(char buffer[]){
    char command[4];
    sscanf(buffer,"%s",command);
    // Determine the action based on the matched string
    if(strcmp(command, "ERR")==0){
        write_answer(buffer);
        return;
    }
    for (size_t i = 0; i < sizeof(input_r) / sizeof(input_r[0]); i++) {
        if(strncmp(input_r[i], command, 3)==0){
            switch (i) {
                // UDP .............................
                case 0: //login/RLI
                    if(analyse_answer("NOK",buffer)){
                        write_answer(" incorrect login attempt\n");
                    }else if(analyse_answer("OK", buffer)){
                        write_answer("successful login\n");
                        strcpy(current_uid, aux_uid);
                        strcpy(current_uid_ps, aux_uid_ps);
                    }else if(analyse_answer("REG", buffer)){
                        write_answer("new user registered\n");
                        strcpy(current_uid, aux_uid);
                        strcpy(current_uid_ps, aux_uid_ps);
                    }
                    break;
                case 1: //logout/RLO
                    if(analyse_answer("NOK",buffer)){
                        write_answer("user not logged in\n");
                    }else if(analyse_answer("OK", buffer)){
                        write_answer("successful logout\n");
                        memset(current_uid, 0, sizeof(current_uid));
                        memset(current_uid_ps, 0, sizeof(current_uid_ps));
                    }else if(analyse_answer("UNR", buffer)){
                        write_answer("unknown user\n");
                    }
                    break;
                case 2: //unregister/RUR
                    if(analyse_answer("NOK",buffer)){
                        write_answer("incorrect unregister attempt\n");
                    }else if(analyse_answer("OK", buffer)){
                        write_answer("successful unregister\n");
                        memset(current_uid, 0, sizeof(current_uid));
                        memset(current_uid_ps, 0, sizeof(current_uid_ps));
                    }else if(analyse_answer("UNR", buffer)){
                        write_answer("unknown user\n");
                    }
                    break;
                case 3: //myauctions/RMA
                    if(analyse_answer("NOK",buffer)){
                        write_answer("user is not involved in any of the currently active auctions\n");
                    }else if(analyse_answer("OK", buffer)){
                        char message[6001];   
                        memset(message, 0, sizeof(message));
                        strcat(message, buffer + 7); 
                        write_answer(message); 
                    }else if(analyse_answer("NLG", buffer)){
                        write_answer("user not logged in\n");
                    }
                    break;  
                case 4: //mybids/RMB
                    if(analyse_answer("NOK",buffer)){
                        write_answer("user has no ongoing bids\n");
                    }else if(analyse_answer("OK", buffer)){
                        char message[6001];   
                        memset(message, 0, sizeof(message));
                        strcat(message, buffer + 7); 
                        write_answer(message); 
                    }else if(analyse_answer("NLG", buffer)){
                        write_answer("user not logged in\n");
                    }
                    break;  
                case 5: //list/RLS
                    if(analyse_answer("NOK",buffer)){
                        write_answer(" no auctions are currently active\n");
                    }else if(analyse_answer("OK", buffer)){
                        char message[6001];   
                        memset(message, 0, sizeof(message));
                        strcat(message, buffer + 7); 
                        write_answer(message); 
                    }
                    break;
                case 6: //show_record/RRC
                    if(analyse_answer("NOK",buffer)){
                        write_answer("that auction does not exist\n");
                    }else if(analyse_answer("OK", buffer)){
                        analyse_record(buffer+7); 
                    }
                    break;

                // TCP ....................................
                case 7: //open/OPA
                    if(analyse_answer("NOK",buffer)){
                        write_answer("auction could not be started\n");
                    }else if(analyse_answer("OK", buffer)){
                        char command[4], answer[4], aid[4], message[34];
                        memset(message, 0, sizeof(message));
                        sscanf(buffer, "%s %s %s", command, answer, aid);
                        strcat(message, "Assigned auction identifier:");
                        strcat(message, " "); 
                        strcat(message, aid);
                        strcat(message, "\n");
                        write_answer(message); 
                    }else if(analyse_answer("NLG", buffer)){
                        write_answer("user not logged in\n");
                    }
                    break;
                case 8: //close/CLS
                    if(analyse_answer("EAU",buffer)){
                        write_answer("that auction does not exist\n");
                    }else if(analyse_answer("EOW", buffer)){
                        write_answer("auction is not owned by user\n");
                    }else if(analyse_answer("END", buffer)){
                        write_answer("auction has already finished.\n");
                    }else if(analyse_answer("OK", buffer)){
                        write_answer("auction successfully closed\n"); 
                    }else if(analyse_answer("NLG", buffer)){
                        write_answer("user not logged in\n");
                    }
                    break;
                case 9: //show_asset/SAS
                    if(analyse_answer("NOK",buffer)){
                        write_answer("no file to be sent, or some other problem\n");
                    }else if(analyse_answer("OK", buffer)){
                        char command[4], answer[4], name[25], size[9], message[38];
                        memset(name, 0, sizeof(name));
                        memset(size, 0, sizeof(size));
                        memset(message, 0, sizeof(message));
                        sscanf(buffer, "%s %s %s %s", command, answer, name, size);
                        strcat(message, name);
                        strcat(message, " ");
                        strcat(message, size);
                        strcat(message, "\n");
                        write_answer(message); 
                    }
                    break;
                case 10: //bid/BID
                    if(analyse_answer("NOK",buffer)){
                        write_answer("auction is not active\n");
                    }else if(analyse_answer("ACC", buffer)){
                        write_answer("bid was accepted\n"); 
                    }else if(analyse_answer("REF", buffer)){
                        write_answer("bid was refused because a larger bid has already been placed previously\n"); 
                    }else if(analyse_answer("ILG", buffer)){
                        write_answer("you cannot make a bid in an auction hosted by yourself\n"); 
                    }else if(analyse_answer("NLG", buffer)){
                        write_answer("you must login first\n");
                    }
                    break;


                default:
                    write_answer(buffer);
                    // Handle default case if needed
                    break;
            }
        }
    }

}

void communication_tcp(char buffer[]){
    char buffer2[5000];
    memset(buffer2, 0, sizeof(buffer2));
    char cmd_sent[4], cmd_rcv[4];
    for(int i=0; i<3; i++){
        cmd_sent[i]=buffer[i];
    }
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        puts("Error socket.");
        exit(1);
        
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM; // TCP socket

    errcode = getaddrinfo(ASIP, port, &hints, &res);
    if (errcode != 0) {
        puts("Error info.");
        exit(1);
    }
    /* Em TCP é necessário estabelecer uma ligação com o servidor primeiro (Handshake).
       Então primeiro cria a conexão para o endereço obtido através de `getaddrinfo()`. */
    n = connect(fd, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        puts("Error connecting to server.");
        exit(1);
    }
    /* Escreve a mensagem para o servidor, especificando o seu tamanho */
    n=write(fd, buffer,strlen(buffer));
    if (n == -1) {
        puts("Error writing to server.");
        exit(1);
    }

    /* Lê 128 Bytes do servidor e guarda-os no buffer. */
    char buffer3[1000000];
    memset(buffer3, 0, sizeof(buffer3));
    while(n != 0) {
        n=read(fd, buffer2, 5000);
        strcat(buffer3, buffer2);
    }
    if (n == -1) {
        puts("Error reading from server.");
        exit(1);
    }
    for(int i=0; i<3; i++){
        cmd_rcv[i]=buffer3[i];
    }
    if(reply_matches(cmd_sent, cmd_rcv)){
        translate_answer(buffer3);
    }else{
        puts("something went wrong communicating with server");
    }
                            
    /* Desaloca a memória da estrutura `res` e fecha o socket */
    freeaddrinfo(res);
    close(fd);
}

void communication_udp(char buffer[], size_t bytes){
    char buffer2[6001];
    memset(buffer2, 0, sizeof(buffer2));
    char cmd_sent[4], cmd_rcv[4];
    memset(cmd_sent, '\0', sizeof(cmd_sent));
    memset(cmd_rcv, '\0', sizeof(cmd_rcv));
    for(int i=0; i<3; i++){
        cmd_sent[i]=buffer[i];
    }
    /* Cria um socket UDP (SOCK_DGRAM) para IPv4 (AF_INET).
    É devolvido um descritor de ficheiro (fd) para onde se deve comunicar. */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        exit(1);
    }

    /* Preenche a estrutura com 0s e depois atribui a informação já conhecida da ligação */
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP socket

    /* Busca informação do host "tejo.tecnico.ulisboa.pt", na porta especificada,
    guardando a informação nas `hints` e na `res`. Caso o host seja um nome
    e não um endereço IP (como é o caso), efetua um DNS Lookup. */
    errcode = getaddrinfo(ASIP, port, &hints, &res);
    if (errcode != 0) {
        puts("error in getaddrinfo");
        exit(1);
    }

    /* Envia para o `fd` (socket) a mensagem "Hello!\n" com o tamanho 20.
       Não são passadas flags (0), e é passado o endereço de destino.
       É apenas aqui criada a ligação ao servidor. */
    n = sendto(fd, buffer, bytes, 0, res->ai_addr, res->ai_addrlen);
    if (n == -1) {
        exit(1);
    }

    /* Recebe 128 Bytes do servidor e guarda-os no buffer.
       As variáveis `addr` e `addrlen` não são usadas pois não foram inicializadas. */
    addrlen = sizeof(addr);
    n = recvfrom(fd, buffer2, 6001, 0, (struct sockaddr *)&addr, &addrlen);
    if (n == -1) {
        puts("error in recvfrom");
        exit(1);
    }



    for(int i=0; i<3; i++){
        cmd_rcv[i]=buffer2[i];
    }
    if(reply_matches(cmd_sent, cmd_rcv)){
        translate_answer(buffer2);
    }else{
        puts("something went wrong communicating with server");
    }

    /* Desaloca a memória da estrutura `res` e fecha o socket */
    freeaddrinfo(res);
    close(fd);
}

void udp_action(char buffer[]) {
    char message[20];
    char command[20];  // Tamanho suficiente para armazenar a palavra
    sscanf(buffer, "%s", command);
        
    if(strcmp(command, "login") == 0){ 
        strcpy(message, "LIN");
        strcat(message, buffer + strlen(command)); //modificar login para LIN e adicionar ao resto do conteudo 
        if(verify_login_input(buffer)){
            sscanf(buffer, "%s\t%s\t%s", command, aux_uid, aux_uid_ps);
            communication_udp(message, 20);
        }
    }
    else if(strcmp(command, "logout") == 0){ 
        if(strlen(current_uid)==6){
            strcpy(message, "LOU");
            strcat(message, " ");
            strcat(message, current_uid);
            strcat(message, " ");
            strcat(message, current_uid_ps);
            strcat(message, "\n");
            communication_udp(message, 20);
        }else{puts("You must login first.");}

    }
    else if(strcmp(command, "exit") == 0){ 
        if(strlen(current_uid)==6){
            puts("You must logout first.");
        }

    }
    else if(strcmp(command, "unregister") == 0){ 
        if(strlen(current_uid)==6){
            strcpy(message, "UNR");
            strcat(message, " ");
            strcat(message, current_uid);
            strcat(message, " ");
            strcat(message, current_uid_ps);
            strcat(message, "\n");
            communication_udp(message, 20);
        }else{puts("You must login first.");}
    }   
    else if(strcmp(command, "myauctions") == 0 || strcmp(command, "ma") == 0 ){ 
        if(strlen(current_uid)==6){
            strcpy(message, "LMA");
            strcat(message, " ");
            strcat(message, current_uid);
            strcat(message, "\n");
            communication_udp(message, 11);
        }else{puts("You must login first.");}
    }
    else if(strcmp(command, "mybids") == 0 || strcmp(command, "mb") == 0 ){ 
        if(strlen(current_uid)==6){
            strcpy(message, "LMB");
            strcat(message, " ");
            strcat(message, current_uid);
            strcat(message, "\n");
            communication_udp(message, 11);
        }else{puts("You must login first.");}
        
    }
    else if(strcmp(command, "list") == 0 || strcmp(command, "l") == 0 ){ 
        strcpy(message, "LST");
        strcat(message, "\n");
        communication_udp(message, 4);
    }
    else if(strcmp(command, "show_record") == 0 || strcmp(command, "sr") == 0 ){ 
        strcpy(message, "SRC");
        strcat(message, buffer + strlen(command));
        if(verify_aid(buffer)){
            communication_udp(message, 8);
        }
    }
    else {
        perror("invalid input");
    }
    
    
}



int check_tcp(char buffer[]) {
    size_t buffer_len = strlen(buffer);

    for (size_t i = 0; i < sizeof(tcp_input) / sizeof(tcp_input[0]); ++i) {
        size_t input_len = strlen(tcp_input[i]);

        // Verifica se o tamanho da palavra no buffer é igual ao tamanho do input
        if (buffer_len >= input_len && strncmp(buffer, tcp_input[i], input_len) == 0) {
            return 1; // Correspondência encontrada
        }
    }

    return 0; // Nenhuma correspondência encontrada
}

void tcp_action(char buffer[]) {
    char message[100000];
    char command[20];  // Tamanho suficiente para armazenar a palavra
    memset(message, 0, sizeof(message));
    memset(command, 0, sizeof(command));
    sscanf(buffer, "%s", command);

    if(strcmp(command, "open") == 0){ 
        char name[11], asset_name[25], start_value[7], time_active[6];

        if(strlen(current_uid)==6){
            sscanf(buffer, "%s %s %s %s %s", command, name, asset_name, start_value, time_active);
            if(verify_open_input(name, start_value, time_active)){
                strcpy(message, "OPA");
                strcat(message, " ");
                strcat(message, current_uid);
                strcat(message, " ");
                strcat(message, current_uid_ps);
                strcat(message, " ");
                strcat(message, name); 
                strcat(message, " ");
                strcat(message, start_value);
                strcat(message, " ");
                strcat(message, time_active);
                strcat(message, " ");
                strcat(message, asset_name);
                strcat(message, " ");
                strcpy(message, getSize(asset_name, message));
                strcat(message, " ");
                strcpy(message, getData(asset_name, message));
                communication_tcp(message);
            }
        } else {
            puts("You most login first.");
        }
    }
    else if(strcmp(command, "close") == 0){ 

        if(strlen(current_uid)==6){
            if(verify_aid(buffer)) {
            strcpy(message, "CLS");
            strcat(message, " ");
            strcat(message, current_uid);
            strcat(message, " ");
            strcat(message, current_uid_ps);
            strcat(message, buffer + strlen(command));
            communication_tcp(message);
            }else{puts("invalid AID");}
        }else{puts("You must login first.\n");}
        

        
    }
    else if(strcmp(command, "show_asset") == 0 || strcmp(command, "sa") == 0 ){ 
        
        if(verify_aid(buffer)) {
            strcpy(message, "SAS");
            strcat(message, buffer + strlen(command)); 
            communication_tcp(message);
        }else{puts("invalid AID");}
        
    }
    else if(strcmp(command, "bid") == 0 || strcmp(command, "b") == 0 ){ 
        if(strlen(current_uid)==6){
            strcpy(message, "BID");
            strcat(message, " ");
            strcat(message, current_uid);
            strcat(message, " ");
            strcat(message, current_uid_ps);
            strcat(message, buffer + strlen(command));
            communication_tcp(message);
        } else {
            puts("You must login first.\n");
            
        }    
    }
    else {
        perror("invalid input");
    }
}


int main(int argc, char *argv[]) {
    fd_set inputs, newfds;
    int max_fd = STDIN_FILENO;
    if (argc == 3){
        if (strcmp(argv[1], "-n") == 0){
            ASIP = (char *)malloc(strlen(argv[2]) + 1);
            strcpy(ASIP, argv[2]);
        } else if (strcmp(argv[1], "-p") == 0){
            strcpy(port, argv[2]);
            if(atoi(port) < 0 || atoi(port) > 65535){
                exit(1);
            }
        }
    }
    else if(argc == 5){
        ASIP = (char *)malloc(strlen(argv[2]) + 1);
        strcpy(ASIP, argv[2]);
        strcpy(port, argv[4]);
        if(atoi(port) < 0 || atoi(port) > 65535){
            exit(1);
        }
    }
    // podemos ler so a primeira letra e depois comparar com o array dessa letra, para reduzir as comparações, ou comparar com todas simplesmente
    memset(current_uid, 0, sizeof(current_uid));
    memset(current_uid_ps, 0, sizeof(current_uid_ps));
    memset(aux_uid, 0, sizeof(aux_uid));
    memset(aux_uid_ps, 0, sizeof(aux_uid_ps));
    while(1) {

        FD_ZERO(&inputs); 
        FD_SET(max_fd, &inputs);
        newfds = inputs;
        char inputtt[1024];


        if (select(max_fd + 1, &newfds, NULL, NULL, NULL) == -1) {

            perror("select");
            exit(EXIT_FAILURE);
        }

        // Check if stdin is ready for reading
        if (FD_ISSET(STDIN_FILENO, &newfds)) {

            memset(inputtt, 0, sizeof(inputtt));
            // Use fgets to read a line from stdin
            if (fgets(inputtt, sizeof(inputtt), stdin) == NULL) {
                perror("fgets");
                exit(EXIT_FAILURE);
            }
            //strcat (buffer, "\n");
            if (check_tcp(inputtt)) {
                tcp_action(inputtt);
            }
            else {
                udp_action(inputtt);
            }
            
        }
        
    }

}