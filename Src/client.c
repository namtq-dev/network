#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <fcntl.h>
#include "message.h"

#define SERVER_ADDR argv[1]
#define SERVER_PORT argv[2]
#define BUFF_SIZE 1000

#define PROJECTID (2332)
#define SHMSIZE (1024)

static volatile sig_atomic_t flag = 1;

int checkIP();
int checkPort();
int checkRequest();
void respond();
void getChallenge();
void catch_ctrl_c_and_exit();
int respondChallenge();
int checkPeer();

pthread_mutex_t a_mutex;
void *do_thread1(void *data);
void *do_thread2(void *data);
void *do_thread3(void *data);
void err_exit(char *buff);

int client;
char request[1000];
char accp[1000];

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

    int res;
    pthread_t p_thread1;
    pthread_t p_thread2;

    /*Khởi tạo mutex*/
    res = pthread_mutex_init(&a_mutex, NULL);

    if (res != 0)
    {
        perror("Mutex create error");
        exit(EXIT_FAILURE);
    }
    /*Tạo tuyến thứ nhất*/
    res = pthread_create(&p_thread1, NULL, do_thread1, NULL);
    if (res != 0)
    {
        perror("Thread create error");
        exit(EXIT_FAILURE);
    }
    /*Tạo tuyến thứ hai*/
    res = pthread_create(&p_thread2, NULL, do_thread2, NULL);
    if (res != 0)
    {
        perror("Thread create error");
        exit(EXIT_FAILURE);
    }

    while (flag)
    {
        sleep(1);
    }

    if (!flag)
    {
        close(client);
        printf("\nClient closed\n");

        FILE *fp;
        fp = fopen("../python/boundary/request.txt", "w");
        fprintf(fp, "%d\n", 0);
        fclose(fp);

        fp = fopen("../python/boundary/challenge.txt", "w");
        fprintf(fp, "%d\n", 0);
        fclose(fp);

        fp = fopen("../python/boundary/peer.txt", "w");
        fprintf(fp, "%d\n", 0);
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
    if (strcmp(buff, "0\n") == 0)
    {
        result = 0; //no request
    }
    else
    {
        fgets(buff, 1000, fp);
        printf("%s\n", buff);
        result = 1; //request incoming
    }
    fclose(fp);

    if (result == 1)
    {
        fp = fopen("../python/boundary/request.txt", "w");
        fprintf(fp, "%d\n", 0);
        fclose(fp);
    }
    return result;
}

