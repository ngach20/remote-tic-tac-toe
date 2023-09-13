#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include "signals.h"

int parent_pid;

int waiting_process_pid;

void print_board(char board[9]){
    printf("+-=-=-+\n");
    for(int i = 0; i <= 6; i += 3){
        printf("|%c|%c|%c|\n", board[i], board[i + 1], board[i + 2]);
    }
    printf("+-=-=-+\n");
    fflush(stdout);
}

void game_loop(int fd){
    char buf[64] = {0};
    int row;
    int col;
    while(1){
        read(fd, buf, 1);

        if(buf[0] == PLAY){
            printf("Its now your turn.\n");

            while(1){
                printf("row: ");
                fflush(stdout);
                scanf("%d", &row);
                fflush(stdin);
                printf("\n");
                printf("column: ");
                fflush(stdout);
                scanf("%d", &col);
                fflush(stdin);

                buf[0] = row;
                write(fd, buf, 1);

                buf[0] = col;
                write(fd, buf, 1);

                read(fd, buf, 1);

                if(buf[0] == MOVE_ILL){
                    printf("Illegal move!\n");
                    fflush(stdout);
                }else if(buf[0] == MOVE_OK){
                    break;
                }
            }

            //Receive board data
            read(fd, buf, 9);

            print_board(buf);
            memset(buf, 0, sizeof(buf));
        }else if(buf[0] == WAIT){
            printf("Its now your opponent's turn.\n");
            fflush(stdout);

            //Receive board data
            read(fd, buf, 9);

            print_board(buf);
            memset(buf, 0, sizeof(buf));
        }else if(buf[0] == WON){
            printf("You won!\n");
            fflush(stdout);
            break;
        }else if(buf[0] == LOST){
            printf("You lost!\n");
            fflush(stdout);
            break;
        }else if(buf[0] == DRAW){
            printf("Its a draw!\n");
            fflush(stdout);
            break;
        }
    }
}

void child_sig_handler(int sig){
    if(sig == SIGQUIT){
        if(waiting_process_pid == getpid()){
            wait(NULL);
            exit(0);
        }
        else if(parent_pid != getpid()){ //if this is a child process
            exit(0);
        }
    }
}

void client_process(){
    parent_pid = getpid();
    signal(SIGQUIT, child_sig_handler);

    int client_fd;
    struct sockaddr_in server_addr;
    
    if((client_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("ERROR: Client could not create a socket endpoint\n");
        exit(-1);
    }

    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9002);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if(connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        printf("ERROR: Client could not connet to server\n");
        close(client_fd);
        exit(-1);
    }


    //Suspend until an opponent is found.
    if((waiting_process_pid = fork()) == 0){
        while(1){
            printf("\r                               ");
            printf("\rWaiting for another player.");
            fflush(stdout);
            sleep(1);
            printf("\rWaiting for another player..");
            fflush(stdout);
            sleep(1);
            printf("\rWaiting for another player...");
            fflush(stdout);
            sleep(1);
        }
    }else{
        char sig[5];

        read(client_fd, sig, 5);

        if(!memcmp(sig, "START", 5)){
            kill(0, SIGQUIT);
            wait(NULL);
            printf("\r                                \r");
            printf("Opponent has been found!\n");
        }

        game_loop(client_fd);
    }

    close(client_fd);
}

int main(int argc, char** argv){
    client_process();

    return 0;
}