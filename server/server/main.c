#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define BUF_SIZE 1024
#define WIDTH 83   
#define HEIGHT 50

#define BULLET_MAX 20

#define ENEMY1_MAX 3
#define ENEMY1_BULLET_MAX 3

unsigned int realscore = 0;
int realtime = 0;
int difficulty = 1;

int sendIntArray[20];
int sendPlayerBulletInfo[3];

int recvIntArray[20];
int recvPlayerInfo;
int playerBulletWork[2]; // 13

int timeScoreInfo[2]; // 0 -> time 1 -> score

struct PlayerState {
	int x, y;
	int isLive;
};

struct Enemy1State {
	int x, y;
	int isLive;
	int hp;
};

struct BulletState {
	int x, y;
	int isLive;
};

struct BulletStateE {
	int x, y;
	int isLive;
};

struct PlayerState player;
struct Enemy1State enemy1[ENEMY1_MAX];
struct BulletState bullet[BULLET_MAX];
struct BulletStateE bulletE[ENEMY1_BULLET_MAX];

void ErrorHandling(char* message);

void playerInit();
void enemyInit(fd_set);

void createBullet(int x, int y);
void createBulletE();
void bulletAction();
void bulletHitCheck();
void playerHitCheck();

void sendPlayerCoor(fd_set);
void sendBulletCoor(fd_set);
void sendEnemyCoor(fd_set);
void sendScoreTimeInfo(fd_set);

void setPlayerCoor();
void setEnemyCoor(int);

int endCheck();

