#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <winsock2.h>
#include <windows.h>
#include "console.h"

#define WIDTH 83   
#define HEIGHT 50

#define BULLET_MAX 20
#define ENEMY1_MAX 3
#define ENEMY1_BULLET_MAX 3

char screen[HEIGHT][WIDTH];
char title[HEIGHT][WIDTH] = {
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n", // 14,33 
     "                                                                                 \n",
     "                                                                                 \n", // 16,33
     "                                                                                 \n", // 17,34 
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n",
     "                                                                                 \n"
};

char playerCursor[3] = "->";
char playerUnit[6] = "=|^|=";
char playerBullet = '|';

char enemyUnit[8] = "-< o >-";
char enemyBullet = '@';

char playerScore[9] = "Score : ";
char playerTime[8] = "Time : ";

unsigned int realscore = 0;
int realtime;

int sendIntArray[20];
int sendPlayerInfo;
int recvIntArray[20];
int recvPlayerBulletInfo[3];

int timeScoreInfo[2]; // 0 -> time 1 -> score

int level = -1;             // -1 : 메인화면 0 : 로딩화면 1~ : 게임시작
int position = 2;                // 1 : 공격자 2 : 방어자
int readysignal = 0; // 1 : 서버 연결 2 : 방어자 연결 3 : 공격자 1 연결 4 : 공격자 2 연결 5 : 공격자 3 연결

struct cursorState {
    int x, y;
};

struct PlayerState {
    int x, y;
    int isLive;
};

struct Enemy1State {
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
struct BulletState bullet[BULLET_MAX];
struct BulletStateE bulletE[ENEMY1_BULLET_MAX];

void waitSec(int time);
void errorDisplay(char* str);

void cursorInit();
void cursorAction();
int cursorSelect();

void playerAction(SOCKET);

void getPlayerCoor(SOCKET);
void getBulletCoor(SOCKET);
void getEnemyCoor(SOCKET);
void getScoreTimeInfo(SOCKET);

unsigned WINAPI draw();
void drawMain();
void drawLoading();
void drawCursor();

void drawPlayer();
void drawEnemy();
void drawBullet();
void drawScore();
void drawTime();

int startCheck();
int endCheck();

int main() {

    srand(time(NULL));

    system("mode con cols=83 lines=50");
    Initial();
    cursorInit();

    int	retval;

    WSADATA	wsa;
    retval = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (retval != 0)	return -1;

    SOCKET	ClientSocket;
    ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ClientSocket == INVALID_SOCKET) {
        errorDisplay("socket() error(INVALID_SOCKET)");
    }

    SOCKADDR_IN	ServerAddr;
    ZeroMemory(&ServerAddr, sizeof(ServerAddr));
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(9000);
    ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");


    while (level == -1) {                       // 메인화면
        cursorAction();
        drawMain();
        if (cursorSelect() == 1) level = 0;
        else if (cursorSelect() == 2) return 0;
        Sleep(100);
    }

