#ifndef ROSFUNC_H
#define ROSFUNC_H

#include <ncurses.h>

// [수정] 윈도우 포인터를 인자로 받도록 원형 변경
void ROS_MAIN_LOOP(void);
void ROSfunc_show_info(WINDOW *data_win);
void ROSfunc_manage_storage(WINDOW *data_win);

#endif