int main(int argc, char* argv[])
{
	srand(time(NULL));

	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAdr, clntAdr, clientaddr;
	TIMEVAL timeout;
	fd_set reads, cpyReads;

	int adrSz;
	int strLen, fdNum, i;
	char buf[BUF_SIZE];
	int addrlen;
	int readysignal = 1;
	int errorcode;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAdr.sin_port = htons(9000);

	if (bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");
	if (listen(hServSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	FD_ZERO(&reads);
	FD_SET(hServSock, &reads);

	int position;

	while (1)
	{
		cpyReads = reads;

		timeout.tv_sec = 5;
		fdNum = select(0, &cpyReads, 0, 0, &timeout);

		if (fdNum == SOCKET_ERROR) {
			break;
		}
		else if (fdNum == 0) {
			continue;
		}
		int recvcount;

		for (i = 0; i < reads.fd_count; i++)
		{
			// read set 만 처리...
			if (FD_ISSET(reads.fd_array[i], &cpyReads))
			{
				if (reads.fd_array[i] == hServSock)     // connection request!
				{
					adrSz = sizeof(clntAdr);
					hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &adrSz);
					FD_SET(hClntSock, &reads);

					if (reads.fd_count == 2) {

						recvcount = recv(reads.fd_array[reads.fd_count - 1], &position, sizeof(position), 0);

						if (position != 2) {
							printf("방어자 먼저 접속해야합니다. \n");
							errorcode = 0;
							send(reads.fd_array[i + 1], &errorcode, sizeof(errorcode), 0);

							FD_CLR(reads.fd_array[i + 1], &reads);
							closesocket(reads.fd_array[i + 1]);

							printf("server : client close\n");
						}
						else {
							readysignal += 1;
							printf("방어자 connected...\n");
							printf("client accepted / port : %d, ip: %s\n", ntohs(clntAdr.sin_port), inet_ntoa(clntAdr.sin_addr));
							printf("현재 연결된 클라이언트 수 : %d\n", reads.fd_count - 1);
						}
					}
					else {
						recvcount = recv(reads.fd_array[reads.fd_count - 1], &position, sizeof(position), 0);
						readysignal += 1;
						printf("공격자 %d connected...\n", reads.fd_count - 2);
						printf("client accepted / port : %d, ip: %s\n", ntohs(clntAdr.sin_port), inet_ntoa(clntAdr.sin_addr));
						printf("현재 연결된 클라이언트 수 : %d\n", reads.fd_count - 1);
					}

					printf("readysignal: %d\n", readysignal);

					if (reads.fd_count > 1) {												// 전체 클라이언트에게 현재 연결 상황 보내주기
						for (int j = 1; j < reads.fd_count; j++) {
							send(reads.fd_array[j], &readysignal, sizeof(readysignal), 0);
						}
					}

				}
				else {
					strLen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);

					if (strLen <= 0)    // close request!
					{
						// socket close 처리
						FD_CLR(reads.fd_array[i], &reads);
						closesocket(reads.fd_array[i]);
						printf("server : client close\n");
					}
				}
			}
		}

		if (reads.fd_count == 5) {
			playerInit(reads);
			enemyInit(reads);
			printf("잠시후 게임이 시작됩니다\n");
			break;
		}

	}

	while (1) {
		cpyReads = reads;

		timeout.tv_sec = 1;
		fdNum = select(0, &cpyReads, 0, 0, &timeout);

		if (fdNum == SOCKET_ERROR) {
			break;
		}
		else if (fdNum == 0) {
			continue;
		}

		for (i = 0; i < reads.fd_count; i++) {
			if (FD_ISSET(reads.fd_array[i], &cpyReads)) {
				if (reads.fd_array[i] == reads.fd_array[1]) {
					recv(reads.fd_array[i], &recvPlayerInfo, sizeof(recvPlayerInfo), 0);
					if (recvPlayerInfo != 2) {
						setPlayerCoor();
					}
					else {
						createBullet(player.x,player.y);
					}
				}

				else if(i > 1){
					recv(reads.fd_array[i], recvIntArray, sizeof(recvIntArray), 0);
					setEnemyCoor(i);
					if (recvIntArray[3] == 1) createBulletE(recvIntArray[1], recvIntArray[2]);
				}

			}
		}

		bulletAction();
		bulletHitCheck();
		playerHitCheck();
		sendEnemyCoor(reads);
		sendBulletCoor(reads);
		sendPlayerCoor(reads);
		if (endCheck()) break;
		sendScoreTimeInfo(reads);

		Sleep(50);
		realtime += 1;
	}
	printf("플레이어 사망 ! \n");
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void playerInit() {
	player.x = 39;
	player.y = 46;
	player.isLive = 1;
}

void enemyInit(fd_set reads) {

	for (int i = 0; i < ENEMY1_MAX; i++) { 

		recv(reads.fd_array[i + 2], &recvIntArray, sizeof(recvIntArray), 0);
		enemy1[i].isLive = recvIntArray[0];
		enemy1[i].x = recvIntArray[1];
		enemy1[i].y = recvIntArray[2];

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

	for (int i = 0; i < ENEMY1_BULLET_MAX; i++) {
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

	for (int i = 0; i < ENEMY1_BULLET_MAX; i++) {
		if (bulletE[i].isLive == 1) {
			bulletE[i].y += 2;
			if (bulletE[i].y >= 50)
				bulletE[i].isLive = 0;
		}
	}


}

void bulletHitCheck() {
	for (int i = 0; i < BULLET_MAX; i++) {
		for (int j = 0; j < ENEMY1_MAX; j++) {
			if (bullet[i].isLive) {
				if (enemy1[j].isLive) {
					if (((enemy1[j].x - 4) <= bullet[i].x) && ((enemy1[j].x + 4) >= bullet[i].x) && (enemy1[j].y == bullet[i].y)) {
						realscore += 100;
						enemy1[j].isLive = 0;
						bullet[i].isLive = 0;
						break;
					}
				}
			}
		}
	}

	for (int i = 0; i < ENEMY1_BULLET_MAX; i++) {
		if (bulletE[i].isLive) {
			if (player.isLive) {
				if (((player.x - 4) <= bulletE[i].x) && ((player.x + 4) >= bulletE[i].x) && (player.y == bulletE[i].y)) {
					player.isLive = 0;
					bulletE[i].isLive = 0;
					break;
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
}


void sendPlayerCoor(fd_set reads) {
	memset(sendIntArray, -1, 20 * sizeof(int));

	sendIntArray[0] = player.isLive;
	sendIntArray[1] = player.x;
	sendIntArray[2] = player.y;

	for (int i = 1; i < reads.fd_count; i++) {
		send(reads.fd_array[i], sendIntArray, sizeof(sendIntArray), 0);
	}

}

void sendBulletCoor(fd_set reads) {
	for (int i = 0; i < BULLET_MAX; i++) {
		sendPlayerBulletInfo[0] = bullet[i].isLive;
		sendPlayerBulletInfo[1] = bullet[i].x;
		sendPlayerBulletInfo[2] = bullet[i].y;

		for (int i = 1; i < reads.fd_count; i++) {
			send(reads.fd_array[i], sendPlayerBulletInfo, sizeof(sendPlayerBulletInfo), 0);
		}
	}

	for (int i = 0; i < ENEMY1_BULLET_MAX; i++) {
		sendPlayerBulletInfo[0] = bulletE[i].isLive;
		sendPlayerBulletInfo[1] = bulletE[i].x;
		sendPlayerBulletInfo[2] = bulletE[i].y;

		for (int i = 1; i < reads.fd_count; i++) {
			send(reads.fd_array[i], sendPlayerBulletInfo, sizeof(sendPlayerBulletInfo), 0);
		}
	}
}


void sendEnemyCoor(fd_set reads) {
	memset(sendIntArray, -1, 20 * sizeof(int));

	sendIntArray[0] = enemy1[0].isLive;
	sendIntArray[1] = enemy1[0].x;
	sendIntArray[2] = enemy1[0].y;
	sendIntArray[3] = enemy1[1].isLive;
	sendIntArray[4] = enemy1[1].x;
	sendIntArray[5] = enemy1[1].y;
	sendIntArray[6] = enemy1[2].isLive;
	sendIntArray[7] = enemy1[2].x;
	sendIntArray[8] = enemy1[2].y;

	for (int i = 1; i < reads.fd_count; i++) {
		send(reads.fd_array[i], sendIntArray, sizeof(sendIntArray), 0);
	}
}

void sendScoreTimeInfo(fd_set reads) {
	timeScoreInfo[0] = realtime;											
	timeScoreInfo[1] = realscore;

	for (int i = 1; i < reads.fd_count; i++) {
		send(reads.fd_array[i], timeScoreInfo, sizeof(timeScoreInfo), 0);
	}
}

void setPlayerCoor() {

	if (recvPlayerInfo == 0) {				// 좌
		if (player.x > WIDTH - 78) {
			player.x -= 4;
		}
	}
	else {
		if (player.x < WIDTH - 4) {			// 우
			player.x += 4;
		}
	}
	
}

void setEnemyCoor(int i) {
	enemy1[i - 2].isLive = recvIntArray[0];
	enemy1[i - 2].x = recvIntArray[1];
	enemy1[i - 2].y = recvIntArray[2];

}

int endCheck() {
	if (player.isLive == 0)
		return 1;
	else
		return 0;
}