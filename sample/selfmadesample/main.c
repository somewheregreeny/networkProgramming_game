#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <winsock2.h>
#include <windows.h>
#include "console.h"

#define WIDTH 81   
#define HEIGHT 50

#define BULLET_MAX 50
#define ENEMY1_MAX 3
#define ENEMY2_MAX 6

char screen[HEIGHT][WIDTH];
char title[HEIGHT][WIDTH] = {
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n", // 14,33 
     "                                                                               \n",
     "                                                                               \n", // 16,33
     "                                                                               \n", // 17,34 
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n",
     "                                                                               \n"
};

char playerCursor[3] = "->";
char playerUnit[6] = "=|^|=";
char playerBullet = '|';

char enemyUnit[8] = "-< o >-";
char enemyUnit2[6] = "-|O|-";
char enemyBullet = '@';

char playerScore[9] = "Score : ";
char playerTime[8] = "Time : ";

unsigned int realscore = 0;
float realtime;
int difficulty = 1;

int level = -2;             // -2 : 메인화면 -1 : 방어자 공격자 선택 0 : 로딩화면 1~ : 게임시작

struct cursorState {
    int x, y;
};

struct PlayerState {
    int x, y;
    int isLive;
};

struct Enemy1State{
    int x, y;
    int isLive;
};
struct Enemy2State {
    int x, y;
    int isLive;
};

struct BulletState {
    int x, y;
    int isLive;
};

struct BulletStateE {
    int x, y;
    int isLive;
};

struct cursorState cursor;
struct PlayerState player;
struct Enemy1State enemy1[ENEMY1_MAX];
struct Enemy2State enemy2[ENEMY2_MAX];
struct BulletState bullet[BULLET_MAX];
struct BulletStateE bulletE[ENEMY2_MAX];

void waitSec(int time);
void errorDisplay(char* str);

void cursorInit();
void cursorAction();
int cursorSelect();
void playerInit();
void playerAction();
void enemyInit();
void enemyMove();

void createBullet();
void createBulletE();
void bulletAction();
void bulletHitCheck();
void playerHitCheck();

void draw();

void drawCursor();
void drawMain();
void drawSelect();
void drawLoading();

void drawPlayer();
void drawEnemy();
void drawBullet();
void drawScore();
void drawTime();

int endCheck();
int startCheck();

int main() {

    srand(time(NULL));

    system("mode con cols=81 lines=50");
    Initial();
    cursorInit();


    while (level == -2) {                       // 메인화면
        cursorAction();
        drawMain();
        if (cursorSelect() == 1) level = 1;
        else if (cursorSelect() == 2) break;
        Sleep(100);
    }

    playerInit();
    enemyInit();
    
    while (level == 1) {    // 방어자
        playerAction();
        bulletAction();
        enemyMove();
        bulletHitCheck();
        playerHitCheck();
        draw();
        if (endCheck()) {
            break;
        }
        Sleep(50);
        realtime += 1;
        if (realtime == 500) {
            difficulty = 2;
        }
    }

}

void waitSec(int time) {
    clock_t start_clk = clock();

    time--;
    while (1) {
        if ((clock() - start_clk) / CLOCKS_PER_SEC > time) break;
    }
}

void errorDisplay(char* str)
{
    printf("<ERROR> %s!!!\n", str);
    exit(-1);
}

void cursorInit() {
    cursor.x = 28;
    cursor.y = 16;
}

void cursorAction() {

    if ((GetAsyncKeyState(VK_UP) & 0x8000)){
        if (cursor.y > 16){
            cursor.y--;
        }
    }
    if ((GetAsyncKeyState(VK_DOWN) & 0x8000)){
        if (cursor.y <= 16){
            cursor.y++;
        }
    }
}

int cursorSelect() {
    if ((GetAsyncKeyState(VK_RETURN) & 0x8000)){
        if (cursor.y == 16){
            return 1;
        }
        else if (cursor.y == 17){
            return 2;
        }
    }
}

void playerInit() {
    player.x = 39;
    player.y = 46;
    player.isLive = 1;

}

void playerAction() {            
    if (player.isLive){
        if ((GetAsyncKeyState(VK_RIGHT) & 0x8000)){
            if (player.x < WIDTH - 4){
                player.x += 4;
            }
        }

        if ((GetAsyncKeyState(VK_LEFT) & 0x8000)){
            if (player.x > WIDTH - 78){
                player.x -= 4;
            }
        }

        if ((GetAsyncKeyState(VK_SPACE) & 0x8000)){
            createBullet(player.x, player.y);
        }
    }
}

