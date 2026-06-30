#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE 15
#define MAX_PLAYERS 3
#define MAX_LOG 100

typedef struct{
    char name[50];
    int row,col,score,health,keys;
    char symbol;
    int movesMade,treasuresFound,trapsTriggered;
    int damageTaken,healthPacks,keysCollected,doorsUnlocked;
} Player;

/* Global variables */
char map[SIZE][SIZE];
int hiddenTrap[SIZE][SIZE];
Player players[MAX_PLAYERS];
int playerCount=0;
int roundCounter=1;

/* Function prototypes */
void initializeMap(void);
void placePlayers(void);
void printMap(void);
void gameLoop(void);
void saveGame(void);
void loadGame(void);
void showScores(void);
void showStats(void);

void initializeMap(void){
    int i,j;
    for(i=0;i<SIZE;i++){
        for(j=0;j<SIZE;j++){
            map[i][j]=' ';
            hiddenTrap[i][j]=0;
        }
    }

    for(i=0;i<SIZE;i++){
        map[0][i]='#';
        map[SIZE-1][i]='#';
        map[i][0]='#';
        map[i][SIZE-1]='#';
    }
}

void placePlayers(void){
    int i;
    for(i=0;i<playerCount;i++){
        players[i].row=1+i;
        players[i].col=1+i;
        map[players[i].row][players[i].col]=players[i].symbol;
    }
}

void printMap(void){
    int i,j;
    printf("\n===== ROUND %d =====\n",roundCounter);

    for(i=0;i<SIZE;i++){
        for(j=0;j<SIZE;j++){
            printf("%c ",map[i][j]);
        }
        printf("\n");
    }
}

void saveGame(void){
    FILE *fp=fopen("savegame.dat","wb");
    if(!fp) return;

    fwrite(&playerCount,sizeof(int),1,fp);
    fwrite(players,sizeof(Player),MAX_PLAYERS,fp);
    fwrite(map,sizeof(map),1,fp);
    fwrite(hiddenTrap,sizeof(hiddenTrap),1,fp);
    fwrite(&roundCounter,sizeof(int),1,fp);

    fclose(fp);
}

void loadGame(void){
    FILE *fp=fopen("savegame.dat","rb");
    if(!fp){
        printf("No save file found.\n");
        return;
    }

    fread(&playerCount,sizeof(int),1,fp);
    fread(players,sizeof(Player),MAX_PLAYERS,fp);
    fread(map,sizeof(map),1,fp);
    fread(hiddenTrap,sizeof(hiddenTrap),1,fp);
    fread(&roundCounter,sizeof(int),1,fp);

    fclose(fp);
}

void showStats(void){
    int i;
    printf("\nPLAYER STATISTICS\n");
    for(i=0;i<playerCount;i++){
        printf("%s Moves:%d Treasures:%d\n",
               players[i].name,
               players[i].movesMade,
               players[i].treasuresFound);
    }
}

void showScores(void){
    int i;
    printf("\nLEADERBOARD\n");
    for(i=0;i<playerCount;i++){
        printf("%s : %d\n",players[i].name,players[i].score);
    }
}

void gameLoop(void){
    int running=1;
    while(running){
        printMap();

        printf("\nEnd demo game loop? (1=yes): ");
        scanf("%d",&running);

        if(running==1) break;

        roundCounter++;
    }

    showScores();
    showStats();
}

int main(void){
    int choice,i;

    srand((unsigned)time(NULL));

    printf("1. New Game\n2. Load Game\nChoice: ");
    scanf("%d",&choice);

    if(choice==2){
        loadGame();
        gameLoop();
        return 0;
    }

    printf("Number of Players (1-3): ");
    scanf("%d",&playerCount);

    if(playerCount<1 || playerCount>3)
        playerCount=1;

    for(i=0;i<playerCount;i++){
        printf("Player %d Name: ",i+1);
        scanf("%49s",players[i].name);

        players[i].score=0;
        players[i].health=100;
        players[i].keys=0;
        players[i].symbol='1'+i;
    }

    initializeMap();
    placePlayers();
    gameLoop();

    return 0;
}
