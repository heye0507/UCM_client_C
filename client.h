/* 
 * File:   client.h
 * Author: test
 *
 * Created on December 8, 2014, 12:44 PM
 */

#ifndef CLIENT_H
#define	CLIENT_H

//#define UCM_PORT "7777"
//#define UCM_IP "192.168.40.133"
#define MAXDATASIZE 256 //max data set to 5000

extern char username[21];
extern char password[21];
extern char UCM_PORT[6];
extern char UCM_IP[16];

void welcome_message();
//print out welcome message
//ask user to enter username and password
//consider adding a feature to set UCM IP and port later
int client();
//connecting to server
//send user command and receive server response 
//pass the response back to UI
int command(char *sendBuffer);
//display command list untill a right command is entered
//asking user to input command
//pack the command in the right order and store it in sendBuffer
//return 0 if success
//command list: login, listcommands
int display(char *recvBuffer,int flag);//flag currently not working
//display the response from server
//if flag set to 1, just log,
//if flag set to 0, do both
//log the response into log file
//return 0 if success

void display_menu();

int help_strip_newline(char *s);
//this is to strip the \n from the fgets()
//the string will be modified with \0 in the end of the s
//return 0 if success, else will return 1

int help_command_vaild(char *s);
//return 0 if the command user entered is not allowed by the UCM
//return 1 if it is a login action
//return 2 if it is a status,queuestatus,logoff action

int help_pack(char *s,int flag,char *sendBuffer);
//FIRST, it will CLEAN the sendBuffer
//login action flag is set to 1
//status queuestatus,logoff is set to 2
//After pack the data into sendBuffer, return 0 if not success

void temp_test();
#endif	/* CLIENT_H */

