#include <ncurses.h>
#include <string.h>
#include "SECTOR_MENU.h"
#include "help_UI.h"
#include "UI_PRINT.h"

#define MAX_VISIBLE 15  /* 한번에 표시할 항목 개수 최대 15개   */

// 1. 기존 원본 SECTOR_MENU 함수 (완벽 유지)
int SECTOR_MENU(const char *title, const char *options[], int count, int *current_cursor, int sector_id) {
    int start_index = 0;
    int key;

    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    /* MAX_VISIBLE 보다 커서위치 값이 더 크면 처음 커서위치 = 0 으로 돌아가게 함.  */
    if (*current_cursor >= MAX_VISIBLE) {
        start_index = *current_cursor - (MAX_VISIBLE - 1);
    }

    while (1) {
        clear();

        // 1. 고정 문자열 크기 + 가변 title 길이 합산
        int title_len = (sizeof(" ===  (Total: ) === ") - 1) + (int)strlen(title);

        // 2. 순수 나눗셈 루프로 자릿수 합산
        int temp_count = count;
        do {
           title_len++;
           temp_count /= 10;
        } while (temp_count > 0);

        // 3. 인자로 길이만 던지면 UI_Center_x가 자동으로 갱신됨
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
                return SIGN_DELETE;

            case 'q':
            case 'Q':
            case 27: //27 means 'ESC'
                return SIGN_CANCEL;

            case 'r':
            case 'R':
                return SIGN_REFRESH; // 리프레시 신호
        }
    }
}

// 2. 신규 분할 화면용 윈도우 전용 메뉴 함수 (실시간 이벤트 구동형으로 완성)
int SECTOR_MENU_WIN(WINDOW *win, const char *title, const char *items[], int count, int *cursor, int max_len) {
    int win_width = getmaxx(win);
    int win_height = getmaxy(win);

    // 테두리 안쪽을 고려하여 x축 중앙 정렬 좌표 계산
    int menu_start_x = (win_width - max_len) / 2;
    if (menu_start_x < 1) menu_start_x = 1;

    // 세로 기준 중앙 배치용 좌표 계산
    int start_y = (win_height / 2) - (count / 2);
    if (start_y < 3) start_y = 3; // 타이틀과 안 겹치게 고정 버퍼 배치

    // 윈도우 내부 지우기 및 테두리 초기화
    werase(win);
    box(win, 0, 0);

    // 1. 윈도우 상단 가로 중앙에 타이틀 역상으로 배치
    int title_x = (win_width - (int)strlen(title)) / 2;
    if (title_x < 1) title_x = 1;
    wattron(win, A_REVERSE);
    mvwprintw(win, 1, title_x, "%s", title);
    wattroff(win, A_REVERSE);

    // 2. 항목들을 해당 윈도우에만 드로잉
    for (int i = 0; i < count; i++) {
        if (i == *cursor) {
            wattron(win, A_REVERSE);
            mvwprintw(win, start_y + i, menu_start_x, "%s", items[i]);
            wattroff(win, A_REVERSE);
        } else {
            mvwprintw(win, start_y + i, menu_start_x, "%s", items[i]);
        }
    }

    // 윈도우 버퍼 물리 화면에 반영
    wrefresh(win);

    // [중요] stdscr의 getch() 대신, 이 윈도우의 wgetch()를 사용하여 키 동기화 꼬임 해결
    keypad(win, TRUE);
    int ch = wgetch(win);

    if (ch == 27) {
        return SIGN_CANCEL;
    }
    else if (ch == KEY_UP || ch == 'w' || ch == 'W') {
        if (*cursor > 0) (*cursor)--;
        else *cursor = count - 1;
        return SIGN_KEY_CHANGED; // 변경되었음을 부모 루프에 즉시 알려 동기화 유도 (-4)
    }
    else if (ch == KEY_DOWN || ch == 's' || ch == 'S') {
        if (*cursor < count - 1) (*cursor)++;
        else *cursor = 0;
        return SIGN_KEY_CHANGED; // 변경되었음을 부모 루프에 즉시 알려 동기화 유도 (-4)
    }
    else if (ch == '\n' || ch == ' ') {
        return *cursor;
    }
    else if (ch == 'd' || ch == 'D') {
        return SIGN_DELETE; // -2 반환
    }
    else if (ch == 'r' || ch == 'R') {
        return SIGN_REFRESH; // -3 반환
    }

    return SIGN_KEY_CHANGED;
}
