#ifndef SECTOR_MENU_H
#define SECTOR_MENU_H

#include <ncurses.h>

/* 섹션 정의를 정수형으로 해서 값 비교 용이하도록 함   */
#define HOME 0
#define ROS  1
#define INS  2
#define EDS  3
#define BKS  4
#define TRS  5
#define EXIT 6
#define CANCEL 7

/* switch 문의 반환값 정의. 값의 이유는 */
#define SIGN_CANCEL -1
#define SIGN_DELETE -2
#define SIGN_REFRESH -3
#define SIGN_KEY_CHANGED -4 // [추가] 윈도우 내 실시간 커서 변경 제어용 신호

// 메뉴 동작을 담당하는 핵심 함수
// 리턴값: 사용자가 선택한 섹션 번호
int SECTOR_MENU(const char *title, const char *options[], int count, int *current_cursor, int sector_id);

int SECTOR_MENU_WIN(WINDOW *win, const char *title, const char *items[], int count, int *cursor, int max_len);

// 이제 섹터별로 메인창 만들때마다 이거 추가해야됨.
void ROS_MAIN_LOOP();
void INS_MAIN_LOOP();
void EDS_MAIN_LOOP();
void BKS_MAIN_LOOP();
void TRS_MAIN_LOOP();

#endif
