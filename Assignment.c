/* IT25102775 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* Constants */
#define GRID_SIZE      15
#define MAX_PLAYERS    3
#define WALLS          30
#define TREASURES      12
#define HEALTH_PACKS   5
#define KEYS           3
#define DOORS          3
#define TRAPS          10
#define LOG_SIZE       100
#define LOG_DISPLAY    5
#define MAX_HP         100
#define MAX_MOVE_LEN   4
#define SAVE_FILE      "savegame.dat"
#define LOG_FILE       "gamelog.txt"

/* Map symbols */
#define SYM_WALL     '#'
#define SYM_TREASURE 'T'
#define SYM_HEALTH   'H'
#define SYM_KEY      'K'
#define SYM_DOOR     'D'
#define SYM_EMPTY    ' '

/* Structs  */
typedef struct {
    char name[30];
    int  x, y;
    int  score;
    int  health;
    int  keys;
    char symbol;      

    
    int movesMade;
    int treasuresFound;
    int trapsTriggered;
    int damageTaken;
    int healthPacksUsed;
    int keysCollected;
    int doorsUnlocked;
} Player;

typedef struct {
    int  round;
    char message[100];
} LogEntry;

/*  Globals  */
char   map[GRID_SIZE][GRID_SIZE];
int    hiddenTrap[GRID_SIZE][GRID_SIZE];   

Player players[MAX_PLAYERS];
int    playerCount = 2;

LogEntry eventLog[LOG_SIZE];
int      logCount = 0;     

int roundCounter = 1;

/*  Prototypes */
void   initializeMap(void);
void   placeWalls(void);
void   placeTreasures(void);
void   placeTraps(void);
void   placeHealthPacks(void);
void   placeKeys(void);
void   placeDoors(void);
void   placePlayers(void);
int    placeRandomSymbol(char symbol);
int    tileIsFree(int x, int y);

void   printMap(void);
int    isValidMove(int x, int y);
void   movePlayer(int index);
void   processTile(int index);
void   gameLoop(void);

void   saveGame(void);
int    loadGame(void);

void   showScores(void);
void   showStats(void);
int    remainingTreasures(void);
int    allEliminated(void);

void   addLog(int r, const char *m);
void   printRecentLog(void);
void   saveLog(void);

void   readLine(char *buf, int size);
void   flushStdin(void);
int    promptPlayerCount(void);
void   setupNewGame(void);
int    mainMenu(void);


int inputEOF = 0; 

void flushStdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* discard */ }
}

void readLine(char *buf, int size) {
    if (fgets(buf, size, stdin) == NULL) { buf[0] = '\0'; inputEOF = 1; return; }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else {
        
        flushStdin();
    }
}


int tileIsFree(int x, int y) {
    return map[x][y] == SYM_EMPTY && hiddenTrap[x][y] == 0;
}

int placeRandomSymbol(char symbol) {
   
    (void)symbol;
    return 0;
}

void initializeMap(void) {
    int r, c;
    for (r = 0; r < GRID_SIZE; r++) {
        for (c = 0; c < GRID_SIZE; c++) {
            hiddenTrap[r][c] = 0;
            if (r == 0 || r == GRID_SIZE - 1 || c == 0 || c == GRID_SIZE - 1) {
                map[r][c] = SYM_WALL;              /* border walls */
            } else {
                map[r][c] = SYM_EMPTY;
            }
        }
    }

    placeWalls();
    placeTreasures();
    placeTraps();
    placeHealthPacks();
    placeKeys();
    placeDoors();
    placePlayers();
}

void placeWalls(void) {
    int placed = 0;
    while (placed < WALLS) {
        int r = 1 + rand() % (GRID_SIZE - 2);
        int c = 1 + rand() % (GRID_SIZE - 2);
        if (tileIsFree(r, c)) {
            map[r][c] = SYM_WALL;
            placed++;
        }
    }
}

void placeTreasures(void) {
    int placed = 0;
    while (placed < TREASURES) {
        int r = 1 + rand() % (GRID_SIZE - 2);
        int c = 1 + rand() % (GRID_SIZE - 2);
        if (tileIsFree(r, c)) {
            map[r][c] = SYM_TREASURE;
            placed++;
        }
    }
}

void placeTraps(void) {
    int placed = 0;
    while (placed < TRAPS) {
        int r = 1 + rand() % (GRID_SIZE - 2);
        int c = 1 + rand() % (GRID_SIZE - 2);
        if (tileIsFree(r, c)) {
            hiddenTrap[r][c] = 1;   /* traps are never drawn on the map */
            placed++;
        }
    }
}

