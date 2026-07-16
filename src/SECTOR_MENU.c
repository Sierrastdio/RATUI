#include <ncurses.h>
#include <string.h>
#include "SECTOR_MENU.h"
#include "help_UI.h"
#include "UI_PRINT.h"

#define MAX_VISIBLE 15  /* 한번에 표시할 항목 개수 최대 15개   */

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

        int title_len = (sizeof(" ===  (Total: ) === ") - 1) + (int)strlen(title);
        int temp_count = count;
        do {
           title_len++;
           temp_count /= 10;
        } while (temp_count > 0);

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

// [수정 완료] switch-case 구조로 완전히 리팩토링된 윈도우 전용 메뉴 기능
int SECTOR_MENU_WIN(WINDOW *win, const char *title, const char *items[], int count, int *cursor, int max_len) {
    int win_width = getmaxx(win);
    int win_height = getmaxy(win);

    int menu_start_x = (win_width - max_len) / 2;
    if (menu_start_x < 1) menu_start_x = 1;

    int start_y = (win_height / 2) - (count / 2);
    if (start_y < 3) start_y = 3;

    werase(win);
    box(win, 0, 0);

    int title_x = (win_width - (int)strlen(title)) / 2;
    if (title_x < 1) title_x = 1;
    wattron(win, A_REVERSE);
    mvwprintw(win, 1, title_x, "%s", title);
    wattroff(win, A_REVERSE);

    for (int i = 0; i < count; i++) {
        if (i == *cursor) {
            wattron(win, A_REVERSE);
            mvwprintw(win, start_y + i, menu_start_x, "%s", items[i]);
            wattroff(win, A_REVERSE);
        } else {
            mvwprintw(win, start_y + i, menu_start_x, "%s", items[i]);
        }
    }

    wrefresh(win);

    keypad(win, TRUE);
    int ch = wgetch(win);

    // 깔끔하게 정돈된 키 분기 처리 스위치문
    switch (ch) {
        case 27: // ESC
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
