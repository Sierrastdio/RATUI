#include "UI_PRINT.h"
#include <ncurses.h>

// 전역 변수 메모리 실체 할당
int UI_Center_x = 0;
int UI_Center_y = 0;

int UI_Win_Width = 0;
int UI_Win_Height = 0;
int UI_Start_Y = 0;
int UI_Left_X = 0;
int UI_Right_X = 0;

// 기존 전체 화면(COLS) 기준 텍스트 중앙 X좌표 계산 함수 (유지)
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

// [추가] 왼쪽 윈도우 내부 출력용 중앙 X좌표 계산 함수
int UI_GET_LEFT_WIN_CENTER_X(int textlen)
{
    if (textlen <= 0 || textlen >= UI_Win_Width) {
        UI_Center_x = 1;
        return 1;
    }

    UI_Center_x = (UI_Win_Width - textlen) / 2;
    return UI_Center_x;
}

// [추가] 오른쪽 윈도우 내부 출력용 중앙 X좌표 계산 함수 (좌우 폭이 동일하므로 UI_Win_Width 재활용)
int UI_GET_RIGHT_WIN_CENTER_X(int textlen)
{
    if (textlen <= 0 || textlen >= UI_Win_Width) {
        UI_Center_x = 1;
        return 1;
    }

    UI_Center_x = (UI_Win_Width - textlen) / 2;
    return UI_Center_x;
}

// 현재 시스템 터미널(LINES, COLS) 기준 전체 분할 UI 레이아웃 크기를 일괄 계산 및 갱신
void UI_INIT_LAYOUT(void)
{
    int total_usable_w = COLS - 4;
    if (total_usable_w < 2) total_usable_w = 2; // 최소 안전폭 확보

    UI_Win_Width = total_usable_w / 2;
    UI_Win_Height = LINES - 6;
    if (UI_Win_Height < 3) UI_Win_Height = 3; // 타이틀 겹침 방지 최소 높이 확보

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