void placeHealthPacks(void) {
    int placed = 0;
    while (placed < HEALTH_PACKS) {
        int r = 1 + rand() % (GRID_SIZE - 2);
        int c = 1 + rand() % (GRID_SIZE - 2);
        if (tileIsFree(r, c)) {
            map[r][c] = SYM_HEALTH;
            placed++;
        }
    }
}

void placeKeys(void) {
    int placed = 0;
    while (placed < KEYS) {
        int r = 1 + rand() % (GRID_SIZE - 2);
        int c = 1 + rand() % (GRID_SIZE - 2);
        if (tileIsFree(r, c)) {
            map[r][c] = SYM_KEY;
            placed++;
        }
    }
}

void placeDoors(void) {
    int placed = 0;
    while (placed < DOORS) {
        int r = 1 + rand() % (GRID_SIZE - 2);
        int c = 1 + rand() % (GRID_SIZE - 2);
        if (tileIsFree(r, c)) {
            map[r][c] = SYM_DOOR;
            placed++;
        }
    }
}

void placePlayers(void) {
    int i;
    for (i = 0; i < playerCount; i++) {
        int r, c;
        do {
            r = 1 + rand() % (GRID_SIZE - 2);
            c = 1 + rand() % (GRID_SIZE - 2);
        } while (!tileIsFree(r, c));

        players[i].x = r;
        players[i].y = c;
        map[r][c] = players[i].symbol;
    }
}


void printMap(void) {
    int r, c, i;

    printf("\n============================================================\n");
    printf(" QUEST FOR THE LOST TREASURE       Round: %d\n", roundCounter);
    printf("============================================================\n\n");

  
    printf("    ");
    for (c = 0; c < GRID_SIZE; c++) printf("%3d", c);
    printf("\n");

    for (r = 0; r < GRID_SIZE; r++) {
        printf("%3d ", r);
        for (c = 0; c < GRID_SIZE; c++) {
            printf("%3c", map[r][c]);
        }
        printf("\n");
    }

    printf("\nLegend: # Wall  T Treasure  H Health Pack  K Key  D Door"
           "  1/2/3 Player\n");

    printf("\n--- Player Status ---\n");
    for (i = 0; i < playerCount; i++) {
        printf(" [%c] %-10s HP:%3d  Score:%3d  Keys:%d  %s\n",
               players[i].symbol, players[i].name,
               players[i].health, players[i].score, players[i].keys,
               players[i].health <= 0 ? "(ELIMINATED)" : "");
    }

    printRecentLog();
    printf("\n");
}

void addLog(int r, const char *m) {
    int idx = logCount % LOG_SIZE;
    eventLog[idx].round = r;
    snprintf(eventLog[idx].message, sizeof(eventLog[idx].message), "%s", m);
    logCount++;
}

void printRecentLog(void) {
    if (logCount == 0) return;

    int total = logCount < LOG_SIZE ? logCount : LOG_SIZE;
    int toShow = total < LOG_DISPLAY ? total : LOG_DISPLAY;

    printf("\n--- Recent Events ---\n");
    for (int i = toShow; i >= 1; i--) {
        int idx = (logCount - i) % LOG_SIZE;
        if (idx < 0) idx += LOG_SIZE;
        printf(" [R%d] %s\n", eventLog[idx].round, eventLog[idx].message);
    }
}

void saveLog(void) {
    FILE *fp = fopen(LOG_FILE, "w");
    if (!fp) {
        printf("Warning: could not write %s\n", LOG_FILE);
        return;
    }
    int total = logCount < LOG_SIZE ? logCount : LOG_SIZE;
    for (int i = total; i >= 1; i--) {
        int idx = (logCount - i) % LOG_SIZE;
        if (idx < 0) idx += LOG_SIZE;
        fprintf(fp, "[R%d] %s\n", eventLog[idx].round, eventLog[idx].message);
    }
    fclose(fp);
    printf("Event log saved to %s\n", LOG_FILE);
}

int isValidMove(int x, int y) {
    if (x < 0 || x >= GRID_SIZE || y < 0 || y >= GRID_SIZE) return 0;
    if (map[x][y] == SYM_WALL) return 0;
    return 1;
}

