#include <ncurses.h>
#include <stdio.h>
#include <string.h>
#include "SECTOR_MENU.h"
#include "help_UI.h"
#include "UI_PRINT.h"

#define MAX_VISIBLE 15  /* 한번에 표시할 항목 개수 최대 15개 */

/*===========================================================================
 * SECTOR_MENU — 전체 화면(stdscr) 기반 중앙 정렬 메인 메뉴
 *==========================================================================*/
int SECTOR_MENU(const char *title, const char *options[], int count, int *current_cursor, int sector_id) {
    int start_index = 0;
    int key;

    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    if (*current_cursor >= MAX_VISIBLE) {
        start_index = *current_cursor - (MAX_VISIBLE - 1);
    }

    while (1) {
        clear();

        // snprintf를 사용해 불필요한 do-while 나눗셈 루프 없이 타이틀 정렬 길이 계산
        int title_len = snprintf(NULL, 0, " === %s (Total: %d) === ", title, count);

        UI_GET_CENTER_X(title_len);

        attron(A_REVERSE);
        mvprintw(1, UI_Center_x, " === %s (Total: %d) === ", title, count);
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

            case KEY_PPAGE:
                *current_cursor -= MAX_VISIBLE;
                if (*current_cursor < 0) *current_cursor = 0;
                break;

            case KEY_NPAGE:
                *current_cursor += MAX_VISIBLE;
                if (*current_cursor >= count) *current_cursor = count - 1;
                break;

            case 10:
                return *current_cursor;

            case 'd':
            case 'D':
                return SIGN_DELETE;

            case 'q':
            case 'Q':
            case 27:
                return SIGN_CANCEL;

            case 'r':
            case 'R':
                return SIGN_REFRESH;
        }
    }
}

/*===========================================================================
 * SECTOR_MENU_WIN — 윈도우(WINDOW*) 전용 서브 메뉴
 *==========================================================================*/
int SECTOR_MENU_WIN(WINDOW *win, const char *title, const char *items[], int count, int *cursor, int max_len) {
    int win_width = getmaxx(win);
    int win_height = getmaxy(win);

    int menu_start_x;
    int start_y;
    int max_draw_count = win_height - 5; // 박스 경계선 및 타이틀/안내선 출력 공간 제외

    if (max_draw_count < 1) max_draw_count = 1;

    // 스크롤 시작 위치 인덱스 계산
    int start_index = 0;
    if (*cursor >= max_draw_count) {
        start_index = *cursor - (max_draw_count - 1);
    }

    if (max_len == SIGN_LEFT_ALIGN) {
        menu_start_x = 4;
        start_y = 3;
    } else {
        menu_start_x = (win_width - max_len) / 2;
        if (menu_start_x < 1) menu_start_x = 1;

        start_y = (win_height / 2) - (count / 2);
        if (start_y < 3) start_y = 3;
    }

    werase(win);
    box(win, 0, 0);

    int title_x = (win_width - (int)strlen(title)) / 2;
    if (title_x < 1) title_x = 1;

    wattron(win, A_REVERSE);
    mvwprintw(win, 1, title_x, "%s", title);
    wattroff(win, A_REVERSE);

    // 안전 스크롤 루프 출력
    for (int i = 0; i < max_draw_count && (start_index + i) < count; i++) {
        int idx = start_index + i;
        int print_y = start_y + i;

        if (print_y >= win_height - 1) break;

        if (idx == *cursor) {
            wattron(win, A_REVERSE);
            mvwprintw(win, print_y, menu_start_x, "%s", items[idx]);
            wattroff(win, A_REVERSE);
        } else {
            mvwprintw(win, print_y, menu_start_x, "%s", items[idx]);
        }
    }

    wrefresh(win);

    keypad(win, TRUE);
    int ch = wgetch(win);

    switch (ch) {
        case 27:
        case 'q':
        case 'Q':
            return SIGN_CANCEL;

        case KEY_UP:
        case 'w':
        case 'W':
            if (*cursor > 0) (*cursor)--;
            else *cursor = count - 1;
            return SIGN_KEY_CHANGED;

        case KEY_DOWN:
        case 's':
        case 'S':
            if (*cursor < count - 1) (*cursor)++;
            else *cursor = 0;
            return SIGN_KEY_CHANGED;

        case KEY_PPAGE:
            *cursor -= max_draw_count;
            if (*cursor < 0) *cursor = 0;
            return SIGN_KEY_CHANGED;

        case KEY_NPAGE:
            *cursor += max_draw_count;
            if (*cursor >= count) *cursor = count - 1;
            return SIGN_KEY_CHANGED;

        case '\n':
        case ' ':
            return *cursor;

        case 'd':
        case 'D':
            return SIGN_DELETE;

        case 'r':
        case 'R':
            return SIGN_REFRESH;

        default:
            return SIGN_KEY_CHANGED;
    }
}
