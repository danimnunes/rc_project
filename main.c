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
struct sockaddr_in addr;
char buffer[128]; // buffer para onde serão escritos os dados recebidos do servidor
char current_uid[7];
char current_uid_ps[9];
char tcp_input[][11] = {"open", "close", "show_asset", "bid"};
/*char input_l[][4] = {"login", "logout", "myauctions", "mybids", "list"};*/
char input_r[][4] = {"RLI", "RLO", "RUR", "RMA", "RMB", "RLS", "RRC"};

void write_answer(char buffer[]){
    write(1, "echo: ", 6);
    write(1, buffer, strlen(buffer));
}

int analyse_answer(char status[], char buffer[]){
    char command[4], answer[4];
    sscanf(buffer, "%s %s", command, answer);
    return !(strcmp(answer, status));
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
                    }else if(analyse_answer("REG", buffer)){
                        write_answer("new user registered\n");
                    }
                    break;
                case 1: //logout/RLO
                    if(analyse_answer("NOK",buffer)){
                        write_answer("user not logged in\n");
                    }else if(analyse_answer("OK", buffer)){
                        write_answer("successful logout\n");
                    }else if(analyse_answer("UNR", buffer)){
                        write_answer("unknown user\n");
                    }
                    break;
                case 2: //unregister/RUR
                    if(analyse_answer("NOK",buffer)){
                        write_answer("incorrect unregister attempt\n");
                    }else if(analyse_answer("OK", buffer)){
                        write_answer("successful unregister\n");
                    }else if(analyse_answer("UNR", buffer)){
                        write_answer("unknown user\n");
                    }
                    break;
                case 3: //myauctions/RMA
                    if(analyse_answer("NOK",buffer)){
                        write_answer("user is not involved in any of the currently active auctions\n");
                    }else if(analyse_answer("OK", buffer)){
                        write_answer("supostamente agr listo as auctions\n"); //TO DO
                    }else if(analyse_answer("NLG", buffer)){
                        write_answer("user not logged in\n");
                    }
                    break;  
                case 4: //mybids/RMB
                    if(analyse_answer("NOK",buffer)){
                        write_answer("user has no active auction bids\n");
                    }else if(analyse_answer("OK", buffer)){
                        write_answer("agr listamos as bids supostamente\n"); //TO DO
                    }else if(analyse_answer("NLG", buffer)){
                        write_answer("user not logged in\n");
                    }
                    break;  
                case 5: //list/RLS
                    if(analyse_answer("NOK",buffer)){
                        write_answer(" no auctions are currently active\n");
                    }else if(analyse_answer("OK", buffer)){
                        write_answer("listar as auctions\n"); //TO DO
                    }
                    break;
                case 6: //show_record/RRC
                    if(analyse_answer("NOK",buffer)){
                        write_answer("that auction does not exist\n");
                    }else if(analyse_answer("OK", buffer)){
                        write_answer("mostrar cenas\n"); //TO DO
                    }
                    break;

                // TCP ....................................
                case 7: //open/OPA
                    if(analyse_answer("NOK",buffer)){
                        write_answer("auction could not be started\n");
                    }else if(analyse_answer("OK", buffer)){
                        write_answer("mostrar AID\n"); //TO DO
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
                        write_answer("fechar\n"); //TO DO
                    }else if(analyse_answer("NLG", buffer)){
                        write_answer("user not logged in\n");
                    }
                    break;
                case 9: //show_asset/SAS
                    if(analyse_answer("NOK",buffer)){
                        write_answer("no file to be sent, or some other problem\n");
                    }else if(analyse_answer("OK", buffer)){
                        write_answer("mostrar ficheiro\n"); //TO DO
                    }
                    break;
                case 10: //bid/BID
                    if(analyse_answer("NOK",buffer)){
                        write_answer("auction is not active\n");
                    }else if(analyse_answer("ACC", buffer)){
                        write_answer("bid was accepted\n"); //TO DO
                    }else if(analyse_answer("REF", buffer)){
                        write_answer("bid was refused because a larger bid has already been placed previously\n"); //TO DO
                    }else if(analyse_answer("ILG", buffer)){
                        write_answer("user tried to make a bid in an auction hosted by himself\n"); //TO DO
                    }else if(analyse_answer("NLG", buffer)){
                        write_answer("user not logged in\n");
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

void send_message(char buffer[], size_t bytes){
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
    errcode = getaddrinfo("tejo.tecnico.ulisboa.pt", PORT, &hints, &res);
    if (errcode != 0) {
        printf("error in getaddrinfo");
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
    n = recvfrom(fd, buffer, 128, 0, (struct sockaddr *)&addr, &addrlen);
    if (n == -1) {
        exit(1);
    }
    translate_answer(buffer);

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
            sscanf(buffer, "%s\t%s\t%s", command, current_uid, current_uid_ps);
            send_message(message, 20);
        }
    }
    else if(strcmp(command, "logout") == 0){ 
        strcpy(message, "LOU");
        strcat(message, " ");
        strcat(message, current_uid);
        strcat(message, " ");
        strcat(message, current_uid_ps);
        strcat(message, "\n");
        send_message(message, 20);
    }
    else if(strcmp(command, "unregister") == 0){ 
        strcpy(message, "UNR");
        strcat(message, " ");
        strcat(message, current_uid);
        strcat(message, " ");
        strcat(message, current_uid_ps);
        strcat(message, "\n");
        send_message(message, 20);
    }
    else if(strcmp(command, "myauctions") == 0 || strcmp(command, "ma") == 0 ){ 
        strcpy(message, "LMA");
        strcat(message, " ");
        strcat(message, current_uid);
        strcat(message, "\n");
        send_message(message, 11);
    }
    else if(strcmp(command, "mybids") == 0 || strcmp(command, "mb") == 0 ){ 
        strcpy(message, "LMB");
        strcat(message, " ");
        strcat(message, current_uid);
        strcat(message, "\n");
        send_message(message, 11);
        
    }
    else if(strcmp(command, "list") == 0 || strcmp(command, "l") == 0 ){ 
        strcpy(message, "LST");
        send_message(message, 4);
    }
    else if(strcmp(command, "show_record") == 0 || strcmp(command, "sr") == 0 ){ 
        strcpy(message, "SRC");
        //strcat(message, buffer + strlen(command));
        send_message(message, 8);
    }
    else {
        perror("invalid input");
        exit(EXIT_FAILURE);
    }
    
    
}


