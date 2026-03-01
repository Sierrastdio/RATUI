#include <ncurses.h>

int main() {
    // 가상의 파일 저장소 목록
    char *items[] = {"[INS] Ingest", "[ROS] Main Storage", "[BKS] Backup", "[Q] Quit"};
    int n_items = 4;
    int cur = 0; // 현재 커서 위치 (0부터 시작)

    initscr();
    noecho();
    curs_set(0); 
    keypad(stdscr, TRUE); // 방향키 사용 설정

    while (1) {
        clear();
        mvprintw(0, 0, "--- Archive Manager ---");

        // 목록 출력 루프
        for (int i = 0; i < n_items; i++) {
            if (i == cur) {
                attron(A_REVERSE); // 선택된 항목 반전
                mvprintw(i + 2, 2, " > %s ", items[i]);
                attroff(A_REVERSE);
            } else {
                mvprintw(i + 2, 4, "%s", items[i]);
            }
        }

        int ch = getch();
        if (ch == 'q' || ch == 'Q') break;

        // 방향키 로직
        if (ch == KEY_UP && cur > 0) cur--;
        if (ch == KEY_DOWN && cur < n_items - 1) cur++;
        if (ch == 10) { // 엔터 키
            if (cur == 3) break; // Quit 선택 시 종료
            mvprintw(n_items + 4, 2, "Selected: %s", items[cur]);
            refresh();
            getch(); // 확인을 위해 아무 키나 대기
        }
    }

    endwin();
    return 0;
}