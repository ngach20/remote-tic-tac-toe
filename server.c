#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "game.c"
#include "signals.h"

void game_loop(int p1_fd, int p2_fd, int server){
    typedef struct {
        char* buf;
        int p_fd;
    } player_t;

    char buf1[64] = {0};
    char buf2[64] = {0};

    player_t *player = malloc(sizeof(player_t)); 
    player_t *waiter = malloc(sizeof(player_t));

    player->buf = buf1;
    player->p_fd = p1_fd;

    waiter->buf = buf2;
    waiter->p_fd = p2_fd;

    int r = 0;
    int c = 0;

    int winner = -1;

    Game *game = init_game();
    while(1){
        player->buf[0] = PLAY;
        waiter->buf[0] = WAIT;

        write(player->p_fd, player->buf, 1);
        write(waiter->p_fd, waiter->buf, 1);

        int illeg = 1;

        while(illeg){
            //row
            read(player->p_fd, player->buf, 1);
            r = player->buf[0] - 1;

            //col
            read(player->p_fd, player->buf, 1);
            c = player->buf[0] - 1;

            illeg = illegal(game, r, c);

            if(illeg){
                player->buf[0] = MOVE_ILL;
                write(player->p_fd, player->buf, 1);
            }
        }

        player->buf[0] = MOVE_OK;
        write(player->p_fd, player->buf, 1);

        make_move(game, r, c);

        write(player->p_fd, game->board, 9);
        write(waiter->p_fd, game->board, 9);

        //Announce the winner and end the game
        if((winner = check_winner(game)) != -1){
            //Its a draw
            if(winner == 2){
                player->buf[0] = DRAW;
                waiter->buf[0] = DRAW;

                write(player->p_fd, player->buf, 1);
                write(waiter->p_fd, waiter->buf, 1);

                break;
            }

            player->buf[0] = WON;
            waiter->buf[0] = LOST;
            write(player->p_fd, player->buf, 1);
            write(waiter->p_fd, waiter->buf, 1);

            break;
        }

        //Switch players
        player_t *tmp = player;
        player = waiter;
        waiter = tmp;
        advance(game);
    }

    //Free all allocated memory
    free(player);
    free(waiter);
    free(game);
}

void server_loop(){
    int server_fd, client_fd1, client_fd2, addr_len, sock_opt;
    sock_opt = 1;
    struct sockaddr_in s_addr;
    
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("ERROR: Server could not create a socket endpoint\n");
        exit(-1);
    }

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &sock_opt, sizeof(sock_opt));
    memset(&s_addr, 0, sizeof(struct sockaddr_in));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(9002);
    s_addr.sin_addr.s_addr = INADDR_ANY;
    addr_len = sizeof(struct sockaddr_in);

    if(bind(server_fd, (struct sockaddr*)&s_addr, addr_len) == -1){
        printf("ERROR: Could not bind to socket\n");
        close(server_fd);
        exit(-1);
    }

    if(listen(server_fd, SOMAXCONN) == -1){
        printf("ERROR: Unable to listen to the socket\n");
        close(server_fd);
        exit(-1);
    }

    printf("Accepting connections...\n");

    while(1){
        while((client_fd1 = accept(server_fd, (struct sockaddr*)&s_addr, &addr_len)) == -1){
            printf("ERROR: Failed to accept connection\n");
        }

        printf("First connection accepted!\n");

        while((client_fd2 = accept(server_fd, (struct sockaddr*)&s_addr, &addr_len)) == -1){
            printf("ERROR: Failed to accept connection\n");
        }

        printf("Second connection accepted!\n");

        int id;
        if((id = fork()) == 0) { //child
            char *buf = "START";
            write(client_fd1, buf, 5);
            write(client_fd2, buf, 5);
            
            game_loop(client_fd1, client_fd2, server_fd);
            //Close connections
            close(client_fd1);
            close(client_fd2);
            close(server_fd);
            exit(0);
        }else{
            close(client_fd1);
            close(client_fd2);
        }
    }

    close(server_fd);
}

int main(int argc, char** argv){
    server_loop();

    return 0;
}