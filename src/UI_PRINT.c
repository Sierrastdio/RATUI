#include "UI_PRINT.h"

#include <string.h>

// 문자열 가운데 X좌표 계산
int UI_CENTER_X(const char *text)
{
    if (text == NULL)
        return 0;

    int len = strlen(text);

    if (len >= COLS)
        return 0;

    return (COLS - len) / 2;
}

// 화면 중앙 Y좌표
int UI_CENTER_Y(void)
{
    return LINES / 2;
}

// 지정한 줄에서 가운데 출력
void UI_PRINT_CENTER(int y, const char *text)
{
    mvprintw(y, UI_CENTER_X(text), "%s", text);
}

// 지정한 줄에서 가운데 + Reverse
void UI_PRINT_CENTER_HIGHLIGHT(int y, const char *text)
{
    attron(A_REVERSE);
    UI_PRINT_CENTER(y, text);
    attroff(A_REVERSE);
}

// 화면 중앙 기준 출력
void UI_PRINT_MIDDLE(int y_offset, const char *text)
{
    UI_PRINT_CENTER(UI_CENTER_Y() + y_offset, text);
}
