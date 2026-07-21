#ifndef ROSFUNC_H
#define ROSFUNC_H

#include <ncurses.h>

/* ROS 메인 대화형 루프 */
void ROS_MAIN_LOOP(void);

/* 우측 데이터 윈도우 기반 기능 함수 */
void ROSfunc_show_info(WINDOW *data_win);
void ROSfunc_manage_storage(WINDOW *data_win);

#endif