void processTile(int index) {
    Player *p = &players[index];
    char logMsg[100];

    /* 1. Hidden trap check */
    if (hiddenTrap[p->x][p->y] == 1) {
        p->health -= 20;
        if (p->health < 0) p->health = 0;
        p->trapsTriggered++;
        p->damageTaken += 20;
        hiddenTrap[p->x][p->y] = 0;
        snprintf(logMsg, sizeof(logMsg), "%s stepped on a hidden trap! (-20 HP)", p->name);
        addLog(roundCounter, logMsg);
        printf(">> %s\n", logMsg);
        if (p->health == 0) {
            snprintf(logMsg, sizeof(logMsg), "%s has been eliminated!", p->name);
            addLog(roundCounter, logMsg);
            printf(">> %s\n", logMsg);
            return; /* no further effects once eliminated */
        }
    }

    /* 2. Treasure check */
    if (map[p->x][p->y] == SYM_TREASURE) {
        p->score += 10;
        p->treasuresFound++;
        map[p->x][p->y] = SYM_EMPTY;
        snprintf(logMsg, sizeof(logMsg), "%s found a treasure! (+10 score)", p->name);
        addLog(roundCounter, logMsg);
        printf(">> %s\n", logMsg);
    }

    /* 3. Health pack check */
    if (map[p->x][p->y] == SYM_HEALTH) {
        p->health += 20;
        if (p->health > MAX_HP) p->health = MAX_HP;
        p->healthPacksUsed++;
        map[p->x][p->y] = SYM_EMPTY;
        snprintf(logMsg, sizeof(logMsg), "%s used a health pack! (+20 HP)", p->name);
        addLog(roundCounter, logMsg);
        printf(">> %s\n", logMsg);
    }

    /* 4. Key check */
    if (map[p->x][p->y] == SYM_KEY) {
        p->keys += 1;
        p->keysCollected++;
        map[p->x][p->y] = SYM_EMPTY;
        snprintf(logMsg, sizeof(logMsg), "%s picked up a key!", p->name);
        addLog(roundCounter, logMsg);
        printf(">> %s\n", logMsg);
    }
}

/* Attempts one single-step move (dx,dy). Handles walls, bounds and doors,
   then updates the map and calls processTile. */
void tryMove(int index, int dx, int dy) {
    Player *p = &players[index];
    int nx = p->x + dx;
    int ny = p->y + dy;
    char logMsg[100];

    if (!isValidMove(nx, ny)) {
        printf(">> Move blocked (wall or out of bounds).\n");
        return;
    }

    /* Prevent two players from occupying the same tile - the map array
       only stores one character per cell, so an overlap would corrupt
       the grid the moment either player later steps away. */
    if (map[nx][ny] >= '1' && map[nx][ny] < (char)('1' + playerCount)) {
        printf(">> Tile occupied by another player. Move blocked.\n");
        return;
    }

    if (map[nx][ny] == SYM_DOOR) {
        if (p->keys > 0) {
            p->keys -= 1;
            p->doorsUnlocked++;
            map[nx][ny] = SYM_EMPTY;   /* door becomes passable */
            snprintf(logMsg, sizeof(logMsg), "%s unlocked a door!", p->name);
            addLog(roundCounter, logMsg);
            printf(">> %s\n", logMsg);
        } else {
            printf(">> Door is locked and you have no keys. Move blocked.\n");
            return;
        }
    }

    /* clear old tile, move player */
    map[p->x][p->y] = SYM_EMPTY;
    p->x = nx;
    p->y = ny;
    p->movesMade++;

    processTile(index);

    /* redraw player symbol at new position (only if still alive) */
    if (p->health > 0) {
        map[p->x][p->y] = p->symbol;
    }
}

void movePlayer(int index) {
    Player *p = &players[index];
    char input[64];

    if (p->health <= 0) {
        printf("%s has 0 HP and is skipped this turn.\n", p->name);
        return;
    }

    printf("\n%s's turn (HP:%d, Score:%d, Keys:%d)\n", p->name, p->health, p->score, p->keys);
    printf("Enter up to %d moves (W/A/S/D): ", MAX_MOVE_LEN);
    readLine(input, sizeof(input));

    int len = (int)strlen(input);
    if (len > MAX_MOVE_LEN) {
        printf(">> More than %d characters entered. Turn cancelled.\n", MAX_MOVE_LEN);
        return;
    }

    for (int i = 0; i < len; i++) {
        if (p->health <= 0) break;  /* eliminated mid-turn */

        char mv = toupper((unsigned char)input[i]);
        int dx = 0, dy = 0;
        switch (mv) {
            case 'W': dx = -1; dy = 0; break;
            case 'S': dx = 1;  dy = 0; break;
            case 'A': dx = 0;  dy = -1; break;
            case 'D': dx = 0;  dy = 1; break;
            default:
                printf(">> Invalid move character '%c' skipped.\n", input[i]);
                continue;
        }
        tryMove(index, dx, dy);
    }
}