void enemyInit() {
    int random;
    random = rand() % 73;

    for (int i = 0; i < ENEMY1_MAX; i++) {
        enemy1[i].x = random;
        enemy1[i].y = 0;
        enemy1[i].isLive = 1;
    }

    random = rand() % 73;

    for (int i = 0; i < ENEMY2_MAX; i++) {
        enemy2[i].x = random;
        enemy2[i].y = 0;
        enemy2[i].isLive = 1;
    }
}

void enemyMove() {

    int random1, random2;

    for (int i = 0; i < ENEMY1_MAX; i++) {
        if (enemy1[i].isLive == 1) {
            random1 = rand() % 4;
            random2 = rand() % 2;

            if (random2 == 0) {
                if((enemy1[i].x - random1) >= 0) 
                    enemy1[i].x -= random1;
                else 
                    enemy1[i].x += random1;
            }
            else if (random2 == 1) {
                if ((enemy1[i].x + random1) <= 73) 
                    enemy1[i].x += random1;
                else
                    enemy1[i].x -= random1;
            }

            enemy1[i].y += 1;
            
            if (enemy1[i].y > 47) enemy1[i].y = 0;
            if (rand() % 100 < 5)
                createBulletE(enemy1[i].x, enemy1[i].y);
        }
    }

    if (level == 2) {
        for (int i = 0; i < ENEMY2_MAX; i++) {
            if (enemy2[i].isLive == 1) {
                random1 = rand() % 4;
                random2 = rand() % 2;

                if (random2 == 0) {
                    if ((enemy2[i].x - random1) >= 0)
                        enemy2[i].x -= random1;
                    else
                        enemy2[i].x += random1;
                }
                else if (random2 == 1) {
                    if ((enemy2[i].x + random1) <= 73)
                        enemy2[i].x += random1;
                    else
                        enemy2[i].x -= random1;
                }

                enemy2[i].y += 2;

                if (enemy2[i].y > 47) enemy2[i].y = 0;
            }
        }
    }
}

void createBullet(int x, int y) {

    for (int i = 0; i < BULLET_MAX; i++) {
        if (bullet[i].isLive == 0) {
            bullet[i].x = x;
            bullet[i].y = y;
            bullet[i].isLive = 1;
            return;
        }
    }
}

void createBulletE(int x, int y) {
    for (int i = 0; i < ENEMY2_MAX; i++) {
        if (bulletE[i].isLive == 0) {
            bulletE[i].x = x;
            bulletE[i].y = y;
            bulletE[i].isLive = 1;
            return;
        }
    }
}


void bulletAction() {

    for (int i = 0; i < BULLET_MAX; i++) {
        if (bullet[i].isLive == 1) {
            bullet[i].y -= 1;
            if (bullet[i].y <= 0) 
                bullet[i].isLive = 0;
        }
    }

    for (int i = 0; i < ENEMY2_MAX; i++) {
        if (bulletE[i].isLive == 1) {
            bulletE[i].y += 2;
            if (bulletE[i].y >=  50)
                bulletE[i].isLive = 0;
        }
    }

}

void bulletHitCheck() {

    for (int i = 0; i < BULLET_MAX; i++) {
        for (int j = 0; j < ENEMY1_MAX; j++) {
            if (bullet[i].isLive) {
                if (enemy1[j].isLive) {
                    if (((enemy1[j].x - 3) <= bullet[i].x) && ((enemy1[j].x + 3) >= bullet[i].x) && (enemy1[j].y == bullet[i].y)) {
                        enemy1[j].isLive = 0;
                        bullet[i].isLive = 0;
                        realscore += 100;
                        break;
                    }
                }
            }
        }
    }

    if (level == 2) {
        for (int i = 0; i < BULLET_MAX; i++) {
            for (int j = 0; j < ENEMY2_MAX; j++) {
                if (bullet[i].isLive) {
                    if (enemy2[j].isLive) {
                        if (((enemy2[j].x - 3) <= bullet[i].x) && ((enemy2[j].x + 3) >= bullet[i].x) && (enemy2[j].y == bullet[i].y)) {
                            enemy2[j].isLive = 0;
                            bullet[i].isLive = 0;
                            realscore += 200;
                            break;
                        }
                    }
                }
            }
        }
    }
}

void playerHitCheck() {
    for (int j = 0; j < ENEMY1_MAX; j++) {
        if (enemy1[j].isLive) {
            if (((enemy1[j].x - 3) <= player.x) && ((enemy1[j].x + 3) >= player.x) && (enemy1[j].y == player.y))
                player.isLive = 0;
        }
    }

    if (level == 2) {
        for (int j = 0; j < ENEMY2_MAX; j++) {
            if (enemy2[j].isLive) {
                if (((enemy2[j].x - 3) <= player.x) && ((enemy2[j].x + 3) >= player.x) && (enemy2[j].y == player.y))
                    player.isLive = 0;
            }
        }
    }
}

