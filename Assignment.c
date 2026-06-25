#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE 15
#define MAX_PLAYERS 3
#define MAX_LOG 100
#define LOG_DISPLAY 5

#define WALL '#'
#define TREASURE 'T'
#define HEALTH 'H'
#define KEY 'K'
#define DOOR 'D'

typedef struct {
    char name[50];
    int row,col;
    int score;
    int health;
    int keys;
    char symbol;

    int movesMade;
    int treasuresFound;
    int trapsTriggered;
    int damageTaken;
    int healthPacks;
    int keysCollected;
    int doorsUnlocked;
} Player;

char map[SIZE][SIZE];
int hiddenTrap[SIZE][SIZE];

Player players[MAX_PLAYERS];
int playerCount;
int roundCounter = 1;

char eventLog[MAX_LOG][200];
int logCount = 0;

void addLog(int r,const char *msg){
    if(logCount < MAX_LOG){
        snprintf(eventLog[logCount],200,"[R%d] %s",r,msg);
        logCount++;
    }
}

void printRecentLog(){
    int start = (logCount > LOG_DISPLAY)? logCount-LOG_DISPLAY : 0;
    printf("\nRecent Events:\n");
    for(int i=start;i<logCount;i++)
        printf("%s\n",eventLog[i]);
}

void initializeMap(){
    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE;j++){
            map[i][j]=' ';
            hiddenTrap[i][j]=0;
        }
    }

    for(int i=0;i<SIZE;i++){
        map[0][i]=WALL;
        map[SIZE-1][i]=WALL;
        map[i][0]=WALL;
        map[i][SIZE-1]=WALL;
    }
}

void placeRandom(char item,int count){
    while(count){
        int r=rand()%SIZE;
        int c=rand()%SIZE;
        if(map[r][c]==' '){
            map[r][c]=item;
            count--;
        }
    }
}

void placeTraps(int count){
    while(count){
        int r=rand()%SIZE;
        int c=rand()%SIZE;
        if(map[r][c]==' ' && hiddenTrap[r][c]==0){
            hiddenTrap[r][c]=1;
            count--;
        }
    }
}

void placePlayers(){
    for(int i=0;i<playerCount;i++){
        while(1){
            int r=rand()%SIZE;
            int c=rand()%SIZE;
            if(map[r][c]==' '){
                players[i].row=r;
                players[i].col=c;
                map[r][c]=players[i].symbol;
                break;
            }
        }
    }
}

void printMap(){
    printf("\n===== ROUND %d =====\n",roundCounter);

    for(int i=0;i<SIZE;i++){
        for(int j=0;j<SIZE;j++)
            printf("%c ",map[i][j]);
        printf("\n");
    }

    printf("\nPlayer Status\n");
    for(int i=0;i<playerCount;i++){
        printf("%s HP:%d Score:%d Keys:%d\n",
               players[i].name,
               players[i].health,
               players[i].score,
               players[i].keys);
    }

    printRecentLog();
}

int isValidMove(int r,int c){
    if(r<0 || c<0 || r>=SIZE || c>=SIZE) return 0;
    if(map[r][c]==WALL) return 0;
    return 1;
}

void processTile(int p){
    int r=players[p].row;
    int c=players[p].col;

    if(hiddenTrap[r][c]){
        hiddenTrap[r][c]=0;
        players[p].health-=20;
        players[p].trapsTriggered++;
        players[p].damageTaken+=20;
    }

    if(map[r][c]==TREASURE){
        players[p].score+=10;
        players[p].treasuresFound++;
        map[r][c]=' ';
    }

    if(map[r][c]==HEALTH){
        players[p].health+=20;
        if(players[p].health>100) players[p].health=100;
        players[p].healthPacks++;
        map[r][c]=' ';
    }

    if(map[r][c]==KEY){
        players[p].keys++;
        players[p].keysCollected++;
        map[r][c]=' ';
    }
}

void movePlayer(int p){
    char moves[20];

    printf("\n%s moves: ",players[p].name);
    scanf("%19s",moves);

    if(strlen(moves)>4){
        printf("Too many moves.\n");
        return;
    }

    for(int k=0;moves[k];k++){

        int nr=players[p].row;
        int nc=players[p].col;

        switch(moves[k]){
            case 'W': case 'w': nr--; break;
            case 'S': case 's': nr++; break;
            case 'A': case 'a': nc--; break;
            case 'D': case 'd': nc++; break;
            default: continue;
        }

        if(!isValidMove(nr,nc)) continue;

        if(map[nr][nc]==DOOR){
            if(players[p].keys>0){
                players[p].keys--;
                players[p].doorsUnlocked++;
                map[nr][nc]=' ';
            }else continue;
        }

        map[players[p].row][players[p].col]=' ';
        players[p].row=nr;
        players[p].col=nc;

        processTile(p);

        map[nr][nc]=players[p].symbol;

        players[p].movesMade++;
    }
}

