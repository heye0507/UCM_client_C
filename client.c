#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>

#include "client.h"
#include "error.h"

char username[21]="hello";
char password[21]="12345";
char UCM_IP[16]="12345";
char UCM_PORT[6]="12345";

void temp_test()
{
    printf("%s\n",username);
    printf("%s\n",password);
}

int help_strip_newline(char* s)
{
    int len=strlen(s);
    if(len>0 && s[len-1]=='\n')
        s[len-1]='\0';
    else
        return 1;
    return 0;
}

int help_command_valid(char *s)
{
    if(strcmp(s,"login")==0)
        return 1;
    else if(strcmp(s,"status")==0)
        return 2;
    else if(strcmp(s,"queuestatus")==0)
        return 2;
    else if(strcmp(s,"logoff")==0)
        return 2;
    else if(strcmp(s,"ping")==0)
        return 2;
    else if(strcmp(s,"parkedcalls")==0)
        return 2;
    else if(strcmp(s,"sippeers")==0)
        return 2;
    else if(strcmp(s,"queuesummary")==0)
        return 2;
    else if(strcmp(s,"agents")==0)
        return 2;
    else if(strcmp(s,"dbget")==0)
        return 2;
    else
        return 0;
}

int help_pack(char *s,int flag,char *sendBuffer)
{
    memset(sendBuffer,0,strlen(sendBuffer));//clean the buffer
    if(flag==1){//this is login action, special care
        strcpy(sendBuffer,"action:");
        strcat(sendBuffer,s);
        strcat(sendBuffer,"\n");
        strcat(sendBuffer,"username:");
        strcat(sendBuffer,username);
        strcat(sendBuffer,"\n");
        strcat(sendBuffer,"secret:");
        strcat(sendBuffer,password);
        strcat(sendBuffer,"\n");
        strcat(sendBuffer,"\n");
        //noob way... but snprintf doesn't work... 
        
    }
    if(flag==2){
        strcpy(sendBuffer,"action:");
        strcat(sendBuffer,s);
        strcat(sendBuffer,"\n");
        strcat(sendBuffer,"\n");
    }
    
    return 0;
}

void welcome_message()
{
    printf("****************************************\n");
    printf("****Welcome to UCM DEMO BETA v1.0*******\n");
    printf("****************************************\n");
    printf("\nPlease enter UCM IP address: ");
    fgets(UCM_IP,sizeof(UCM_IP),stdin);
    if((help_strip_newline(UCM_IP))!=0)
    {
        fprintf(stderr,"Read UCM_IP fail\n");
        exit(3);
    }
    printf("\nPlease enter UCM PORT number: ");
    fgets(UCM_PORT,sizeof(UCM_PORT),stdin);
    if((help_strip_newline(UCM_PORT))!=0)
    {
        fprintf(stderr,"Read UCM_PORT fail\n");
        exit(3);
    }
    printf("\nPlease enter your username: ");
    fgets(username,sizeof(username),stdin);
    if((help_strip_newline(username))!=0)
    {
        fprintf(stderr,"Read username fail\n");
        exit(3);
    }
    printf("\nPlease enter your password: ");
    fgets(password,sizeof(password),stdin);
    if((help_strip_newline(password))!=0)
    {
        fprintf(stderr,"Read password fail\n");
        exit(3);
    }
    display_menu();
        
    
}