int remainingTreasures(void) {
    int r, c, count = 0;
    for (r = 0; r < GRID_SIZE; r++)
        for (c = 0; c < GRID_SIZE; c++)
            if (map[r][c] == SYM_TREASURE) count++;
    return count;
}

int allEliminated(void) {
    for (int i = 0; i < playerCount; i++)
        if (players[i].health > 0) return 0;
    return 1;
}

void showScores(void) {
    int order[MAX_PLAYERS];
    for (int i = 0; i < playerCount; i++) order[i] = i;

    /* Apply HP bonus to surviving players */
    for (int i = 0; i < playerCount; i++) {
        if (players[i].health > 0) {
            int bonus = players[i].health / 2;   /* floor(HP/2) */
            players[i].score += bonus;
            printf("%s receives a survival bonus of +%d score.\n", players[i].name, bonus);
        }
    }

    /* Simple selection sort, descending by score */
    for (int i = 0; i < playerCount - 1; i++) {
        int best = i;
        for (int j = i + 1; j < playerCount; j++) {
            if (players[order[j]].score > players[order[best]].score) best = j;
        }
        if (best != i) {
            int tmp = order[i];
            order[i] = order[best];
            order[best] = tmp;
        }
    }

    printf("\n==================== LEADERBOARD ====================\n");
    for (int i = 0; i < playerCount; i++) {
        Player *p = &players[order[i]];
        printf(" %d. %-10s  Score:%3d  %s\n",
               i + 1, p->name, p->score, p->health <= 0 ? "(eliminated)" : "");
    }
    printf("======================================================\n");

    /* Winner / tie detection */
    int topScore = players[order[0]].score;
    int tieCount = 0;
    for (int i = 0; i < playerCount; i++)
        if (players[order[i]].score == topScore) tieCount++;

    if (tieCount > 1) {
        printf("Result: TIE between %d players with a score of %d!\n", tieCount, topScore);
    } else {
        printf("Winner: %s with a score of %d!\n", players[order[0]].name, topScore);
    }
}

void showStats(void) {
    printf("\n================= PLAYER STATISTICS =================\n");
    printf("%-10s %6s %6s %6s %6s %6s %6s %6s\n",
           "Name", "Moves", "Treas", "Traps", "Dmg", "HPacks", "Keys", "Doors");
    for (int i = 0; i < playerCount; i++) {
        Player *p = &players[i];
        printf("%-10s %6d %6d %6d %6d %6d %6d %6d\n",
               p->name, p->movesMade, p->treasuresFound, p->trapsTriggered,
               p->damageTaken, p->healthPacksUsed, p->keysCollected, p->doorsUnlocked);
    }
    printf("=======================================================\n");
}

void saveGame(void) {
    FILE *fp = fopen(SAVE_FILE, "wb");
    if (!fp) {
        printf("Error: could not open %s for writing.\n", SAVE_FILE);
        return;
    }

    fwrite(&playerCount, sizeof(int), 1, fp);
    fwrite(players, sizeof(Player), MAX_PLAYERS, fp);
    fwrite(map, sizeof(char), GRID_SIZE * GRID_SIZE, fp);
    fwrite(hiddenTrap, sizeof(int), GRID_SIZE * GRID_SIZE, fp);
    fwrite(&roundCounter, sizeof(int), 1, fp);
    fwrite(&logCount, sizeof(int), 1, fp);
    fwrite(eventLog, sizeof(LogEntry), LOG_SIZE, fp);

    fclose(fp);
    printf("Game saved to %s\n", SAVE_FILE);
}

/* Returns 1 on success, 0 if the file is missing/invalid */
int loadGame(void) {
    FILE *fp = fopen(SAVE_FILE, "rb");
    if (!fp) return 0;

    int ok = 1;
    int tmpPlayerCount;

    if (fread(&tmpPlayerCount, sizeof(int), 1, fp) != 1) ok = 0;
    if (ok && (tmpPlayerCount < 1 || tmpPlayerCount > MAX_PLAYERS)) ok = 0;

    if (ok) {
        playerCount = tmpPlayerCount;
        if (fread(players, sizeof(Player), MAX_PLAYERS, fp) != MAX_PLAYERS) ok = 0;
    }
    if (ok && fread(map, sizeof(char), GRID_SIZE * GRID_SIZE, fp) != (size_t)(GRID_SIZE * GRID_SIZE)) ok = 0;
    if (ok && fread(hiddenTrap, sizeof(int), GRID_SIZE * GRID_SIZE, fp) != (size_t)(GRID_SIZE * GRID_SIZE)) ok = 0;
    if (ok && fread(&roundCounter, sizeof(int), 1, fp) != 1) ok = 0;
    if (ok && fread(&logCount, sizeof(int), 1, fp) != 1) ok = 0;
    if (ok && fread(eventLog, sizeof(LogEntry), LOG_SIZE, fp) != LOG_SIZE) ok = 0;

    fclose(fp);

    if (!ok) {
        printf("Save file missing or invalid. Starting a fresh game.\n");
        return 0;
    }

    printf("Game loaded from %s\n", SAVE_FILE);
    return 1;
}

