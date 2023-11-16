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

#define PORT "59001"
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


int main() {
    fd_set inputs, newfds;
    int max_fd = STDIN_FILENO;
    char possible_inputs[][4] = {"LIN", "RLI", "LOU", "RLO", "UNR", "RUR", "LMA", "RMA", "LMB", "RMB", "LST", "RLS", "SRC", "RRC"};
    char input_l[][4] = {"LIN", "LOU", "LMA", "LMB", "LST"};
    char input_r[][4] = {"RLI", "RLO", "RUR", "RMA", "RMB", "RLS", "RRC"};

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
            // Assume buffer contains some data
            if (strncmp(buffer, "L", 1) == 0) {
                // Determine the action based on the matched string
                for (int i = 0; i < sizeof(input_l) / sizeof(input_l[0]); i++) {
            // Using strncmp to compare the first three characters of buffer with compareStrings[i]
                    switch (i) {
                        case 0: //LIN
                            loginUser();
                            break;
                        case 1: //LOU
                            logoutUser();
                            break;
                        case 2: //LMA
                            requestMyAuctions();
                            break;
                        case 3: //LMB
                            requestAuctionsBids();
                            break;  
                        case 4: //LST
                            requestAuctions();
                            break;  
                        // Add more cases for other matches

                        default:
                            perror("invalid input");
                            exit(EXIT_FAILURE);}
                            // Handle default case if needed
                            break;
                    }
                }
            

            else if (strncmp(buffer, "R", 1) == 0) {
                // Determine the action based on the matched string
                for (int i = 0; i < sizeof(input_r) / sizeof(input_r[0]); i++) {
            // Using strncmp to compare the first three characters of buffer with compareStrings[i]
                    switch (i) {
                        case 0: //RLI
                            checkUserExists();
                            break;
                        case 1: //RLO
                            checkUserLogged();
                            break;
                        case 2: //RUR
                            checkUserExistsLogged();
                            break;
                        case 3: //RMA
                            checkMyAuctions();
                            break;
                        case 4: //RMB
                            checkAuctionsBids();
                            break;
                        case 5: //RLS
                            checkAuctions();
                            break;
                        case 6: //RRC
                            detailedAuction();
                            break;
                        // Add more cases for other matches

                        default:
                            perror("invalid input");
                            exit(EXIT_FAILURE);}
                            // Handle default case if needed
                            break;
                }
            }
            
            //starts with other character than L or R
            else if(strncmp(buffer, "SRC", 3) == 0){ 
                requestRecord();
            }
            else if(strncmp(buffer, "UNR", 3) == 0){ 
                unregisterUsed();
                
            }else{
                perror("invalid input");
                exit(EXIT_FAILURE);
            }
        }
            

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

    /* Busca informação do host "localhost", na porta especificada,
    guardando a informação nas `hints` e na `res`. Caso o host seja um nome
    e não um endereço IP (como é o caso), efetua um DNS Lookup. */
    errcode = getaddrinfo(IP, PORT, &hints, &res);
    if (errcode != 0) {
        exit(1);
    }

    /* Envia para o `fd` (socket) a mensagem "Hello!\n" com o tamanho 7.
       Não são passadas flags (0), e é passado o endereço de destino.
       É apenas aqui criada a ligação ao servidor. */
    n = sendto(fd, "Hello!\n", 7, 0, res->ai_addr, res->ai_addrlen);
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

    /* Imprime a mensagem "echo" e o conteúdo do buffer (ou seja, o que foi recebido
    do servidor) para o STDOUT (fd = 1) */
    write(1, "echo: ", 6);
    write(1, buffer, n);

    /* Desaloca a memória da estrutura `res` e fecha o socket */
    freeaddrinfo(res);
    close(fd);
}