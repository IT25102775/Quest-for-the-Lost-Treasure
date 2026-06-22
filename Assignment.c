#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_PLAYERS 3
#define MAX_SYMBOLS 3
const char symbols[MAX_SYMBOLS] = {'X', 'O', 'Z'};

typedef struct {
    int isComputer;
    char symbol;
} Player;

char **board;
int N;
FILE *logFile;

void initializeBoard(int size) {
    N = size;
    board = (char **)malloc(N * sizeof(char *));
    for (int i = 0; i < N; i++) {
        board[i] = (char *)malloc(N * sizeof(char));
        for (int j = 0; j < N; j++) {
            board[i][j] = ' ';
        }
    }
}
return o;
}
