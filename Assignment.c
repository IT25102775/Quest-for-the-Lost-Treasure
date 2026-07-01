
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/* ---------------------------------------------------------------------
   CONSTANTS  (Global Variables & Constants)
   --------------------------------------------------------------------- */
#define MAP_SIZE        15
#define MAX_PLAYERS     3
#define WALLS           30
#define TREASURES       12
#define HEALTH_PACKS    5
#define KEYS            3
#define DOORS           3
#define TRAPS           10
#define MAX_MOVES       4
#define LOG_SIZE        100
#define LOG_DISPLAY     5
#define NAME_LEN        30
#define SAVE_FILE       "savegame.dat"
#define LOG_FILE        "gamelog.txt"

#define SYM_WALL        '#'
#define SYM_TREASURE    'T'
#define SYM_HEALTH      'H'
#define SYM_KEY         'K'
#define SYM_DOOR        'D'
#define SYM_EMPTY       ' '

/* ---------------------------------------------------------------------
   STRUCTS
   --------------------------------------------------------------------- */
typedef struct {
    char name[NAME_LEN];
    int  x, y;
    int  score;
    int  health;
    int  keys;
    char symbol;
    int  active;           /* 1 = still in the game (was ever playing)   */

    /* Extended struct fields -- Part B statistics */
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
    char message[80];
} LogEntry;

/* ---------------------------------------------------------------------
   GLOBAL STATE
   --------------------------------------------------------------------- */
char map[MAP_SIZE][MAP_SIZE];
int  hiddenTrap[MAP_SIZE][MAP_SIZE];   /* 1 = trap present, invisible    */

Player players[MAX_PLAYERS];
int playerCount = 0;

LogEntry eventLog[LOG_SIZE];
int logCount = 0;          /* number of entries used (<= LOG_SIZE)       */
int logNext  = 0;          /* next slot to write (circular)              */

int roundNumber = 1;

/* ---------------------------------------------------------------------
   FUNCTION PROTOTYPES  (Guidance & Proposed Functions)
   --------------------------------------------------------------------- */
void initializeMap(void);
void placeWalls(void);
void placeTreasures(void);
void placeTraps(void);
void placeHealthPacks(void);
void placeKeys(void);
void placeDoors(void);
void placePlayers(void);

void placeSymbolRandomly(char symbol, int count);

void printMap(void);
int  isValidMove(int x, int y);
void movePlayer(int index);
void processTile(int index);
void gameLoop(void);

void saveGame(void);
void loadGame(int *loaded);

void showScores(void);
int  remainingTreasures(void);

void addLog(int r, const char *m);
void printRecentLog(void);
void saveLog(void);
void showStats(void);

int  allEliminated(void);
void trim(char *s);
void readLine(char *buf, int size);
int  emptyTileCount(void);

/* =====================================================================
   MAP INITIALISATION
   ===================================================================== */
void initializeMap(void) {
    int r, c;
    for (r = 0; r < MAP_SIZE; r++)
        for (c = 0; c < MAP_SIZE; c++) {
            hiddenTrap[r][c] = 0;
            if (r == 0 || r == MAP_SIZE - 1 || c == 0 || c == MAP_SIZE - 1)
                map[r][c] = SYM_WALL;
            else
                map[r][c] = SYM_EMPTY;
        }

    placeWalls();
    placeTraps();
    placeTreasures();
    placeHealthPacks();
    placeKeys();
    placeDoors();
}

/* Count interior empty tiles -- used to guard against infinite loops if
   the board somehow fills up. */
int emptyTileCount(void) {
    int r, c, n = 0;
    for (r = 1; r < MAP_SIZE - 1; r++)
        for (c = 1; c < MAP_SIZE - 1; c++)
            if (map[r][c] == SYM_EMPTY && !hiddenTrap[r][c]) n++;
    return n;
}

/* Generic helper: scatter `count` copies of `symbol` on random empty
   interior tiles (2-D Arrays, Random Number Generation). */
void placeSymbolRandomly(char symbol, int count) {
    int placed = 0, guard = 0;
    while (placed < count && guard < 100000) {
        int r = 1 + rand() % (MAP_SIZE - 2);
        int c = 1 + rand() % (MAP_SIZE - 2);
        guard++;
        if (map[r][c] == SYM_EMPTY && !hiddenTrap[r][c]) {
            map[r][c] = symbol;
            placed++;
        }
    }
}

