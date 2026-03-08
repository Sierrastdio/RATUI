#ifndef SECTOR_MENU_H
#define SECTOR_MENU_H

#include <ncurses.h>

// 섹션 정의
/**/
#define HOME 0
#define ROS  1
#define INS  2
#define EDS  3
#define BKS  4
#define TRS  5
#define EXIT 6
#define CANCEL 7

// 메뉴 동작을 담당하는 핵심 함수
// 리턴값: 사용자가 선택한 섹션 번호
int SECTOR_MENU(const char *title, const char *options[], int count, int *current_cursor, int sector_id);

// 이제 섹터별로 메인창 만들때마다 이거 추가해야됨.
void ROS_MAIN_LOOP();
void INS_MAIN_LOOP();
void EDS_MAIN_LOOP();
void BKS_MAIN_LOOP();
void TRS_MAIN_LOOP();

#endif