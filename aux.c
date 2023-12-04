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

int verify_open_input(char name[], char start_value[], char time_active[]){
    if(strlen(name)>10){
        printf("Invalid input: name size wrong\n");
        return 0;
    }
    for (size_t i=0; i<strlen(name);i++){
        if(!isdigit(name[i]) && !isalpha(name[i])){
            printf("Invalid input: name is alphanumeric\n");
            return 0;
        }
    } 

    if(strlen(start_value)>6){
        printf("Invalid input: start_value size wrong\n");
        return 0;
    }
    for (size_t i=0; i<strlen(start_value);i++){
        if(!isdigit(start_value[i])){
            printf("Invalid input: start_value is a digit\n");
            return 0;
        }
    } 

    if(strlen(time_active)>5){
        printf("Invalid input: time_active size wrong\n");
        return 0;
    }
    for (size_t i=0; i<strlen(time_active);i++){
        if(!isdigit(time_active[i])){
            printf("Invalid input: time_active is a digit\n");
            return 0;
        }
    } 
    return 1;
}

char *getSize(char asset_name[], char buffer[]) 
{ 
    // Store the content of the file
    char fsize_str[9];
    // opening the file in read mode 
    FILE* fp = fopen(asset_name, "r"); 
  
    // checking if the file exist or not 
    if (fp == NULL) { 
        printf("Asset Not Found!\n"); 
        return ""; 
    } 

    fseek(fp, 0L, SEEK_END); 
  
    // calculating the size of the file 
    long int fsize = ftell(fp); 
    sprintf(fsize_str, "%ld", fsize);
    strcat(buffer, fsize_str);
    // closing the file 
    fclose(fp); 
  
    return buffer; 
} 

char *getData(char asset_name[], char buffer[]) 
{ 
    // Store the content of the file
    char fdata[100];  //HERE aqui supostamnete a data pode ter ate 10 000 000 bytes, no entanto se puser esse valor da segmentation fault por isso esta 100 para já
    // opening the file in read mode 
    FILE* fp = fopen(asset_name, "r"); 
  
    // checking if the file exist or not 
    if (fp == NULL) { 
        printf("Asset Not Found!\n"); 
        return ""; 
    } 
    // Read the content and store it inside myString
    fgets(fdata, sizeof(fdata), fp);
    strcat(buffer, fdata);
    strcat(buffer, "\n");
  
    // closing the file 
    fclose(fp); 
  
    return buffer; 
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