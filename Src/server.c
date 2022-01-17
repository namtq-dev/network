#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include "message.h"
#include "user.h"

#define SERVER_PORT argv[1]
#define BUFF_SIZE 1000

int checkPort();
int clientHandler();

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("Wrong numbers of arguments!\n");
        return 1;
    }
    if (checkPort(argv[1]) == 0)
    {
        printf("Wrong parameter! You need to enter a valid port number!\n");
        return 1;
    }

    // Initiate server socket
    int server;
    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0)
    {
        perror("1-Problem in creating the socket");
        exit(2);
    }
    printf("Server socket constructed!\n");

    //Bind address to socket
    struct sockaddr_in serverAddr;
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    u_short serverPort = (ushort)atoi(SERVER_PORT);
    serverAddr.sin_port = htons(serverPort);
    if (bind(server, (struct sockaddr *)&serverAddr, sizeof(serverAddr)))
    {
        perror("2-Error: ");
        exit(3);
    }
    printf("Server started.\n");

    //Multi-plexing
    fd_set fds1, fds2;
    FD_ZERO(&fds1);
    FD_ZERO(&fds2);

    if (listen(server, 5))
    {
        perror("3-Error: ");
        exit(4);
    }
    FD_SET(server, &fds1); 
    //FD_SET(server, &fds2); 
    int maxfd = server;
    printf("Server running...waiting for connections.\n");

    //Database
    client list = createNewList();
    int size = inputFromFile(list);
    //showList(list, size);

    // Communicate with client
    connfd client[FD_SETSIZE];
    struct sockaddr_in cliaddr;
    char buff[BUFF_SIZE + 1];
    socklen_t len;
    len = sizeof(cliaddr);
    for (int i = 0; i < FD_SETSIZE; i++){
        memset(&(client[i].clientfd), -1, sizeof(int));
    }
    

    int nEvent;
    int conn;
    int count_conn = 0;
    while(1)
    {
        fds2 = fds1;
        nEvent = select(maxfd+1, &fds2, NULL, NULL, NULL);
        if (nEvent == -1) {
            perror("Select-Error: ");
        } else {
            if(FD_ISSET(server, &fds2)){
                //Accept a connection request
                conn = accept(server, (struct sockaddr *)&cliaddr, &len);
                count_conn++;
                maxfd = conn;
                FD_SET(conn, &fds1);
                for(int i = 0; i < FD_SETSIZE-1; ++i){
                    if(client[i].clientfd == -1) {
                        client[i].clientfd = conn;
                        break;
                    }
                }
                printf("Received request...\n");
            }
            for(int i = 0; i < FD_SETSIZE-1; ++i){
                if(FD_ISSET(client[i].clientfd, &fds2)){
                    int result = clientHandler(client, buff, i, list, &size, count_conn);
                    if(result == 0){
                        count_conn--;
                        close(client[i].clientfd);
                        FD_CLR(client[i].clientfd, &fds1);
                        client[i].clientfd = -1;
                    }
                }
            }
        }
    }

    close(server);

    return 0;
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

int clientHandler(connfd *user, char *buff, int i, client list, int *size, int maxfd){
    int rcvBytes, sendBytes;
    socklen_t len;
    message mess;
    int mess_len;
    char respond[1000];
    rcvBytes = recv((*(user + i)).clientfd, buff, BUFF_SIZE, 0);
    if(rcvBytes < 0){
        perror("Receive:");
        return 0;
    }
    if(rcvBytes == 0){
        printf("Client disconnected\n");
        return 0;
    }

    buff[rcvBytes] = '\0';
    getpeername((*(user + i)).clientfd, (struct sockaddr *)&((*(user + i)).cliaddr), &len);
    printf("Client[%s:%d]: %s\n", inet_ntoa((*(user + i)).cliaddr.sin_addr), ntohs((*(user + i)).cliaddr.sin_port), buff);
    strncpy(mess.msg_type, buff, 4);
    mess.msg_type[4] = '\0';
    printf("Message type: %s\n", mess.msg_type);

    mess_len = strlen(buff) - 5;
    strncpy(mess.msg_payload, buff+5, mess_len);
    mess.msg_payload[mess_len] = '\0';
    printf("Message payload: %s\n", mess.msg_payload);

    if (strcmp(mess.msg_type, "USER") == 0)
    {
        printf("1\n");
        strcpy(respond, checkLogin(list, *size, mess.msg_payload));
    } 
    
    if (strcmp(mess.msg_type, "REGI") == 0)
    {
        printf("2\n");
        strcpy(respond, checkSignUp(list, size, mess.msg_payload));        
    } 

    if (strcmp(mess.msg_type, "RANK") == 0)
    {
        printf("3\n");
        strcpy((*(user + i)).username, mess.msg_payload);
        strcpy(respond, getRank(list, *size, mess.msg_payload));
    }

    if (strcmp(mess.msg_type, "LIST") == 0)
    {
        printf("4\n");
        strcpy((*(user + i)).username, mess.msg_payload);
        for(int i = 0; i < *size; i++){
            if(strcmp(list[i].username, mess.msg_payload) == 0){
                list[i].status = 1;
            }
        }
        strcpy(respond, checkStadium(list, *size, mess.msg_payload));
    }
    
    if (strcmp(mess.msg_type, "LEAV") == 0)
    {
        printf("5\n");
        strcpy(respond, leaveStadium(list, *size, mess.msg_payload));
    }

    if (strcmp(mess.msg_type, "CHAL") == 0)
    {
        printf("6\n");
        strcpy(respond, sendChallenge(user, mess.msg_payload, maxfd));
    }

    if (strcmp(mess.msg_type, "SUBM") == 0)
    {
        printf("7\n");
        strcpy(respond, checkResult(list, *size, mess.msg_payload));
    }

    sendBytes = send((*(user + i)).clientfd, respond, strlen(respond)+1, 0);

}