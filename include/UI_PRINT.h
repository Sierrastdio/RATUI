#ifndef UI_PRINT_H
#define UI_PRINT_H

#include <ncurses.h>

// UI 출력을 전담하는 레이아웃 제어용 전역 변수들
extern int UI_Center_x;     // 표준 화면 타이틀 중앙 정렬용 X좌표
extern int UI_Center_y;     // 표준 화면 타이틀 중앙 정렬용 Y좌표
extern int UI_Win_Width;    // 분할 배치될 개별 윈도우의 가로 폭
extern int UI_Win_Height;   // 분할 배치될 개별 윈도우의 세로 높이
extern int UI_Start_Y;      // 분할 윈도우가 시작되는 세로 Y좌표
extern int UI_Left_X;       // 왼쪽 메뉴 윈도우의 시작 X좌표
extern int UI_Right_X;      // 오른쪽 데이터 윈도우의 시작 X좌표

// [수정] 충돌 방지를 위해 원본 소스 규격에 맞춰 void -> int로 변경 완료
int UI_GET_CENTER_X(int text_len);

// 현재 시스템 터미널(LINES, COLS) 기준 전체 분할 UI 레이아웃 크기를 갱신하는 함수
void UI_INIT_LAYOUT(void);

// 윈도우 생성 헬퍼 함수
WINDOW *UI_CREATE_WINDOW(int height, int width, int start_y, int start_x);

#endif
