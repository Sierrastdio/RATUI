#ifndef UI_PRINT_H
#define UI_PRINT_H

#include <ncurses.h>

// 주어진 텍스트 길이를 기준으로 가로 중앙 정렬 시작 X 좌표를 반환하는 함수 원형
int UI_GET_CENTER_X(int x, int textlen);

#endif
