#include <Windows.h>
#include <stdio.h>
#include "console.h"

HANDLE hConsole;

void Initial() {            // 커서 안보이게 해주는 함수
    CONSOLE_CURSOR_INFO csCursor;

    csCursor.bVisible = FALSE;
    csCursor.dwSize = 1;

    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorInfo(hConsole, &csCursor);
}

void MoveCursor(int x, int y) {            // 커서 움직여주는 함수
    COORD coord;
    coord.X = x;
    coord.Y = y;

    SetConsoleCursorPosition(hConsole, coord);

}

void ClearScreen() {            // 스크린을 공백으로채워 빈화면으로 만들어주는 함수
    int x, y;

    for (y = 0; y < HEIGHT; y++) {
        MoveCursor(0, y);
        for (x = 0; x < WIDTH; x++) {
            printf("%c", ' ');
        }
    }
}