void respond(char *buff)
{
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

void getChallenge(char *buff)
{
    FILE *fp;
    fp = fopen("../python/boundary/challenge.txt", "w");
    if (fp == NULL)
    {
        printf("Input - Empty file!");
        fclose(fp);
    }
    fprintf(fp, "%d\n%s", 1, buff);
    fclose(fp);
}

int respondChallenge(char *buff)
{
    int result;
    FILE *fp;
    fp = fopen("../python/boundary/accept.txt", "r");
    if (fp == NULL)
    {
        printf("Input - Empty file!");
        result = 0;
    }
    fgets(buff, 1000, fp);
    char answer[30];
    char addr[30];
    char port[30];
    if (strcmp(buff, "0\n") == 0)
    {
        result = 0; //no answer
    }
    else
    {
        //fgets(buff, 1000, fp);
        fscanf(fp, "%s %s %s", answer, addr, port);
        sprintf(buff, "RESP\n%s %s %s", answer, addr, port);
        printf("%s\n", buff);
        result = 1; //have answer
    }
    fclose(fp);

    if (result == 1)
    {
        fp = fopen("../python/boundary/accept.txt", "w");
        fprintf(fp, "%d\n", 0);
        fclose(fp);
    }
    return result;
}

void catch_ctrl_c_and_exit(int sig)
{
    flag = 0;
}

void *do_thread1(void *data)
{
    char buff[BUFF_SIZE + 1];
    int sendBytes, rcvBytes;
    int n;
    while (flag)
    {
        n = checkRequest(buff);
        if (n == 1)
        {
            pthread_mutex_lock(&a_mutex);
            sendBytes = send(client, buff, strlen(buff), 0);
            if (sendBytes < 0)
            {
                perror("Error send: ");
                exit(0);
            }
            pthread_mutex_unlock(&a_mutex);
        }
        if (respondChallenge(buff) == 1)
        {
            pthread_mutex_lock(&a_mutex);
            sendBytes = send(client, buff, strlen(buff), 0);
            if (sendBytes < 0)
            {
                perror("Error send: ");
                exit(0);
            }
            pthread_mutex_unlock(&a_mutex);
        }
        usleep(100000);
    }
}

void *do_thread2(void *data)
{
    char buff[BUFF_SIZE + 1];
    int sendBytes, rcvBytes;
    message mess;
    int mess_len;
    while (flag)
    {
        rcvBytes = recv(client, buff, BUFF_SIZE, 0);
        //printf("%d\n", rcvBytes);
        if (rcvBytes < 0)
        {
            perror("Error recv: ");
            exit(0);
        }
        else if (rcvBytes == 0)
        {
            perror("The server terminated prematurely\n");
            exit(0);
        }
        buff[rcvBytes] = '\0';
        printf("[Message from server]: \n%s\n", buff);
        strncpy(mess.msg_type, buff, 4);
        mess.msg_type[4] = '\0';
        printf("Message type: %s\n", mess.msg_type);

        mess_len = strlen(buff) - 5;
        strncpy(mess.msg_payload, buff + 5, mess_len);
        mess.msg_payload[mess_len] = '\0';
        printf("Message payload: %s\n", mess.msg_payload);

        if (strcmp(mess.msg_type, "RESP") == 0)
        {
            respond(mess.msg_payload);
        }
        else
        {
            getChallenge(mess.msg_payload);
        }
    }
}

void *do_thread3(void *data)
{
    char MY_ADDR[30];
    char MY_PORT[10];
    char KEYDIR[30];

    FILE *fp;
    fp = fopen("../setting.txt", "r");

    while (fscanf(fp, "%s\n%s\n%s\n", MY_ADDR, MY_PORT, KEYDIR) != EOF)
        ;
    fclose(fp);

    //share memory
    key_t key;
    if (-1 != open(KEYDIR, O_CREAT, 0777))
    {
        key = ftok(KEYDIR, PROJECTID);
        printf("qwert\n");
    }
    if (key < 0)
        err_exit("ftok error");
    if (key < 0)
        err_exit("ftok error");

    int shmid;
    shmid = shmget(key, SHMSIZE, IPC_CREAT | IPC_EXCL | 0664);
    if (shmid == -1)
    {
        if (errno == EEXIST)
        {
            printf("shared memeory already exist\n");
            shmid = shmget(key, 0, 0);
            printf("reference shmid = %d\n", shmid);
        }
        else
        {
            perror("errno");
            err_exit("shmget error");
        }
    }

    char *addr;

    /* Do not to specific the address to attach
     * and attach for read & write*/
    if ((addr = shmat(shmid, 0, 0)) == (void *)-1)
    {
        if (shmctl(shmid, IPC_RMID, NULL) == -1)
            err_exit("shmctl error");
        else
        {
            printf("Attach shared memory failed\n");
            printf("remove shared memory identifier successful\n");
        }
        err_exit("shmat error");
    }

    int clientUDP;
    clientUDP = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientUDP < 0)
    {
        perror("Socket");
        exit(errno);
    }
    printf("ClientUDP socket constructed!\n");

    struct sockaddr_in myAddr;
    bzero(&myAddr, sizeof(myAddr));
    myAddr.sin_family = AF_INET;
    inet_aton(MY_ADDR, &myAddr.sin_addr);
    u_short myPort = (ushort)atoi(MY_PORT);
    myAddr.sin_port = htons(myPort);

    // Bind socket
    if (bind(clientUDP, (struct sockaddr *)&myAddr, sizeof(myAddr)) != 0)
    {
        perror("Bind");
        exit(errno);
    }

    // Define the address of the peer clientUDP
    struct sockaddr_in peerAddr;
    bzero(&peerAddr, sizeof(peerAddr));
    peerAddr.sin_family = AF_INET;

    char PEER_ADDR[30];
    char PEER_PORT[10];
    while (flag)
    {
        if (checkPeer(PEER_ADDR, PEER_PORT) == 1)
        {
            inet_aton(PEER_ADDR, &peerAddr.sin_addr);
            u_short peerPort = (ushort)atoi(PEER_PORT);
            peerAddr.sin_port = htons(peerPort);
            //Connect with peer port
            if (connect(clientUDP, (struct sockaddr *)&peerAddr, sizeof(peerAddr)) != 0)
            {
                perror("Connect");
                exit(errno);
            }
            // Communicate with peer clientUDP
            char buff[BUFF_SIZE + 1];
            int sendBytes, rcvBytes;
            socklen_t len = sizeof(peerAddr);

            int x1, y1, x2, y2, n1, n2;

            while (1)
            {
                sscanf(addr, "%d%d%d%d%d%d", &x1, &y1, &x2, &y2, &n1, &n2);
                sprintf(buff, "%d %d %d", x1, y1, n1);
                sendBytes = sendto(clientUDP, buff, strlen(buff), 0, (struct sockaddr *)&peerAddr, len);
                buff[0] = '\0';

                rcvBytes = recvfrom(clientUDP, buff, BUFF_SIZE, 0, (struct sockaddr *)&peerAddr, &len);

                sscanf(buff, "%d%d%d", &x2, &y2, &n2);
                sprintf(addr, "%d      %d      %d      %d      %d      %d      abc", x1, y1, x2, y2, n1, n2);
                buff[rcvBytes] = '\0';
                usleep(10000);
            }
        }
        usleep(100000);
    }

    close(clientUDP);
}

void err_exit(char *buff)
{
    fprintf(stderr, "%s\n", buff);
    exit(1);
}

int checkPeer(char *addr, char *port)
{
    int result;
    char buff[1000];
    FILE *fp;
    fp = fopen("../python/boundary/peer.txt", "r");
    if (fp == NULL)
    {
        printf("Input - Empty file!");
        result = 0;
    }
    fgets(buff, 1000, fp);
    if (strcmp(buff, "0\n") == 0)
    {
        result = 0; //no request
    }
    else
    {
        fscanf(fp, "%s %s", addr, port);
        printf("%s\n", port);
        result = 1; //request incoming
    }
    fclose(fp);

    if (result == 1)
    {
        fp = fopen("../python/boundary/peer.txt", "w");
        fprintf(fp, "%d\n", 0);
        fclose(fp);
    }
    return result;
}