void gameLoop(void) {
    char resp[8];

    while (remainingTreasures() > 0 && !allEliminated()) {
        printMap();

        for (int i = 0; i < playerCount; i++) {
            if (remainingTreasures() == 0 || allEliminated()) break;
            movePlayer(i);
            if (inputEOF) break;
        }

        roundCounter++;

        if (inputEOF) {
            printf("\nInput closed - ending session.\n");
            break;
        }
        if (remainingTreasures() == 0 || allEliminated()) break;

        printf("\nSave game before continuing? (y/n): ");
        readLine(resp, sizeof(resp));
        if (resp[0] == 'y' || resp[0] == 'Y') {
            saveGame();
        }
        if (inputEOF) break;
    }

    /* End of game */
    printMap();
    if (remainingTreasures() == 0) {
        printf("\n*** All treasures have been collected! Game over. ***\n");
    } else if (allEliminated()) {
        printf("\n*** All players have been eliminated! Game over. ***\n");
    } else {
        printf("\n*** Session ended early (input closed). ***\n");
    }

    showScores();
    showStats();
    saveLog();
}

int promptPlayerCount(void) {
    char buf[16];
    int n;
    while (1) {
        printf("How many players (1-3)? ");
        readLine(buf, sizeof(buf));
        n = atoi(buf);
        if (n >= 1 && n <= 3) return n;
        printf("Invalid input. Please enter 1, 2, or 3.\n");
    }
}

void setupNewGame(void) {
    char nameBuf[30];

    playerCount = promptPlayerCount();

    for (int i = 0; i < playerCount; i++) {
        printf("Enter name for Player %d (leave blank for default): ", i + 1);
        readLine(nameBuf, sizeof(nameBuf));
        if (strlen(nameBuf) == 0) {
            snprintf(players[i].name, sizeof(players[i].name), "Player%d", i + 1);
        } else {
            snprintf(players[i].name, sizeof(players[i].name), "%s", nameBuf);
        }
        players[i].score = 0;
        players[i].health = MAX_HP;
        players[i].keys = 0;
        players[i].symbol = (char)('1' + i);
        players[i].movesMade = 0;
        players[i].treasuresFound = 0;
        players[i].trapsTriggered = 0;
        players[i].damageTaken = 0;
        players[i].healthPacksUsed = 0;
        players[i].keysCollected = 0;
        players[i].doorsUnlocked = 0;
    }

    roundCounter = 1;
    logCount = 0;

    initializeMap();
    printf("\nA new map has been generated. Good luck!\n");
}

int mainMenu(void) {
    char buf[16];
    printf("\n==========================================\n");
    printf("      QUEST FOR THE LOST TREASURE\n");
    printf("==========================================\n");
    printf(" 1. New Game\n");
    printf(" 2. Load Game\n");
    printf(" 3. Exit\n");
    printf("Choose an option: ");
    readLine(buf, sizeof(buf));
    if (inputEOF) return 3;
    return atoi(buf);
}

int main(void) {
    srand((unsigned int)time(NULL));
    int choice;

    while (1) {
        choice = mainMenu();

        if (choice == 1) {
            setupNewGame();
            gameLoop();
        } else if (choice == 2) {
            if (!loadGame()) {
                printf("No valid save found. Starting a new game instead.\n");
                setupNewGame();
            }
            gameLoop();
        } else if (choice == 3) {
            printf("Thanks for playing!\n");
            break;
        } else {
            printf("Invalid option, please try again.\n");
            continue;
        }

        if (inputEOF) break;

        printf("\nPlay again? (y/n): ");
        char again[8];
        readLine(again, sizeof(again));
        if (inputEOF || !(again[0] == 'y' || again[0] == 'Y')) {
            printf("Thanks for playing!\n");
            break;
        }
    }

    return 0;
}
