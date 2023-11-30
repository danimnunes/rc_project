#include "aux.h"
#include "string.h"
#include "stdio.h"
#include "ctype.h"

int verify_uid(char uid[]){
    if(strlen(uid)!=6){
        printf("Invalid input: uid size wrong\n");
        return 0;
    }
    for (size_t i=0; i<strlen(uid);i++){
        if(!isdigit(uid[i])){
            printf("Invalid input: uid has 6 digits\n");
            return 0;
        }
    } 
    return 1;
}

int verify_login_input(char buffer[]){

    char command[20], uid[8], password[10];
    sscanf(buffer, "%s\t%s\t%s", command, uid, password);
    
    if(strlen(password)!=8){
        printf("Invalid input: password has 8 digits or characters\n");
        return 0;
    }
    for (size_t i=0; i<strlen(password);i++){
        if(!isdigit(password[i]) && !isalpha(password[i])){
            printf("Invalid input: password\n");
            return 0;
        }
    }


    return verify_uid(uid);;
}


int verify_aid(char buffer[]){
    char command[20], aid[4];
    sscanf(buffer, "%s\t%s", command, aid);
    
    if(strlen(aid)!=3){
        printf("Invalid input: aid size wrong\n");
        return 0;
    }
    for (size_t i=0; i<strlen(aid);i++){
        if(!isdigit(aid[i])){
            printf("Invalid input: uid has 3 digits\n");
            return 0;
        }
    } 
    return 1;
}

/*
int verify_input(char buffer[]){

    char command[20], uid[8], password[10];
    sscanf(buffer, "%s\t%s\t%s", command, uid, password);
    
    if(strlen(password)!=8){
        printf("Invalid input: password has 8 digits or characters\n");
        return 0;
    }
    for (size_t i=0; i<strlen(password);i++){
        if(!isdigit(password[i]) && !isalpha(password[i])){
            printf("Invalid input: password\n");
            return 0;
        }
    }


    return verify_uid(uid);;
}*/

int requestMyAuctions(){
    return 0;
}
int requestAuctionsBids(){
    return 0;
}
int requestAuctions(){
    return 0;
}
int checkUserExists(){

    return 0;
}
int checkUserLogged(){
    return 0; 
}
int checkUserExistsLogged(){
    return 0;
}
int checkMyAuctions(){
    return 0;
}
int checkAuctionsBids(){
    return 0;
}
int checkAuctions(){
    return 0;
}
int detailedAuction(){
    return 0;
}
int requestRecord(){
    return 0;
}
int unregisterUsed(){
    return 0;
}
int function(){return 0;}
