/*  업그레이드 계획.  
 *
 *  [V] Scrolling (화면 단위 전환): 전체 리스트에서 현재 화면에 출력할 start_index를 관리.
 *
 *  [V] Page Up/Down: 한 번에 화면 크기(예: 10줄)만큼 커서를 점프시키고, 화면 범위를 갱신.
 *
 *  [ ] Search (검색): 특정 키(예: / 또는 f)를 누르면 검색어를 입력받고, 
 *  해당 글자가 포함된 항목만 필터링해서 보여주기
 *
 */

#include <ncurses.h>
#include <string.h>
#include "SECTOR_MENU.h"

// 한 화면에 표시할 최대 메뉴 항목 수 (터미널 크기에 맞춰 조절 가능)
#define MAX_VISIBLE 15 

int SECTOR_MENU(const char *title, const char *options[], int count, int *current_cursor) {
    int start_index = 0; // 화면에 출력될 리스트의 시작 지점
    int key;

    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    // 초기 진입 시 커서 위치에 따른 start_index 보정
    if (*current_cursor >= MAX_VISIBLE) {
        start_index = *current_cursor - (MAX_VISIBLE - 1);
    }

    while (1) {
        clear();
        
        // 1. 상단 타이틀 및 상태 바 출력
        attron(A_REVERSE);
        mvprintw(1, 2, " === %s (Total: %d) === ", title, count);
        attroff(A_REVERSE);

        // 2. 스크롤 위치 계산 (커서가 화면 밖으로 나갈 때)
        if (*current_cursor < start_index) {
            start_index = *current_cursor;
        } else if (*current_cursor >= start_index + MAX_VISIBLE) {
            start_index = *current_cursor - MAX_VISIBLE + 1;
        }

        // 3. 현재 화면 범위 내의 메뉴 출력
        for (int i = 0; i < MAX_VISIBLE && (start_index + i) < count; i++) {
            int idx = start_index + i;
            if (idx == *current_cursor) {
                attron(A_REVERSE);
                mvprintw(3 + i, 4, " > %s ", options[idx]);
                attroff(A_REVERSE);
            } else {
                mvprintw(3 + i, 4, "   %s ", options[idx]);
            }
        }

        // 4. 하단 가이드라인 (스크롤 바 위치 표시 기능 포함)
        int progress = (count > 0) ? ((*current_cursor + 1) * 100 / count) : 0;
        mvprintw(LINES - 2, 2, " [UP/DOWN]: Move | [PgUp/PgDn]: Jump | [ENTER]: Select | Progress: %d%% ", progress);
        refresh();

        key = getch();

        switch (key) {
            case KEY_UP:
                if (*current_cursor > 0) (*current_cursor)--;
                else *current_cursor = count - 1; // 순환
                break;

            case KEY_DOWN:
                if (*current_cursor < count - 1) (*current_cursor)++;
                else *current_cursor = 0; // 순환
                break;

            case KEY_PPAGE: // Page Up
                *current_cursor -= MAX_VISIBLE;
                if (*current_cursor < 0) *current_cursor = 0;
                break;

            case KEY_NPAGE: // Page Down
                *current_cursor += MAX_VISIBLE;
                if (*current_cursor >= count) *current_cursor = count - 1;
                break;

            case 10: // ENTER 키
                return *current_cursor;

            default:
                break;
        }
    }
}