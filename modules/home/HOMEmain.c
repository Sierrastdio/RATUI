#include <ncurses.h>
#include <stdlib.h>
#include "SECTOR_MENU.h"
#include "PATH_CONFIG.h"
#include "help_UI.h"
#include "UI_PRINT.h"

int main() {
    LOAD_CONFIG();          /* config.rtuconf 의 설정 불러오기 */
    ENSURE_DIRECTORIES();   /* PATH_CONFIG.h 의 함수 */

    static const char *home_items[] = {"[ROS]", "[INS]", "[EDS]", "[BKS]", "[TRS]", "[EXIT]"};
    const int home_count = (int)(sizeof(home_items) / sizeof(home_items[0]));
    int currentSEC = HOME; //'HOME'은 SECTOR_MENU.h 에서 옴.
    int cursor = 0;
    int result = 0;

    setenv("ESCDELAY", "25", 1);    /* ESC 키 반응 기본값 1000ms에서 25ms 로 단축 */
    initscr();              /* ncurses 모드 시작 */
    noecho();               /* 사용자가 키보드로 입력하는 글자가 터미널 화면에 보이지 않게(Echo 금지) 만드는 함수 */
    curs_set(0);            /* 터미널 커서 숨기기 */
    keypad(stdscr, TRUE);   /* 특수키 입력 활성화(F1~F12, 방향키, PgUp, PgDn 등) */

    while (1) {
        if (currentSEC == HOME) {
            clear();
            refresh();

            // 1. 현재 화면 규격 상태 갱신
            UI_INIT_LAYOUT();

            // 2. 대시보드용 단일 메인 윈도우 생성
            const int main_win_w = COLS - 4;
            const int main_win_h = LINES - 6;
            const int main_win_y = 2;
            const int main_win_x = 2;

            WINDOW *main_win = UI_CREATE_WINDOW(main_win_h, main_win_w, main_win_y, main_win_x);

            if (main_win == NULL) {
                result = SECTOR_MENU("===MAIN DASHBOARD===", home_items, home_count, &cursor, HOME);
                if (result >= 0) {
                    switch(result){
                        case 0: ROS_MAIN_LOOP(); break;
                        case 1: INS_MAIN_LOOP(); break;
                        case 2: EDS_MAIN_LOOP(); break;
                        case 3: BKS_MAIN_LOOP(); break;
                        case 4: TRS_MAIN_LOOP(); break;
                        case 5: endwin(); return 0;
                    }
                    cursor = 0;
                }
                continue;
            }

            // 방향키 조작(SIGN_KEY_CHANGED) 시 창을 부수지 않고 내부에서 대기하는 서브 루프
            while (1) {
                result = SECTOR_MENU_WIN(main_win, "=== MAIN DASHBOARD ===", home_items, home_count, &cursor, 10);

                // 단순히 커서만 바뀐 경우, 윈도우를 해제하지 않고 이 내부 루프에서 즉시 렌더링하도록 유도
                if (result == SIGN_KEY_CHANGED) {
                    continue;
                }

                // 취소 신호나 정상 선택 이벤트가 들어오면 서브 루프 탈출 후 메인 로직 처리
                if (result == SIGN_CANCEL) {
                    break;
                }

                if (result >= 0) {
                    delwin(main_win); // 섹터 진입 전에 윈도우 리소스 해제

                    /* 결괏값(정수형)에 따라 각 섹터의 메인루프를 실행해도록 함 */
                    switch(result){
                        case 0: ROS_MAIN_LOOP(); break;
                        case 1: INS_MAIN_LOOP(); break;
                        case 2: EDS_MAIN_LOOP(); break;
                        case 3: BKS_MAIN_LOOP(); break;
                        case 4: TRS_MAIN_LOOP(); break;
                        case 5: endwin(); return 0;
                    }
                    cursor = 0;
                    break; // 섹터에서 복귀 시 대시보드 창을 새로 구성하기 위해 탈출
                }
            }

            // SIGN_CANCEL 등으로 루프를 빠져나왔을 때 안전하게 리소스 해제
            if (result == SIGN_CANCEL) {
                delwin(main_win);
            }
        }
    }
    return 0;
}
