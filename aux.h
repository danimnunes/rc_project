#ifndef AUX_H
#define AUX_H

void getCurrentTime(char timestr[20]);
int countAuctionDirectories(const char *path);
int verify_login_input(char buffer[]);
int verify_logout_input(char buffer[]);
int verify_uid(char uid[]);
int uid_pass_match(char uid[], char password[]);
int check_user_loggedin(char uid[], char password[]);
int verify_aid(char buffer[]);
int verify_open_input(char name[], char start_value[], char time_active[]);
char *getSize(char asset_name[], char buffer[]);
char *getData(char asset_name[], char buffer[]);
int requestMyAuctions();
int requestAuctionsBids();
int requestAuctions();
int checkUserExists();
int checkUserLogged();
int checkUserExistsLogged();
int checkMyAuctions();
int checkAuctionsBids();
int checkAuctions();
int detailedAuction();
int requestRecord();
int unregisterUsed();
int function();

#endif