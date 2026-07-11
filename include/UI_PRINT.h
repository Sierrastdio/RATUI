#ifndef UI_PRINT_H
#define UI_PRINT_H

#include <ncurses.h>

// 문자열을 가운데 출력하기 위한 X 좌표 계산
int UI_CENTER_X(const char *text);

// 화면 세로 중앙 Y 좌표 반환
int UI_CENTER_Y(void);

// 지정한 Y에서 문자열 가운데 출력
void UI_PRINT_CENTER(int y, const char *text);

// 지정한 Y에서 문자열 가운데 + Reverse 출력
void UI_PRINT_CENTER_HIGHLIGHT(int y, const char *text);

// 화면 중앙(Y 기준)에 출력
void UI_PRINT_MIDDLE(int y_offset, const char *text);

#endif
