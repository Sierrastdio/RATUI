#include <ncurses.h>
#include <stdlib.h>
#include "SECTOR_MENU.h"
#include "PATH_CONFIG.h"
#include "help_UI.h"

int main() {
    LOAD_CONFIG();          /*  config.rtuconf 의 설정 불러오기   */
    ENSURE_DIRECTORIES();   /*  PATH_CONFIG.h 의 함수*/

    const char *home_items[] = {"[ROS]", "[INS]", "[EDS]", "[BKS]", "[TRS]", "[EXIT]"};
    int home_count = 6;
    int currentSEC = HOME;
    int cursor = 0;

    setenv("ESCDELAY", "25", 1);    /*  ESC 키 반응 기본값 1000ms에서 25ms 로 단축  */
    initscr();              /*  ncurses 모드 시작   */
    noecho();               /*  사용자가 키보드로 입력하는 글자가 터미널 화면에 보이지 않게(Echo 금지) 만드는 함수  */
    curs_set(0);            /*  터미널 커서 숨기기    */
    keypad(stdscr, TRUE);   /*  특수키 입력 활성화(F1~F12, 방향키, PgUp, PgDn 등)   */

    while (1) {
        if (currentSEC == HOME) {
            /*  SECTOR_MENU가 입력 처리와 도움말 표시를 전담함  */
            int result = SECTOR_MENU("MAIN DASHBOARD", home_items, home_count, &cursor, HOME);
            /*  위 는 SECTOR_MENU 함수에 항목값(문자), 항목 개수, 커서 위치를 보내게 함   */
            if (result >= 0) {
                /*  결괏값(정수형)에 따라 각 섹터의 메인루프를 실행해도록 함    */
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
        }
    }
    return 0;
}