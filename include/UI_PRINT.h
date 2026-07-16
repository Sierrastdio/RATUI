#ifndef UI_PRINT_H
#define UI_PRINT_H

#include <ncurses.h>

// 전역 공유 좌표 변수 선언
extern int UI_Center_x;
extern int UI_Center_y;

// 이제 인자로 x를 받지 않고, 내부에서 UI_mx를 직접 제어/반환함
int UI_GET_CENTER_X(int textlen);

#endif
