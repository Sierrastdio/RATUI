#include <ncurses.h>
#include <string.h>
#include "SECTOR_MENU.h"

int SECTOR_MENU(const char *title, const char *options[], int count, int *current_cursor) {
    int choice = -1;
    int key;

    // 키패드 활성화 (방향키 사용을 위해 필수)
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);

    while (1) {
        clear();
        
        // 상단 타이틀 바
        attron(A_REVERSE);
        mvprintw(1, 2, " === %s === ", title);
        attroff(A_REVERSE);

        // 메뉴 목록 출력
        for (int i = 0; i < count; i++) {
            if (i == *current_cursor) {
                attron(A_REVERSE); // 선택된 항목 강조
                mvprintw(3 + i, 4, " > %s ", options[i]);
                attroff(A_REVERSE);
            } else {
                mvprintw(3 + i, 4, "   %s ", options[i]);
            }
        }

        mvprintw(LINES - 2, 2, " [UP/DOWN]: Move | [ENTER]: Select ");
        refresh();

        key = getch();

        switch (key) {
            case KEY_UP:
                // [순환 로직] 맨 위에서 위를 누르면 맨 아래로 이동
                if (*current_cursor > 0) {
                    (*current_cursor)--;
                } else {
                    *current_cursor = count - 1; 
                }
                break;

            case KEY_DOWN:
                // [순환 로직] 맨 아래에서 아래를 누르면 맨 위로 이동
                if (*current_cursor < count - 1) {
                    (*current_cursor)++;
                } else {
                    *current_cursor = 0; 
                }
                break;

            case 10: // ENTER 키
                choice = *current_cursor;
                break;

            default:
                // q키 등 다른 키 입력은 무시됨
                break;
        }

        // 선택이 완료되면 루프 탈출
        if (choice != -1) {
            break;
        }
    }

    return choice;
}