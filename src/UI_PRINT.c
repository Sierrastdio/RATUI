#include "UI_PRINT.h"
#include <ncurses.h>

// 전역 변수 메모리 할당
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

    // 전역 변수에 계산값을 바로 때려 박음
    UI_Center_x = (COLS - textlen) / 2;
    return UI_Center_x;
}
