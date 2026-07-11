#include <ncurses.h>
#include <string.h>
#include "SECTOR_MENU.h"
#include "help_UI.h"

#define MAX_VISIBLE 15  /*  한번에 표시할 항목 개수 최대 15개   */

int SECTOR_MENU(const char *title, const char *options[], int count, int *current_cursor, int sector_id) {
    int start_index = 0;
    int key;

    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    /*  MAX_VISIBLE 보다 커서위치 값이 더 크면 처음 커서위치 = 0 으로 돌아가게 함.  */
    if (*current_cursor >= MAX_VISIBLE) {
        start_index = *current_cursor - (MAX_VISIBLE - 1);
    }

    while (1) {
        clear();
        attron(A_REVERSE);
        mvprintw(1, 2, " === %s (Total: %d) === ", title, count);
        attroff(A_REVERSE);

        if (*current_cursor < start_index) {
            start_index = *current_cursor;
        } else if (*current_cursor >= start_index + MAX_VISIBLE) {
            start_index = *current_cursor - MAX_VISIBLE + 1;
        }

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

        int progress = (count > 0) ? ((*current_cursor + 1) * 100 / count) : 0;
        mvprintw(LINES - 2, 2, " [UP/DOWN]: Move | [PgUp/PgDn]: Jump | [?]: Help | [ENTER]: Select | Progress: %d%% ", progress);
        refresh();

        key = getch();

        if (key == '?') {
            SHOW_HELP(sector_id);
            clear();
            refresh();
            continue;
        }

        switch (key) {
            case KEY_UP:
                if (*current_cursor > 0) (*current_cursor)--;
                else *current_cursor = count - 1;
                break;

            case KEY_DOWN:
                if (*current_cursor < count - 1) (*current_cursor)++;
                else *current_cursor = 0;
                break;

            case KEY_PPAGE: // Page Up
                *current_cursor -= MAX_VISIBLE;
                if (*current_cursor < 0) *current_cursor = 0;
                break;

            case KEY_NPAGE: // Page Down
                *current_cursor += MAX_VISIBLE;
                if (*current_cursor >= count) *current_cursor = count - 1;
                break;

            case 10: // ENTER (선택/진입)
                return *current_cursor;

            case 'd': // [추가] d키를 누르면 삭제 신호 -2 반환
            case 'D':
                return -2;

            case 'q':
            case 'Q':
            case 27:
                return -1; // 취소 신호

            case 'r':
            case 'R':
                return -3;
        }
    }
}
