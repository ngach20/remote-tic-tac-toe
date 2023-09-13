#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define BOARD game->board
#define PLAYER game->cur_player

typedef struct Game{
    char cur_player;
    char board[3][3];
} Game;

//Creates a game struct
//Passes the ownership to the caller
Game *init_game(){
    struct Game *game = malloc(sizeof(struct Game));
    PLAYER = 'O';
    memset(BOARD, '#', sizeof(char) * 9);
    return game;
}

void make_move(Game *game, int row, int col){
    assert(row >= 0 && row <= 2);
    assert(col >= 0 && col <= 2);

    BOARD[row][col] = PLAYER;
}

int check_winner(Game *game){
    //left diagonal
    if(BOARD[0][0] == PLAYER && BOARD[1][1] == PLAYER && BOARD[2][2] == PLAYER){
        return PLAYER;
    }
    
    //right diagonal
    if(BOARD[0][2] == PLAYER && BOARD[1][1] == PLAYER && BOARD[2][0] == PLAYER){
        return PLAYER;
    }

    //check all rows
    for(int i = 0; i <= 2; ++i){   
        if(BOARD[i][2] == PLAYER && BOARD[i][1] == PLAYER && BOARD[i][0] == PLAYER){
            return PLAYER;
        }
    }

    //check all columns
    for(int i = 0; i <= 2; ++i){   
        if(BOARD[0][i] == PLAYER && BOARD[1][i] == PLAYER && BOARD[2][i] == PLAYER){
            return PLAYER;
        }
    }

    //If the board is full
    int full = 1;
    for(int i = 0; i <= 2; ++i){
        for(int j = 0; j <= 2; ++j){
            if(BOARD[i][j] == '#'){
                full = 0;
                break;
            }
        }
    }

    if(full){
        return 2;
    }

    return -1;
}

void advance(Game *game){
    PLAYER = (PLAYER == 'X') ? 'O' : 'X';
}

int illegal(Game *game, int r, int c){
    return (r < 0 || r > 2 || c < 0 || c > 2 || BOARD[r][c] != '#');
}