void placeWalls(void)      { placeSymbolRandomly(SYM_WALL, WALLS); }
void placeTreasures(void)  { placeSymbolRandomly(SYM_TREASURE, TREASURES); }
void placeHealthPacks(void){ placeSymbolRandomly(SYM_HEALTH, HEALTH_PACKS); }
void placeKeys(void)       { placeSymbolRandomly(SYM_KEY, KEYS); }
void placeDoors(void)      { placeSymbolRandomly(SYM_DOOR, DOORS); }

/* Traps are stored ONLY in hiddenTrap[][]; they are never drawn. */
void placeTraps(void) {
    int placed = 0, guard = 0;
    while (placed < TRAPS && guard < 100000) {
        int r = 1 + rand() % (MAP_SIZE - 2);
        int c = 1 + rand() % (MAP_SIZE - 2);
        guard++;
        if (map[r][c] == SYM_EMPTY && !hiddenTrap[r][c]) {
            hiddenTrap[r][c] = 1;
            placed++;
        }
    }
}

/* Random starting position for every active player, on an empty tile. */
void placePlayers(void) {
    int i;
    for (i = 0; i < playerCount; i++) {
        int guard = 0;
        while (guard < 100000) {
            int r = 1 + rand() % (MAP_SIZE - 2);
            int c = 1 + rand() % (MAP_SIZE - 2);
            guard++;
            if (map[r][c] == SYM_EMPTY && !hiddenTrap[r][c]) {
                players[i].x = r;
                players[i].y = c;
                map[r][c] = players[i].symbol;
                break;
            }
        }
    }
}

/* =====================================================================
   DISPLAY
   ===================================================================== */
void printMap(void) {
    int r, c, i;

    printf("\n=============== ROUND %d ===============\n", roundNumber);
    printf("    ");
    for (c = 0; c < MAP_SIZE; c++) printf("%2d", c);
    printf("\n");

    for (r = 0; r < MAP_SIZE; r++) {
        printf("%2d  ", r);
        for (c = 0; c < MAP_SIZE; c++)
            printf(" %c", map[r][c]);
        printf("\n");
    }

    printf("\n--- Player Status ---\n");
    for (i = 0; i < playerCount; i++) {
        printf("[%c] %-10s HP:%-4d Score:%-5d Keys:%-2d %s\n",
               players[i].symbol, players[i].name,
               players[i].health, players[i].score, players[i].keys,
               players[i].health <= 0 ? "(ELIMINATED)" : "");
    }

    printRecentLog();
}

/* =====================================================================
   MOVEMENT VALIDATION
   ===================================================================== */
int isValidMove(int x, int y) {
    if (x < 0 || x >= MAP_SIZE || y < 0 || y >= MAP_SIZE) return 0;
    if (map[x][y] == SYM_WALL) return 0;
    return 1;
}

/* =====================================================================
   TILE EFFECTS  (order: Trap -> Treasure -> Health Pack -> Key)
   ===================================================================== */
void processTile(int index) {
    Player *p = &players[index];
    int x = p->x, y = p->y;
    char msg[100];

    /* Hidden trap check */
    if (hiddenTrap[x][y]) {
        hiddenTrap[x][y] = 0;
        p->health -= 20;
        p->damageTaken += 20;
        p->trapsTriggered++;
        if (p->health < 0) p->health = 0;
        snprintf(msg, sizeof(msg), "%s hit a hidden trap! (-20 HP)", p->name);
        addLog(roundNumber, msg);
        if (p->health == 0) {
            snprintf(msg, sizeof(msg), "%s has been eliminated!", p->name);
            addLog(roundNumber, msg);
        }
    }

    /* Treasure check */
    if (map[x][y] == SYM_TREASURE) {
        p->score += 10;
        p->treasuresFound++;
        map[x][y] = SYM_EMPTY;
        snprintf(msg, sizeof(msg), "%s found a treasure! (+10 score)", p->name);
        addLog(roundNumber, msg);
    }

    /* Health pack check */
    if (map[x][y] == SYM_HEALTH) {
        p->health += 20;
        if (p->health > 100) p->health = 100;
        p->healthPacksUsed++;
        map[x][y] = SYM_EMPTY;
        snprintf(msg, sizeof(msg), "%s used a health pack! (+20 HP)", p->name);
        addLog(roundNumber, msg);
    }

    /* Key check */
    if (map[x][y] == SYM_KEY) {
        p->keys++;
        p->keysCollected++;
        map[x][y] = SYM_EMPTY;
        snprintf(msg, sizeof(msg), "%s picked up a key!", p->name);
        addLog(roundNumber, msg);
    }
}

