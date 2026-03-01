#include <ncurses.h>
#include <unistd.h>

int main() {
    int x = 10, y = 5; // 시작 좌표
    int ch;
    int max_y, max_x;

    // ncurses 초기화
    initscr();            // ncurses 모드 시작
    raw();                // 줄 버퍼링 비활성화 (엔터 없이 입력)
    keypad(stdscr, TRUE); // 방향키 등 특수키 입력 허용
    noecho();             // 입력한 키를 화면에 출력하지 않음
    curs_set(0);          // 커서 숨기기

    // 현재 터미널의 최대 크기 가져오기
    getmaxyx(stdscr, max_y, max_x);

    while (1) {
        // 1. 화면 지우기
        clear();

        // 2. 안내 메시지 및 현재 좌표 출력
        mvprintw(0, 0, "Move with Arrow Keys or WASD | [Q]: Quit");
        mvprintw(1, 0, "Current Position: y=%d, x=%d", y, x);

        // 3. 특정 위치로 이동하여 문자 출력
        // ncurses는 y가 먼저, x가 나중입니다: mvaddch(y, x, character)
        mvaddch(y, x, '@');

        // 4. 화면 물리적 갱신 (반드시 호출해야 화면에 반영됨)
        refresh();

        // 5. 키 입력 받기
        ch = getch();

        if (ch == 'q' || ch == 'Q') break;

        // 6. 좌표 업데이트 및 경계 체크
        switch (ch) {
            case 'w': case 'W': case KEY_UP:
                if (y > 2) y--;
                break;
            case 's': case 'S': case KEY_DOWN:
                if (y < max_y - 1) y++;
                break;
            case 'a': case 'A': case KEY_LEFT:
                if (x > 0) x--;
                break;
            case 'd': case 'D': case KEY_RIGHT:
                if (x < max_x - 1) x++;
                break;
        }
    }

    // ncurses 모드 종료 및 터미널 복구
    endwin();
    return 0;
}