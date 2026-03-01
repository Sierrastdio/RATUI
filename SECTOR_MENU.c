#include "SECTOR_MENU.h"

int SECTOR_MENU(char *title, char *items[], int count, int *cursor_pos) {
    clear();

    // 1. 전달받은 제목을 상단에 출력
    attron(A_BOLD);
    mvprintw(1, 2, "--- %s ---", title);
    attroff(A_BOLD);

    // 2. 전달받은 아이템 리스트를 루프 돌며 출력
    for (int i = 0; i < count; i++) {
        if (i == *cursor_pos) {
            attron(A_REVERSE);
            // i번째 아이템(items[i])을 출력
            mvprintw(i + 3, 2, " > %s ", items[i]);
            attroff(A_REVERSE);
        } else {
            mvprintw(i + 3, 4, "%s", items[i]);
        }
    }

    mvprintw(LINES - 2, 2, "[UP/DOWN] Move | [ENTER] Select | [Q] Quit");
    refresh();

    // 3. 키 입력 처리
    int ch = getch();

    if (ch == KEY_UP && *cursor_pos > 0) {
        (*cursor_pos)--;
    } 
    else if (ch == KEY_DOWN && *cursor_pos < count - 1) {
        (*cursor_pos)++;
    } 
    else if (ch == 10) { // Enter 키
        return *cursor_pos; // 선택된 번호를 반환
    } 
    else if (ch == 'q' || ch == 'Q') {
        return -1; // 종료 신호 (EXIT 대신 숫자로 명시)
    }

    return -2; // 아무 변화 없음 (다시 그려라)
}