/*int check_tcp(char buffer[]) {

    char dest[4];  

    strncpy(dest, buffer, 3);
    dest[3] = '\0';  // Null-terminate the destination string

    for (size_t i = 0; i < sizeof(tcp_input) / sizeof(tcp_input[0]); ++i) {
        if (strcmp(dest, tcp_input[i]) == 0) {
            return 1; // Match found
        }
    }

    return 0; // No match found
}*/ 
//tive de fazer uma funçao nova porque os tcp_inputs já nao têm todos o mesmo tamanho, mas nao sei se a nova está certa

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

    char command[20];  // Tamanho suficiente para armazenar a palavra
    sscanf(buffer, "%s", command);

    if(strcmp(command, "open") == 0){ 
        strcpy(buffer, "OPA");
        strcat(buffer, buffer + strlen(command)); 
        function(buffer);
        send_message(buffer, 20);
    }
    else if(strcmp(command, "close") == 0){ 
        strcpy(buffer, "CLS");
        strcat(buffer, buffer + strlen(command)); 
        function(buffer);
        send_message(buffer, 20);
    }
    else if(strcmp(command, "show_asset") == 0 || strcmp(command, "sa") == 0 ){ 
        strcpy(buffer, "SAS");
        strcat(buffer, buffer + strlen(command)); 
        function(buffer);
        send_message(buffer, 20);
    }
    else if(strcmp(command, "bid") == 0 || strcmp(command, "b") == 0 ){ 
        strcpy(buffer, "BID");
        strcat(buffer, buffer + strlen(command)); 
        function(buffer);
        send_message(buffer, 20);
    }
    else {
        perror("invalid input");
        exit(EXIT_FAILURE);
    }
}


int main() {
    fd_set inputs, newfds;
    int max_fd = STDIN_FILENO;


    // podemos ler so a primeira letra e depois comparar com o array dessa letra, para reduzir as comparações, ou comparar com todas simplesmente


    while(1) {

        FD_ZERO(&inputs); 
        FD_SET(max_fd, &inputs);
        newfds = inputs;


        if (select(max_fd + 1, &newfds, NULL, NULL, NULL) == -1) {

            perror("select");
            exit(EXIT_FAILURE);
        }

        // Check if stdin is ready for reading
        if (FD_ISSET(STDIN_FILENO, &newfds)) {

            char buffer[1024];

            // Use fgets to read a line from stdin
            if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
                perror("fgets");
                exit(EXIT_FAILURE);
            }
            //strcat (buffer, "\n");
            if (check_tcp(buffer)) {
                tcp_action(buffer);
            }
            else {
                udp_action(buffer);
            }
            
        }
        
    }

}