/* =====================================================================
   PLAYER TURN
   ===================================================================== */
void movePlayer(int index) {
    Player *p = &players[index];
    char input[64];
    char msg[100];
    int i;

    printf("\n%s (HP:%d, Score:%d, Keys:%d) - enter up to %d moves (WASD): ",
           p->name, p->health, p->score, p->keys, MAX_MOVES);
    readLine(input, sizeof(input));
    trim(input);

    if ((int)strlen(input) > MAX_MOVES) {
        printf(">> More than %d characters entered. Turn cancelled.\n", MAX_MOVES);
        snprintf(msg, sizeof(msg), "%s entered an invalid move string; turn cancelled.", p->name);
        addLog(roundNumber, msg);
        return;
    }

    for (i = 0; i < (int)strlen(input); i++) {
        char c = (char)toupper((unsigned char)input[i]);
        int dx = 0, dy = 0;
        int nx, ny;

        if (c == 'W')      { dx = -1; dy = 0; }
        else if (c == 'S') { dx = 1;  dy = 0; }
        else if (c == 'A') { dx = 0;  dy = -1; }
        else if (c == 'D') { dx = 0;  dy = 1; }
        else {
            printf(">> Warning: '%c' is not a valid move character, skipped.\n", input[i]);
            continue;
        }

        nx = p->x + dx;
        ny = p->y + dy;

        if (!isValidMove(nx, ny)) {
            printf(">> Move '%c' blocked (wall or out of bounds).\n", c);
            continue;
        }

        if (map[nx][ny] == SYM_DOOR) {
            if (p->keys > 0) {
                p->keys--;
                p->doorsUnlocked++;
                map[nx][ny] = SYM_EMPTY;
                snprintf(msg, sizeof(msg), "%s unlocked a door.", p->name);
                addLog(roundNumber, msg);
            } else {
                printf(">> Move '%c' blocked: locked door, no keys.\n", c);
                continue;
            }
        }

        /* commit the move */
        map[p->x][p->y] = SYM_EMPTY;
        p->x = nx;
        p->y = ny;
        p->movesMade++;

        processTile(index);

        /* redraw player symbol at new position (if still alive) */
        map[p->x][p->y] = p->symbol;

        if (p->health <= 0) {
            printf(">> %s has been eliminated!\n", p->name);
            break;   /* stop processing remaining moves in the string */
        }
    }
}

/* =====================================================================
   GAME-END HELPERS
   ===================================================================== */
int remainingTreasures(void) {
    int r, c, n = 0;
    for (r = 0; r < MAP_SIZE; r++)
        for (c = 0; c < MAP_SIZE; c++)
            if (map[r][c] == SYM_TREASURE) n++;
    return n;
}

int allEliminated(void) {
    int i;
    for (i = 0; i < playerCount; i++)
        if (players[i].health > 0) return 0;
    return 1;
}

/* =====================================================================
   EVENT LOG  (Part B) -- circular buffer, Multi-dimensional-style log
   ===================================================================== */
void addLog(int r, const char *m) {
    eventLog[logNext].round = r;
    snprintf(eventLog[logNext].message, sizeof(eventLog[logNext].message), "%s", m);
    logNext = (logNext + 1) % LOG_SIZE;
    if (logCount < LOG_SIZE) logCount++;
}

void printRecentLog(void) {
    int total = logCount;
    int show = total < LOG_DISPLAY ? total : LOG_DISPLAY;
    int i, idx;

    printf("\n--- Recent Events ---\n");
    if (show == 0) {
        printf("(none yet)\n");
        return;
    }
    /* entries are stored circularly; walk backwards from logNext-1 */
    for (i = show - 1; i >= 0; i--) {
        idx = (logNext - 1 - i + LOG_SIZE) % LOG_SIZE;
        printf("[R%d] %s\n", eventLog[idx].round, eventLog[idx].message);
    }
}

