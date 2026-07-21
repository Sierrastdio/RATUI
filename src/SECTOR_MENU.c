#include <ncurses.h>
#include <string.h>
#include "SECTOR_MENU.h"
#include "help_UI.h"
#include "UI_PRINT.h"

#define MAX_VISIBLE 15  /* 한번에 표시할 항목 개수 최대 15개 */

// 전체 화면(stdscr) 기반 중앙 정렬 메인 메뉴
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

        // 전체 화면(COLS) 기준 전역 X좌표 계산 함수 활용
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

// 분할 윈도우 전용 서브 메뉴
int SECTOR_MENU_WIN(WINDOW *win, const char *title, const char *items[], int count, int *cursor, int max_len) {
    int win_width = getmaxx(win);
    int win_height = getmaxy(win);

    int menu_start_x;
    int start_y;

    // [레이아웃 연동] 좌측 정렬 규칙(SIGN_LEFT_ALIGN)일 때 좌표 초기화 설정
    if (max_len == SIGN_LEFT_ALIGN) {
        menu_start_x = 4; // 좌측에서 4칸 띄우기
        start_y = 3;      // 타이틀 아래 여백을 두고 3라인부터 차례대로 출력
    } else {
        // 기존 중앙 정렬 방식 유지
        menu_start_x = (win_width - max_len) / 2;
        if (menu_start_x < 1) menu_start_x = 1;

        start_y = (win_height / 2) - (count / 2);
        if (start_y < 3) start_y = 3;
    }

    werase(win);
    box(win, 0, 0);

    // 상단 타이틀 정렬 (UI_PRINT 전역 헬퍼 기반 폭 계산)
    UI_GET_RIGHT_WIN_CENTER_X((int)strlen(title));
    int title_x = UI_Center_x;
    if (title_x < 1) title_x = 1;

    wattron(win, A_REVERSE);
    mvwprintw(win, 1, title_x, "%s", title);
    wattroff(win, A_REVERSE);

    // 목록 출력 루프
    for (int i = 0; i < count; i++) {
        // 데이터 창 높이를 넘어가지 않도록 안전 제어
        if (start_y + i >= win_height - 1) break;

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

    switch (ch) {
        case 27:
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