void draw() {
    int i;
    for (i = 0; i < HEIGHT; i++)            
    {
        memset(screen[i], ' ', WIDTH);
        screen[i][WIDTH - 1] = NULL;            
    }

    drawPlayer();
    drawBullet();
    drawScore();
    drawTime();
    drawEnemy();

    for (i = 0; i < HEIGHT; i++)
    {
        MoveCursor(0, i);        //  커서의 시작 지점 즉 y축을 바꾸어주며 한줄단위로 출력
        printf(screen[i]);

        MoveCursor(8, 48);
        printf("%d", realscore); // 실시간 점수 출력

        MoveCursor(8, 49);
        printf("%.f", realtime); // 실시간 시간 출력
    }
}

void drawCursor() {
    int x = cursor.x;
    int y = cursor.y;

    for (int i = 0; i < 2; i++)
    {
        title[y][x++] = playerCursor[i];
    }
}

void drawMain() {

    for (int i = 0; i < HEIGHT; i++)
    {
        strcpy(title[14], "                                비행기 게임                                    \n");
        strcpy(title[16], "                                Game start                                     \n");
        strcpy(title[17], "                                 End game                                      \n");
        title[i] [WIDTH - 1] = NULL;
    }

    drawCursor();

    for (int i = 0; i < HEIGHT; i++)
    {
        MoveCursor(0, i);
        printf(title[i]);
    }
}

void drawSelect() {
    for (int i = 0; i < HEIGHT; i++)
    {
        strcpy(title[14], "                                역할 선택                                      \n");
        strcpy(title[16], "                                 공격자                                        \n");
        strcpy(title[17], "                                 방어자                                        \n");
        title[i][WIDTH - 1] = NULL;
    }

    drawCursor();

    for (int i = 0; i < HEIGHT; i++)
    {
        MoveCursor(0, i);
        printf(title[i]);
    }
}

void drawPlayer() {        // 플레이어 비행기 좌표 지정

    int x = player.x - 5 / 2;
    int y = player.y;

    if (y < 0 || y >= HEIGHT)        // y값이 화면 밖으로나가면 오류로 실행종료
        return;

    if (player.isLive == 1)
    {
        for (int i = 0; i < 5; i++)
        {
            if (x >= 0 && x < WIDTH - 1)
                screen[y][x++] = playerUnit[i];
        }

    }
}

void drawEnemy() {

    int random;

    int x, y;

    for (int i = 0; i < ENEMY1_MAX; i++) {
        if (enemy1[i].isLive == 1 && enemy1[i].y == 0) {
           random = rand() % 73;
           enemy1[i].x = random;
        }
        if (enemy1[i].isLive == 0) {
            random = rand() % 73;
            enemy1[i].x = random;
            enemy1[i].y = 0;
            enemy1[i].isLive = 1;
        }

        x = enemy1[i].x;
        y = enemy1[i].y;

        for (int j = 0; j < 7; j++) {
            screen[y][x++] = enemyUnit[j];
        }
        
    }

    if (level == 2) {
        for (int i = 0; i < ENEMY2_MAX; i++) {
            if (enemy2[i].isLive == 1 && enemy2[i].y == 0) {
                random = rand() % 73;
                enemy2[i].x = random;
            }
            if (enemy2[i].isLive == 0) {
                random = rand() % 73;
                enemy2[i].x = random;
                enemy2[i].y = 0;
                enemy2[i].isLive = 1;
            }

            x = enemy2[i].x;
            y = enemy2[i].y;

            for (int j = 0; j < 6; j++) {
                screen[y][x++] = enemyUnit2[j];
            }

        }
    }


}

void drawBullet() {

    for (int i = 0; i < BULLET_MAX; i++) {
        if (bullet[i].isLive == 1) {
            screen[bullet[i].y][bullet[i].x] = playerBullet;
        }
    }

    for (int i = 0; i < ENEMY2_MAX; i++) {
        if (bulletE[i].isLive == 1) {
            screen[bulletE[i].y][bulletE[i].x] = enemyBullet;
        }
    }
}

void drawScore() {
    int x = 0;
    int y = 48;

    for (int i = 0; i < sizeof(playerScore); i++) {
        screen[y][x++] = playerScore[i];
    }
}

void drawTime() {
    int x = 0;
    int y = 49;

    for (int i = 0; i < sizeof(playerTime); i++) {
        screen[y][x++] = playerTime[i];
    }
}

int endCheck() {
    if (player.isLive == 0)
        return 1;
    else
        return 0;
}