int remainingTreasures(){
    int count=0;
    for(int i=0;i<SIZE;i++)
        for(int j=0;j<SIZE;j++)
            if(map[i][j]==TREASURE) count++;
    return count;
}

void saveGame(){
    FILE *fp=fopen("savegame.dat","wb");
    if(!fp) return;

    fwrite(&playerCount,sizeof(int),1,fp);
    fwrite(players,sizeof(Player),MAX_PLAYERS,fp);
    fwrite(map,sizeof(map),1,fp);
    fwrite(hiddenTrap,sizeof(hiddenTrap),1,fp);
    fwrite(&roundCounter,sizeof(int),1,fp);

    fclose(fp);
}

void loadGame(){
    FILE *fp=fopen("savegame.dat","rb");
    if(!fp) return;

    fread(&playerCount,sizeof(int),1,fp);
    fread(players,sizeof(Player),MAX_PLAYERS,fp);
    fread(map,sizeof(map),1,fp);
    fread(hiddenTrap,sizeof(hiddenTrap),1,fp);
    fread(&roundCounter,sizeof(int),1,fp);

    fclose(fp);
}

void saveLog(){
    FILE *fp=fopen("gamelog.txt","w");
    if(!fp) return;

    for(int i=0;i<logCount;i++)
        fprintf(fp,"%s\n",eventLog[i]);

    fclose(fp);
}

void showStats(){
    printf("\nPLAYER STATISTICS\n");

    for(int i=0;i<playerCount;i++){
        printf("\n%s\n",players[i].name);
        printf("Moves:%d\n",players[i].movesMade);
        printf("Treasures:%d\n",players[i].treasuresFound);
        printf("Traps:%d\n",players[i].trapsTriggered);
        printf("Damage:%d\n",players[i].damageTaken);
        printf("Health Packs:%d\n",players[i].healthPacks);
        printf("Keys:%d\n",players[i].keysCollected);
        printf("Doors:%d\n",players[i].doorsUnlocked);
    }
}

void showScores(){
    for(int i=0;i<playerCount;i++)
        if(players[i].health>0)
            players[i].score += players[i].health/2;

    printf("\nLEADERBOARD\n");
    for(int i=0;i<playerCount;i++)
        printf("%s : %d\n",players[i].name,players[i].score);
}

void gameLoop(){
    while(1){

        printMap();

        for(int i=0;i<playerCount;i++){
            if(players[i].health>0)
                movePlayer(i);
        }

        if(remainingTreasures()==0){
            printf("\nAll treasures collected!\n");
            break;
        }

        int alive=0;
        for(int i=0;i<playerCount;i++)
            if(players[i].health>0) alive++;

        if(alive==0){
            printf("\nAll players eliminated!\n");
            break;
        }

        roundCounter++;

        char ch;
        printf("\nSave game? (y/n): ");
        scanf(" %c",&ch);

        if(ch=='y' || ch=='Y')
            saveGame();
    }

    saveLog();
    showScores();
    showStats();
}

int main(){
    srand((unsigned)time(NULL));

    int choice;

    printf("1. New Game\n2. Load Game\nChoice: ");
    scanf("%d",&choice);

    if(choice==2){
        loadGame();
        gameLoop();
        return 0;
    }

    printf("Players (1-3): ");
    scanf("%d",&playerCount);

    if(playerCount<1 || playerCount>3)
        playerCount=1;

    initializeMap();

    placeRandom(WALL,30);
    placeRandom(TREASURE,12);
    placeRandom(HEALTH,5);
    placeRandom(KEY,3);
    placeRandom(DOOR,3);
    placeTraps(10);

    for(int i=0;i<playerCount;i++){
        printf("Player %d Name: ",i+1);
        scanf("%s",players[i].name);

        players[i].score=0;
        players[i].health=100;
        players[i].keys=0;
        players[i].symbol='1'+i;

        players[i].movesMade=0;
        players[i].treasuresFound=0;
        players[i].trapsTriggered=0;
        players[i].damageTaken=0;
        players[i].healthPacks=0;
        players[i].keysCollected=0;
        players[i].doorsUnlocked=0;
    }

    placePlayers();

    gameLoop();

    return 0;
}