    retval = connect(ClientSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
    retval = send(ClientSocket, &position, sizeof(position), 0);
    if (retval == SOCKET_ERROR) {
        errorDisplay("connect() error(SOCKET_ERROR)");
    }

    int recvcount;

    while (level == 0) {                      // 로딩화면
        recvcount = recv(ClientSocket, &readysignal, sizeof(readysignal), 0);
        drawLoading();
        if (startCheck() == 1) {
            level = 1;
            break;
        }
        waitSec(1);
    }

    player.isLive = 1;

    unsigned threadID1;
    HANDLE hHandle1;

    while (1) {
        playerAction(ClientSocket);
        getEnemyCoor(ClientSocket);
        getBulletCoor(ClientSocket);
        getPlayerCoor(ClientSocket);
        if (endCheck()) break;
        getScoreTimeInfo(ClientSocket);
        hHandle1 = _beginthreadex(NULL, 0, draw, NULL, 0, &threadID1);
        Sleep(50);
    }

    printf("\n플레이어 사망 ! 버틴 시간 : %d 최종 점수 : %d\n", realtime, realscore);
    return 0;
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

    if ((GetAsyncKeyState(VK_UP) & 0x8000)) {
        if (cursor.y > 16) {
            cursor.y--;
        }
    }
    if ((GetAsyncKeyState(VK_DOWN) & 0x8000)) {
        if (cursor.y <= 16) {
            cursor.y++;
        }
    }
}

int cursorSelect() {
    if ((GetAsyncKeyState(VK_RETURN) & 0x8000)) {
        if (cursor.y == 16) {
            return 1;
        }
        else if (cursor.y == 17) {
            return 2;
        }
    }   
}

void playerAction(SOCKET ClientSocket) {

    int x, y;

    x = player.x;
    y = player.y;

    if (player.isLive) {
        if ((GetAsyncKeyState(VK_LEFT) & 0x8000)) {         // 좌 :0 우 : 1 총알 : 2
            sendPlayerInfo = 0;
            send(ClientSocket, &sendPlayerInfo, sizeof(sendPlayerInfo), 0);
        }

        if ((GetAsyncKeyState(VK_RIGHT) & 0x8000)) {
            sendPlayerInfo = 1;
            send(ClientSocket, &sendPlayerInfo, sizeof(sendPlayerInfo), 0);
        }

        if ((GetAsyncKeyState(VK_SPACE) & 0x8000)) {
            sendPlayerInfo = 2;
            send(ClientSocket, &sendPlayerInfo, sizeof(sendPlayerInfo), 0);
        }
    }

}

void getPlayerCoor(SOCKET ClientSocket) {

    recv(ClientSocket, &recvIntArray, sizeof(recvIntArray), 0);

    player.isLive = recvIntArray[0];
    player.x = recvIntArray[1];
    player.y = recvIntArray[2];

}

void getBulletCoor(SOCKET ClientSocket) {
    for (int i = 0; i < BULLET_MAX; i++) {
        recv(ClientSocket, recvPlayerBulletInfo, sizeof(recvPlayerBulletInfo), 0);
        bullet[i].isLive = recvPlayerBulletInfo[0];
        bullet[i].x = recvPlayerBulletInfo[1];
        bullet[i].y = recvPlayerBulletInfo[2];
    }

    for (int i = 0; i < ENEMY1_BULLET_MAX; i++) {
        recv(ClientSocket, recvPlayerBulletInfo, sizeof(recvPlayerBulletInfo), 0);
        bulletE[i].isLive = recvPlayerBulletInfo[0];
        bulletE[i].x = recvPlayerBulletInfo[1];
        bulletE[i].y = recvPlayerBulletInfo[2];
    }
}


void getEnemyCoor(SOCKET ClientSocket) {

    recv(ClientSocket, &recvIntArray, sizeof(recvIntArray), 0);

    enemy1[0].isLive = recvIntArray[0];
    enemy1[0].x = recvIntArray[1];
    enemy1[0].y = recvIntArray[2];
    enemy1[1].isLive = recvIntArray[3];
    enemy1[1].x = recvIntArray[4];
    enemy1[1].y = recvIntArray[5];
    enemy1[2].isLive = recvIntArray[6];
    enemy1[2].x = recvIntArray[7];
    enemy1[2].y = recvIntArray[8];

}

void getScoreTimeInfo(SOCKET ClientSocket) {
    recv(ClientSocket, &timeScoreInfo, sizeof(timeScoreInfo), 0);

    realtime = timeScoreInfo[0];
    realscore = timeScoreInfo[1];
}

unsigned WINAPI draw() {
    int i;
    for (i = 0; i < HEIGHT; i++)
    {
        memset(screen[i], ' ', WIDTH);
        screen[i][WIDTH - 1] = NULL;
    }

    drawPlayer();
    drawBullet();
    drawEnemy();
    drawScore();
    drawTime();

    for (i = 0; i < HEIGHT; i++)
    {
        MoveCursor(0, i);        //  커서의 시작 지점 즉 y축을 바꾸어주며 한줄단위로 출력
        printf(screen[i]);

        MoveCursor(8, 48);
        printf("%d", realscore); // 실시간 점수 출력

        MoveCursor(8, 49);
        printf("%d", realtime); // 실시간 시간 출력
    }

    return 0;
}

void drawMain() {

    for (int i = 0; i < HEIGHT; i++)
    {
        strcpy(title[14], "                             비행기 게임(방어자)                                 \n");
        strcpy(title[16], "                                 Game start                                      \n");
        strcpy(title[17], "                                  End game                                       \n");
        title[i][WIDTH - 1] = NULL;
    }

    drawCursor();

    for (int i = 0; i < HEIGHT; i++)
    {
        MoveCursor(0, i);
        printf(title[i]);
    }
}

void drawLoading() {
    for (int i = 0; i < HEIGHT; i++)
    {
        strcpy(title[14], "                                서버 접속완료                                    \n");

        if (readysignal > 1) {
            strcpy(title[15], "                                  방어자....O                                    \n");
        }
        else {
            strcpy(title[15], "                                  방어자....X                                    \n");
        }


        if (readysignal > 2) {
            strcpy(title[16], "                                  공격자1...O                                    \n");
        }
        else {
            strcpy(title[16], "                                  공격자1...X                                    \n");
        }


        if (readysignal > 3) {
            strcpy(title[17], "                                  공격자2...O                                    \n");
        }
        else {
            strcpy(title[17], "                                  공격자2...X                                    \n");
        }


        if (readysignal > 4) {
            strcpy(title[18], "                                  공격자3...O                                    \n");
        }
        else {
            strcpy(title[18], "                                  공격자3...X                                    \n");
        }

    }


    for (int i = 0; i < HEIGHT; i++)
    {
        MoveCursor(0, i);
        printf(title[i]);
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

    int x, y;

    for (int i = 0; i < ENEMY1_MAX; i++) {
        x = enemy1[i].x;
        y = enemy1[i].y;

        for (int j = 0; j < 7; j++) {
            screen[y][x++] = enemyUnit[j];
        }
    }

}

void drawBullet() {
    for (int i = 0; i < BULLET_MAX; i++) {
        if (bullet[i].isLive == 1) {
            screen[bullet[i].y][bullet[i].x] = playerBullet;
        }
    }

    for (int i = 0; i < ENEMY1_BULLET_MAX; i++) {
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

int startCheck() {
    if (readysignal == 5)
        return 1;
    else return 0;
}

int endCheck() {
    if (player.isLive == 0)
        return 1;
    else
        return 0;
}