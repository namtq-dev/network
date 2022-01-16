#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>

typedef struct User{
    char username[30];
    char password[30];
    int status;
    int win;
    int lose;
} *client;

typedef struct Connect{
    char username[30];
    int clientfd;
    struct sockaddr_in cliaddr;
} connfd;

client createNewList();
int inputFromFile();
char* checkLogin();
char* checkSignUp();
char *checkStadium();
char *leaveStadium();
void updateDB();
char *getRank();
void swap();
void showList();
char *sendChallenge();

client createNewList(){
    client list;
    list = calloc(100, sizeof(struct User));
    return list;
}

int inputFromFile(client list)
{
    FILE *fp;
    fp = fopen("account.txt", "r");
    if (fp == NULL)
    {
        printf("Empty file!");
        fclose(fp);
        exit(1);
    }
    int i = 0;
    while (fscanf( fp, "%s %s %d %d", list[i].username, list[i].password, &(list[i].win), &(list[i].lose)) != EOF){
        ++i;
    }
    return i;
}

void showList(client list, int size){
    for(int i = 0; i < size; i++){
        printf("%s %s %d %d\n", list[i].username, list[i].password, list[i].win, list[i].lose);
    }
}

char* checkLogin(client list, int size, char *buff){
    char username[30];
    char password[30];
    char *log = calloc(5000, sizeof(char));
    int result = 0;
    sscanf(buff, "%s %s", username, password);
    for(int i = 0; i < size; ++i){
        if( strcmp(username, list[i].username) == 0 && strcmp(password, list[i].password) == 0){
            result = 1;
            break;
        }
    }
    if(result == 1){ 
        strcpy(log, "RESP\nsuccess\n");
    } else{
        strcpy(log, "RESP\nfail\n");    
    }
    return log;
}

char* checkSignUp(client list, int *size, char *buff){
    char username[30];
    char password[30];
    char *log = calloc(5000, sizeof(char));
    int result = 1;
    sscanf(buff, "%s %s", username, password);
    for(int i = 0; i < *size; ++i){
        if( strcmp(username, list[i].username) == 0){
            result = 0;
            break;
        }
    }
    if(result == 1){
        strcpy(list[*size].username, username);
        strcpy(list[*size].password, password);
        (*size)++;
        updateDB(list, *size);       
    }
    if(result == 1){ 
        strcpy(log, "RESP\nsuccess\n");
    } else{
        strcpy(log, "RESP\nfail\n");    
    }
    return log;
}

char *checkStadium(client list, int size, char *buff){
    char *log = calloc(5000, sizeof(char));
    sprintf(log, "%s\nsuccess\n", "RESP");
    for(int i = 0; i < size; i++){
        if((list[i].status == 1) && (strcmp(buff, list[i].username) != 0)){            
            sprintf(log + strlen(log), "%s\n", list[i].username);
        }
    }
    return log;
}

char *leaveStadium(client list, int size, char *buff){
    char username[30];
    char *log = calloc(5000, sizeof(char));
    sscanf(buff, "%s", username);
    for(int i = 0; i < size; ++i){
        if( strcmp(username, list[i].username) == 0){
            list[i].status = 0;
            break;
        }
    }
    strcpy(log, "RESP\nsuccess\n");
    return log;
}

void updateDB(client list, int size){
    FILE *fp;
    fp = fopen("account.txt", "w");
    if (fp == NULL)
    {
        printf("Empty file!");
        fclose(fp);
        exit(1);
    }

    for (int i = 0; i < size; i++)
    {
        fprintf(fp, "%s %s %d %d\n", list[i].username, list[i].password, list[i].win, list[i].lose);
    }
    fclose(fp);
}

char *getRank(client list, int size, char* buff){
    char *log = calloc(5000, sizeof(char));
    char username[size][30];
    int win[size];
    int lose[size];
    int score[size];
    char cur_name[30];
    int cur_win, cur_lose;
    sscanf(buff, "%s", cur_name);
    for(int i = 0; i < size; ++i){
        if(strcmp(cur_name, list[i].username) == 0){
            cur_win = list[i].win;
            cur_lose = list[i].lose;
        }
        strcpy(username[i], list[i].username);
        win[i] = list[i].win;
        lose[i] = list[i].lose;
        score[i] = win[i] - lose[i];
    }

    int have_swap = 0;
    for(int i = 0; i < size - 1; i++){
        have_swap = 0;
        for(int j = 0; j < size - i - 1; j++){
            if(score[j] < score[j+1]){
                swap(username[j], username[j+1], &(win[j]), &(win[j+1]), &(lose[j]), &(lose[j+1]), &(score[j]), &(score[j+1]));
                have_swap = 1;
            }
        }
        if(have_swap == 0){
            break;
        }
    }
    // Current User
    sprintf(log, "RESP\nsuccess\n%d %d\n", cur_win, cur_lose);  

    // Ranking
    for (int i = 0; i < size; i++)
    {
        sprintf(log + strlen(log), "%s %d %d %d\n", username[i], win[i], lose[i], i+1);
    }

    return log; 
}

void swap(char *A, char *B, int *winA, int *winB, int *loseA, int *loseB, int *scoreA, int *scoreB){
    char temp[30];
    int tmp_win;
    int tmp_lose;
    int tmp_score;

    strcpy(temp, A);
    strcpy(A, B);
    strcpy(B, temp);

    tmp_win = *winA;
    *winA = *winB;
    *winB = tmp_win;

    tmp_lose = *loseA;
    *loseA = *loseB;
    *loseB = tmp_lose;

    tmp_score = *scoreA;
    *scoreA = *scoreB;
    *scoreB = tmp_score;
}

char *sendChallenge(connfd *user, char *buff, int maxfd){
    char nameA[30];
    char nameB[30];
    char addr[30];
    char port[30];
    char *log = calloc(5000, sizeof(char));
    int sendBytes, rcvBytes;
    sscanf(buff, "%s %s %s %s", nameA, nameB, addr, port);
    for(int i = 0; i < maxfd; i++){
        printf("%d--- %s\n", i, user[i].username);
        if(strcmp(user[i].username, nameB) == 0){
            sprintf(buff, "CHAL %s %s %s %s", nameA, nameB, addr, port);
            sendBytes = send(user[i].clientfd, buff, strlen(buff), 0);
            if(sendBytes < 0){
                perror("Send Challenge Error:");
                strcpy(log, "RESP\nfail\n"); 
                break;
            }
            memset(buff, '\0', 1000*sizeof(char));
            rcvBytes = recv(user[i].clientfd, buff, 1000, 0);
            if(rcvBytes < 0){
                perror("Receive Challenge Error:");
                strcpy(log, "RESP\nfail\n"); 
                break;
            }
            strcpy(log, buff);
            break;
        }
    }

    return log;
}

