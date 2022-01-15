#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include "message.h"

#define SERVER_ADDR argv[1]
#define SERVER_PORT argv[2]
#define BUFF_SIZE 1000
static volatile sig_atomic_t flag = 1;


int checkIP();
int checkPort();
int checkRequest();
void respond();
void catch_ctrl_c_and_exit();

int main(int argc, char const *argv[])
{
    signal(SIGINT, catch_ctrl_c_and_exit);
    if (argc != 3)
    {
        printf("Wrong numbers of arguments!\n");
        return 1;
    }
    if (checkIP(argv[1]) == 0)
    {
        printf("Wrong parameter 1! You need to enter a valid IP address!\n");
        return 1;
    }
    if (checkPort(argv[2]) == 0)
    {
        printf("Wrong parameter 2! You need to enter a valid port number!\n");
        return 1;
    }

    // Initiate client socket
    int client;
    client = socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0)
    {
        perror("Problem in creating the socket");
        exit(2);
    }
    printf("Client socket constructed!\n");

    // Define the address of the server
    struct sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    inet_aton(SERVER_ADDR, &serverAddr.sin_addr);
    u_short serverPort = (ushort)atoi(SERVER_PORT);
    serverAddr.sin_port = htons(serverPort);

    //Connect the client to the server
    if (connect(client, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Problem in connecting to the server");
        exit(3);
    }

    char request[1000], accept[1000];
    char buff[BUFF_SIZE + 1];
    int sendBytes, rcvBytes;
    int n;
    message mess;
    int mess_len;
    
    while(flag){
        n = checkRequest(buff);
        if(n == 1){
            sendBytes = send(client, buff, strlen(buff), 0);
            if (sendBytes < 0)
            {
                perror("Error send: ");
                exit(4);                
            }
            rcvBytes = recv(client, buff, BUFF_SIZE, 0);
            if (rcvBytes < 0)
            {
                perror("Error recv: ");
                return 0;            
            } else if (rcvBytes == 0){
                perror("The server terminated prematurely\n");
                exit(5);
            }
            buff[rcvBytes] = '\0';
            printf("[Message from server]: \n%s\n", buff); 
            strncpy(mess.msg_type, buff, 4);
            mess.msg_type[4] = '\0';
            printf("Message type: %s\n", mess.msg_type);

            mess_len = strlen(buff) - 5;
            strncpy(mess.msg_payload, buff+5, mess_len);
            mess.msg_payload[mess_len] = '\0';
            printf("Message payload: %s\n", mess.msg_payload);

            if(strcmp(mess.msg_type, "RESP") == 0){
                respond(mess.msg_payload);
            } 
        }     
    }

    if(!flag) {
        close(client);
        printf("\nClient closed\n");

        FILE *fp;
        fp = fopen("../python/boundary/request.txt", "w");
        fprintf(fp, "%d", 0);
        fclose(fp);
    }

    return 0;
}

int checkIP(char *ip)
{
    int length = strlen(ip);
    int i = 0, dot = 0;
    if (('0' > ip[0]) || (ip[0] > '9'))
        return 0;
    int tmp_number = ip[0] - 48;
    if (('0' > ip[length - 1]) || (ip[length - 1] > '9'))
        return 0;
    for (i = 1; i < length; i++)
    {
        if (ip[i] == '.')
        {
            if (ip[i - 1] == '.')
                return 0;
            tmp_number = 0;
            ++dot;
        }
        else if (('0' > ip[i]) || (ip[i] > '9'))
            return 0;
        else
        {
            tmp_number = tmp_number * 10 + (ip[i] - 48);
            if (tmp_number > 255)
                return 0;
        }
    }
    if (dot != 3)
        return 0;
    return 1;
}

int checkPort(char *num)
{
    int length = strlen(num);
    for (int i = 0; i < length; ++i)
    {
        if (('0' > num[i]) || (num[i] > '9'))
            return 0;
    }
    int port = atoi(num);
    if (port > 65535)
        return 0;
    return 1;
}

int checkRequest(char *buff)
{
    int result;
    FILE *fp;
    fp = fopen("../python/boundary/request.txt", "r");
    if (fp == NULL)
    {
        printf("Input - Empty file!");
        result = 0;
    }
    fgets(buff, 1000, fp);
    if(strcmp(buff, "0") == 0){
        result = 0; //no request
    } else {
        fgets(buff, 1000, fp);
        printf("%s\n", buff);
        result = 1; //request incoming
    }
    fclose(fp);

    fp = fopen("../python/boundary/request.txt", "w");
    fprintf(fp, "%d", 0);
    fclose(fp);
    return result;
}

void respond(char *buff){
    FILE *fp;
    fp = fopen("../python/boundary/respond.txt", "w");
    if (fp == NULL)
    {
        printf("Input - Empty file!");
        fclose(fp);
    }
    fprintf(fp, "%d\n%s", 1, buff);
    fclose(fp);
}

void catch_ctrl_c_and_exit(int sig) {
    flag = 0;
}