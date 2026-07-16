#include "UI_PRINT.h"
#include <ncurses.h>

// 전역 변수 메모리 실체 할당
int UI_Center_x = 0;
int UI_Center_y = 0;

int UI_GET_CENTER_X(int textlen)
{
    if (textlen <= 0) {
        UI_Center_x = 0;
        return 0;
    }

    if (textlen >= COLS) {
        UI_Center_x = 0;
        return 0;
    }

    UI_Center_x = (COLS - textlen) / 2;
    return UI_Center_x;
}

// [링킹 에러 해결] 실제 윈도우 생성 및 테두리 헬퍼 함수 구현체
WINDOW *UI_CREATE_WINDOW(int height, int width, int start_y, int start_x)
{
    WINDOW *win = newwin(height, width, start_y, start_x);
    if (win == NULL) return NULL;

    box(win, 0, 0); // 테두리 치기
    wrefresh(win);  // 물리 화면에 즉시 투영
    return win;
}