int client()
{
    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    /////////////////Connection Setup///////////////////////
    ////////////////////////////////////////////////////////
    int sockfd,numbytes,temp;
    int flag=0;//display() mode
    char recvBuffer[MAXDATASIZE],sendBuffer[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *res;
    
    memset(&hints,0,sizeof hints);
    hints.ai_family=AF_INET; //IPV4 only
    hints.ai_socktype=SOCK_STREAM;
    
    if((temp=getaddrinfo(UCM_IP,UCM_PORT,&hints,&servinfo))!=0)
    {
        fprintf(stderr,"Sanity check, getaddrinfo error\n");
        return GETADDR_ERROR;
    }
    
    for(res=servinfo;res!=NULL;res=res->ai_next)
    {
        if((sockfd=socket(res->ai_family,res->ai_socktype,res->ai_protocol))==-1)
        {
            fprintf(stderr,"Sanity check, socket failed\n");
            continue;
        }
        
        if(connect(sockfd,res->ai_addr,res->ai_addrlen)==-1)
        {
            close(sockfd);
            fprintf(stderr,"Sanity check connect fail\n");
            continue;
        }
        
        break; 
    }
    
    if(res==NULL)
    {
        fprintf(stderr, "client: failed to connect server,please check\n");
        return CONNECT_ERROR;
    }
    
    freeaddrinfo(servinfo);
    
    ////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////
    /////////////////Connection Setup ends//////////////////
    ////////////////////////////////////////////////////////
    
    
    //************************************
    //select loop to receive packets
    //************************************
    
    fd_set master_set;
    fd_set read_set;
    int fd_max,i;
    FD_ZERO(&master_set);
    FD_ZERO(&read_set);
    
    FD_SET(sockfd,&master_set);
    FD_SET(0,&master_set);//let stdin into water
    fd_max=sockfd;//Only one in the set
    
    while(1){
        read_set=master_set;
        if(select(fd_max+1,&read_set,NULL,NULL,NULL)==-1)
        {
            fprintf(stderr,"client select: fail\n");
            exit(4);
        }
        for(i=0;i<=fd_max;i++){
            if(FD_ISSET(i,&read_set)){
               //if i=socketd display
                //if i=stdin, pull command window
                if(i==sockfd){
                    if((numbytes=recv(sockfd,recvBuffer,MAXDATASIZE-1,0))==-1){
                        fprintf(stderr,"client: recv error\n");
                        close(i);
                        FD_CLR(i,&master_set);
                        return RECV_ERROR;
                    }//error case read
                    else if(numbytes==0){
                        printf("Thank you for using UCM demo, goodbye\n");
                        close(i);
                        FD_CLR(i,&master_set);
                        return 0;
                    }//if nbytes=0 ends
                    else{//lets read some data
                        recvBuffer[numbytes]='\0';
                        flag=0;//do both
                        display(recvBuffer,flag);
                        
                    }
                }//end i=sockfd
                else if(i==0){//lets read stdin
                    command(sendBuffer);
                    int len=strlen(sendBuffer);
                    numbytes=send(sockfd,sendBuffer,len,0);
                }//end of stdin
                else{
                    //we cant have other sockfd, critical error
                    fprintf(stderr,"Client select: new sockfd pops, system quit\n");
                    exit(5);
                }//sanity check loop, dont worry
            }//if FD_ISSET ends
        }//end of for loop
        
    }//while (1) loop ends here
    
    return 0;
}

//if flag set to 1, just log,
//if flag set to 0, do both
int display(char *recvBuffer,int flag)
{
    if(flag==0)
        printf("%s",recvBuffer);
    FILE *fp;
    if((fp=fopen("logfile.txt","a"))==NULL)
    {
        fprintf(stderr,"client: open file fail\n");
        return OFILE_ERROR;
    }
    fprintf(fp,"%s",recvBuffer);
    fclose(fp);
    return 0;
}

int command(char* sendBuffer)
{
    char s[100];
    int i,flag=0;
    while(1)
    {
        fgets(s,sizeof(s),stdin);
        if((help_strip_newline(s))!=0)
            printf("No line stripped %s\n",s);
        for(i=0;i<strlen(s);i++)
            s[i]=tolower(s[i]);
        if((flag=(help_command_valid(s)))!=0)
            break;//right data, lets pack
        else
        {
            printf("You have entered %s .",s);
            printf("Command is not vaild\n");
            printf("Please check the command list and enter again\n\n");
            display_menu();
        
            printf("Please enter command: ");
            continue;//sorry, please try again
        }
             
    }//while 1 loop ends
    
    help_pack(s,flag,sendBuffer);
    //printf("\nTEST: sendBuffer is \n%s",sendBuffer);
    
    return 0;
}

void display_menu()
{
    printf("**********************************\n");
    printf("*******UCM COMMAND LIST***********\n");
    printf("*****command case insensitive*****\n");
    printf("*1.login\n");
    printf("*2.status\n");
    printf("*3.queuestatus\n");
    printf("*4.logoff\n");
    printf("*5.ping\n");
    printf("*6.parkedcalls\n");
    printf("*7.sippeers\n");
    printf("*8.queuesummary\n");
    printf("*9.agents\n");
    printf("*10.DBget\n");
    printf("**********************************\n\n");
}
