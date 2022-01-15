#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define GETEKYDIR ("/tmp")
#define PROJECTID (2332)
#define SHMSIZE (1024)

#define MY_ADDR argv[1]
#define MY_PORT argv[2]

#define PEER_ADDR argv[3]
#define PEER_PORT argv[4]

#define BUFF_SIZE 1024

int checkIP();
int checkPort();
int input();

void err_exit(char *buf)
{
    fprintf(stderr, "%s\n", buf);
    exit(1);
}

int main(int argc, char const *argv[])
{
    if (argc != 5)
    {
        printf("Wrong numbers of arguments!\n");
        return 0;
    }
    if (checkIP(argv[1]) == 0 || checkIP(argv[3]) == 0)
    {
        printf("Wrong parameter 1! You need to enter a valid IP address!\n");
        return 0;
    }
    if (checkPort(argv[2]) == 0 || checkPort(argv[4]) == 0)
    {
        printf("Wrong parameter 2! You need to enter a valid port number!\n");
        return 0;
    }

    //share memory
    key_t key = ftok(GETEKYDIR, PROJECTID);
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

    // Initiate client socket
    int client;
    client = socket(AF_INET, SOCK_DGRAM, 0);
    if (client < 0)
    {
        perror("Socket");
        exit(errno);
    }
    printf("Client socket constructed!\n");

    struct sockaddr_in myAddr;
    bzero(&myAddr, sizeof(myAddr));
    myAddr.sin_family = AF_INET;
    inet_aton(MY_ADDR, &myAddr.sin_addr);
    u_short myPort = (ushort)atoi(MY_PORT);
    myAddr.sin_port = htons(myPort);

    // Bind socket
    if (bind(client, (struct sockaddr *)&myAddr, sizeof(myAddr)) != 0)
    {
        perror("Bind");
        exit(errno);
    }

    // Define the address of the peer client
    struct sockaddr_in peerAddr;
    bzero(&peerAddr, sizeof(peerAddr));
    peerAddr.sin_family = AF_INET;
    inet_aton(PEER_ADDR, &peerAddr.sin_addr);
    u_short peerPort = (ushort)atoi(PEER_PORT);
    peerAddr.sin_port = htons(peerPort);

    //Connect with peer port
    if (connect(client, (struct sockaddr *)&peerAddr, sizeof(peerAddr)) != 0)
    {
        perror("Connect");
        exit(errno);
    }

    // Communicate with peer client
    char buff[BUFF_SIZE + 1];
    int sendBytes, rcvBytes;
    socklen_t len = sizeof(peerAddr);

    int x1, y1, x2, y2, n1, n2;

    while (1)
    {

        sscanf(addr, "%d%d%d%d%d%d", &x1, &y1, &x2, &y2, &n1, &n2);
        sprintf(buff, "%d %d %d", x1, y1, n1);
        sendBytes = sendto(client, buff, strlen(buff), 0, (struct sockaddr *)&peerAddr, len);
        buff[0] = '\0';

        rcvBytes = recvfrom(client, buff, BUFF_SIZE, 0, (struct sockaddr *)&peerAddr, &len);

        sscanf(buff, "%d%d%d", &x2, &y2, &n2);
        sprintf(addr, "%d      %d      %d      %d      %d      %d      abc", x1, y1, x2, y2, n1, n2);
        buff[rcvBytes] = '\0';
        usleep(10000);
    }

    close(client);
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

int input(char s[])
{
    memset(s, '\0', BUFF_SIZE * sizeof(char));
    char c;
    int i = 0, check = 0;
    c = getchar();
    while (c != '\n')
    {
        if (c == ' ')
            check = 2; //error blank
        s[i] = c;
        i++;
        c = getchar();
    }
    if (check == 2)
    {
        printf("Can't input blank!");
        return 2;
    }
    if (i == 0)
        return 1; //enter -> end client

    return 0; //no error
}
