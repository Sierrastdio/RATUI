#ifndef SECTOR_MENU_H
#define SECTOR_MENU_H

#include <ncurses.h>

// 섹션 정의
#define HOME 0
#define ROS  1
#define INS  2
#define EDS  3
#define BKS  4
#define EXIT 5
#define CANCEL 6

// 메뉴 동작을 담당하는 핵심 함수
// 리턴값: 사용자가 선택한 섹션 번호
int SECTOR_MENU(char *title, char *items[], int count, int *cursor_pos);

// 이제 섹터별로 메인창 만들때마다 이거 추가해야됨.
void ROS_MAIN_LOOP();

#endif