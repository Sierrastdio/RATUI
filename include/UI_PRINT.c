#include "UI_PRINT.h"
#include <ncurses.h>
#include <string.h>

// 전역 변수 메모리 실체 할당
int UI_Center_x = 0;
int UI_Center_y = 0;

int UI_Win_Width = 0;
int UI_Win_Height = 0;
int UI_Start_Y = 0;
int UI_Left_X = 0;
int UI_Right_X = 0;

int UI_GET_CENTER_X(int textlen)
{
    if (textlen <= 0 || textlen >= COLS) {
        UI_Center_x = 0;
        return 0;
    }

    UI_Center_x = (COLS - textlen) / 2;
    return UI_Center_x;
}

int UI_GET_LEFT_WIN_CENTER_X(int textlen)
{
    if (textlen <= 0 || textlen >= UI_Win_Width) {
        UI_Center_x = 1;
        return 1;
    }

    UI_Center_x = (UI_Win_Width - textlen) / 2;
    return UI_Center_x;
}

int UI_GET_RIGHT_WIN_CENTER_X(int textlen)
{
    return UI_GET_LEFT_WIN_CENTER_X(textlen);
}

int UI_GET_WIN_CENTER_Y(int offset)
{
    UI_Center_y = (UI_Win_Height / 2) + offset;

    if (UI_Center_y < 1) {
        UI_Center_y = 1;
    } else if (UI_Center_y >= UI_Win_Height - 1) {
        UI_Center_y = UI_Win_Height - 2;
        if (UI_Center_y < 1) UI_Center_y = 1;
    }

    return UI_Center_y;
}

void UI_INIT_LAYOUT(void)
{
    int total_usable_w = COLS - 4;
    if (total_usable_w < 2) total_usable_w = 2;

    UI_Win_Width = total_usable_w / 2;
    UI_Win_Height = LINES - 6;
    if (UI_Win_Height < 3) UI_Win_Height = 3;

    UI_Start_Y = 2;
    UI_Left_X = 2;
    UI_Right_X = UI_Left_X + UI_Win_Width;
}

// 기존 테두리(box) 및 물리 화면 투영(wrefresh) 스타일을 완벽히 계승한 윈도우 생성 헬퍼 함수
WINDOW *UI_CREATE_WINDOW(int height, int width, int start_y, int start_x)
{
    WINDOW *win = newwin(height, width, start_y, start_x);
    if (win == NULL) return NULL;

    box(win, 0, 0); // 기존 스타일대로 초기 테두리 형성
    wrefresh(win);  // 물리 화면에 즉시 투영

    // 키패드 매핑 활성화 (메뉴 내부 wgetch 동작용)
    keypad(win, TRUE);

    return win;
}

WINDOW *UI_CREATE_FOOTER_WINDOW(void)
{
    WINDOW *win = newwin(3, COLS - 4, LINES - 4, 2);
    if (win == NULL) return NULL;

    box(win, 0, 0);
    mvwprintw(win, 1, 2, "Use UP/DOWN to Navigate. Press [ENTER] to select. (ESC/q: Back)");
    wrefresh(win);

    return win;
}

void UI_CLEAR_WINDOW(WINDOW *win)
{
    if (win == NULL) return;

    werase(win);
    box(win, 0, 0);
}

void UI_PRINT_CENTERED(WINDOW *win, int y, const char *text)
{
    int win_width;
    int text_len;
    int x;

    if (win == NULL || text == NULL) return;

    win_width = getmaxx(win);
    text_len = (int)strlen(text);
    x = (win_width - text_len) / 2;
    if (x < 1) x = 1;

    mvwprintw(win, y, x, "%s", text);
}

void UI_PRINT_IDLE(WINDOW *win)
{
    if (win == NULL) return;

    UI_CLEAR_WINDOW(win);
    UI_PRINT_CENTERED(win, UI_GET_WIN_CENTER_Y(0), "[ No Active Task ]");
    wrefresh(win);
}