void saveLog(void) {
    FILE *f = fopen(LOG_FILE, "w");
    int total = logCount;
    int i, idx, start;

    if (!f) {
        printf("Could not write %s\n", LOG_FILE);
        return;
    }

    start = (logCount < LOG_SIZE) ? 0 : logNext; /* oldest entry index */
    for (i = 0; i < total; i++) {
        idx = (start + i) % LOG_SIZE;
        fprintf(f, "[R%d] %s\n", eventLog[idx].round, eventLog[idx].message);
    }
    fclose(f);
    printf("Event log saved to %s\n", LOG_FILE);
}

/* =====================================================================
   SCORING / STATISTICS
   ===================================================================== */
void showScores(void) {
    int i, j;
    Player order[MAX_PLAYERS];

    for (i = 0; i < playerCount; i++) {
        order[i] = players[i];
        if (order[i].health > 0) {
            int bonus = order[i].health / 2; /* floor(HP/2) */
            order[i].score += bonus;
        }
    }

    /* simple bubble sort by score descending -- Sorting Algorithm */
    for (i = 0; i < playerCount - 1; i++)
        for (j = 0; j < playerCount - 1 - i; j++)
            if (order[j].score < order[j + 1].score) {
                Player tmp = order[j];
                order[j] = order[j + 1];
                order[j + 1] = tmp;
            }

    printf("\n================ LEADERBOARD ================\n");
    for (i = 0; i < playerCount; i++) {
        printf("%d. %-10s (Player %c)  Final Score: %d\n",
               i + 1, order[i].name, order[i].symbol, order[i].score);
    }

    if (playerCount > 1 && order[0].score == order[1].score) {
        printf("\nResult: TIE between the top players!\n");
    } else {
        printf("\nWinner: %s (Player %c)!\n", order[0].name, order[0].symbol);
    }
}

void showStats(void) {
    int i;
    printf("\n================ PLAYER STATISTICS ================\n");
    printf("%-10s %6s %6s %6s %6s %6s %6s %6s\n",
           "Name", "Moves", "Treas", "Traps", "Dmg", "HPacks", "Keys", "Doors");
    for (i = 0; i < playerCount; i++) {
        Player *p = &players[i];
        printf("%-10s %6d %6d %6d %6d %6d %6d %6d\n",
               p->name, p->movesMade, p->treasuresFound, p->trapsTriggered,
               p->damageTaken, p->healthPacksUsed, p->keysCollected, p->doorsUnlocked);
    }
}

/* =====================================================================
   SAVE / LOAD  (File I/O)
   ===================================================================== */
void saveGame(void) {
    FILE *f = fopen(SAVE_FILE, "wb");
    if (!f) {
        printf("Error: could not open %s for writing.\n", SAVE_FILE);
        return;
    }

    fwrite(&playerCount, sizeof(int), 1, f);
    fwrite(&roundNumber, sizeof(int), 1, f);
    fwrite(players, sizeof(Player), MAX_PLAYERS, f);
    fwrite(map, sizeof(char), MAP_SIZE * MAP_SIZE, f);
    fwrite(hiddenTrap, sizeof(int), MAP_SIZE * MAP_SIZE, f);
    fwrite(&logCount, sizeof(int), 1, f);
    fwrite(&logNext, sizeof(int), 1, f);
    fwrite(eventLog, sizeof(LogEntry), LOG_SIZE, f);

    fclose(f);
    printf("Game saved to %s\n", SAVE_FILE);
}

void loadGame(int *loaded) {
    FILE *f = fopen(SAVE_FILE, "rb");
    *loaded = 0;
    if (!f) {
        printf("No valid save file found. Starting a fresh game.\n");
        return;
    }

    if (fread(&playerCount, sizeof(int), 1, f) != 1 ||
        playerCount < 1 || playerCount > MAX_PLAYERS ||
        fread(&roundNumber, sizeof(int), 1, f) != 1 ||
        fread(players, sizeof(Player), MAX_PLAYERS, f) != MAX_PLAYERS ||
        fread(map, sizeof(char), MAP_SIZE * MAP_SIZE, f) != MAP_SIZE * MAP_SIZE ||
        fread(hiddenTrap, sizeof(int), MAP_SIZE * MAP_SIZE, f) != MAP_SIZE * MAP_SIZE ||
        fread(&logCount, sizeof(int), 1, f) != 1 ||
        fread(&logNext, sizeof(int), 1, f) != 1 ||
        fread(eventLog, sizeof(LogEntry), LOG_SIZE, f) != LOG_SIZE) {
        printf("Save file is invalid or corrupt. Starting a fresh game.\n");
        fclose(f);
        return;
    }

    fclose(f);
    *loaded = 1;
    printf("Game loaded successfully from %s\n", SAVE_FILE);
}

