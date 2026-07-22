#ifndef SECTOR_MENU_H
#define SECTOR_MENU_H

#include <ncurses.h>

/* 섹션 정의 (메뉴 ID) */
#define HOME   0
#define ROS    1
#define INS    2
#define EDS    3
#define BKS    4
#define TRS    5
#define EXIT   6
#define CANCEL 7

/* 제어 시그널 반환값 정의 */
#define SIGN_CANCEL      -1
#define SIGN_DELETE      -2
#define SIGN_REFRESH     -3
#define SIGN_KEY_CHANGED -4 // 윈도우 내 실시간 커서 변경 제어용 신호
#define SIGN_LEFT_ALIGN  -5 // 데이터 윈도우 파일 리스트 좌측 정렬 지표

// 메인 화면 전체 전용 메뉴 렌더러
int SECTOR_MENU(const char *title, const char *options[], int count, int *current_cursor, int sector_id);

// 분할 윈도우(WINDOW*) 전용 서브 메뉴 렌더러
int SECTOR_MENU_WIN(WINDOW *win, const char *title, const char *items[], int count, int *cursor, int max_len);

// 섹터별 메인 루프 프로토타입
void ROS_MAIN_LOOP(void);
void INS_MAIN_LOOP(void);
void EDS_MAIN_LOOP(void);
void BKS_MAIN_LOOP(void);
void TRS_MAIN_LOOP(void);

#endif
