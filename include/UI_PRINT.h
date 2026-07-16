#ifndef UI_PRINT_H
#define UI_PRINT_H

#include <ncurses.h>

// 전역 공유 좌표 변수 선언
extern int UI_Center_x;
extern int UI_Center_y;

// 가로 중앙 정렬 시작 X 좌표 계산 및 UI_Center_x 자동 갱신
int UI_GET_CENTER_X(int textlen);

// [핵심] 이 함수 원형이 있어야 컴파일러가 WINDOW* 반환형을 인지함
WINDOW *UI_CREATE_WINDOW(int height, int width, int start_y, int start_x);

#endif