/* =====================================================================
   SMALL UTILITIES  (String Handling)
   ===================================================================== */
void readLine(char *buf, int size) {
    if (fgets(buf, size, stdin) == NULL) {
        /* Input stream closed (e.g. Ctrl+D) -- exit gracefully instead
           of spinning in a busy loop reading empty input forever. */
        printf("\nInput closed. Exiting.\n");
        exit(0);
    }
}

void trim(char *s) {
    int len = (int)strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r' || s[len - 1] == ' ')) {
        s[len - 1] = '\0';
        len--;
    }
}

/* =====================================================================
   MAIN GAME LOOP
   ===================================================================== */
void gameLoop(void) {
    char saveChoice[16];
    int i;

    while (1) {
        printMap();

        for (i = 0; i < playerCount; i++) {
            if (players[i].health <= 0) {
                printf("\n%s is eliminated and is skipped this round.\n", players[i].name);
                continue;
            }
            movePlayer(i);

            if (remainingTreasures() == 0) {
                printf("\nAll treasures have been collected!\n");
                goto endgame;
            }
            if (allEliminated()) {
                printf("\nAll players have been eliminated!\n");
                goto endgame;
            }
        }

        roundNumber++;

        printf("\nSave game? (y/n): ");
        readLine(saveChoice, sizeof(saveChoice));
        trim(saveChoice);
        if (saveChoice[0] == 'y' || saveChoice[0] == 'Y') {
            saveGame();
        }
    }

endgame:
    printf("\n================ GAME OVER ================\n");
    showScores();
    showStats();
    saveLog();
}

/* =====================================================================
   MAIN MENU
   ===================================================================== */
int main(void) {
    char choice[16];
    char buf[64];
    int loaded = 0;

    srand((unsigned int)time(NULL));

    printf("=========================================\n");
    printf("      QUEST FOR THE LOST TREASURE\n");
    printf("=========================================\n");

    while (1) {
        printf("\nMain Menu:\n");
        printf("1. New Game\n");
        printf("2. Load Game\n");
        printf("3. Exit\n");
        printf("Choose an option: ");
        readLine(choice, sizeof(choice));
        trim(choice);

        if (strcmp(choice, "1") == 0) {
            int n = 0, i;
            do {
                printf("Enter number of players (1-3): ");
                readLine(buf, sizeof(buf));
                trim(buf);
                n = atoi(buf);
            } while (n < 1 || n > 3);

            playerCount = n;
            roundNumber = 1;
            logCount = 0;
            logNext = 0;

            for (i = 0; i < playerCount; i++) {
                printf("Enter name for Player %d: ", i + 1);
                readLine(buf, sizeof(buf));
                trim(buf);
                if (strlen(buf) == 0) snprintf(buf, sizeof(buf), "Player%d", i + 1);
                buf[NAME_LEN - 1] = '\0';
                snprintf(players[i].name, NAME_LEN, "%s", buf);
                players[i].score = 0;
                players[i].health = 100;
                players[i].keys = 0;
                players[i].symbol = (char)('1' + i);
                players[i].active = 1;
                players[i].movesMade = 0;
                players[i].treasuresFound = 0;
                players[i].trapsTriggered = 0;
                players[i].damageTaken = 0;
                players[i].healthPacksUsed = 0;
                players[i].keysCollected = 0;
                players[i].doorsUnlocked = 0;
            }
            for (i = playerCount; i < MAX_PLAYERS; i++) {
                memset(&players[i], 0, sizeof(Player));
            }

            initializeMap();
            placePlayers();
            gameLoop();
        } else if (strcmp(choice, "2") == 0) {
            loadGame(&loaded);
            if (loaded) {
                gameLoop();
            }
        } else if (strcmp(choice, "3") == 0) {
            printf("Goodbye!\n");
            break;
        } else {
            printf("Invalid option, please try again.\n");
        }
    }

    return 0;
}
