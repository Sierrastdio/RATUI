#include <ncurses.h>
#include <stdlib.h>
#include "SECTOR_MENU.h"
#include "PathConfig.h"
#include "help_UI.h" // 경로에 맞게 수정됨

int main() {
    LOAD_CONFIG();
    ENSURE_DIRECTORIES();

    const char *home_items[] = {"[ROS]", "[INS]", "[EDS]", "[BKS]", "[TRS]", "[EXIT]"};
    int home_count = 6;
    int currentSEC = HOME;
    int cursor = 0;

    setenv("ESCDELAY", "25", 1);
    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    while (1) {
        if (currentSEC == HOME) {
            // SECTOR_MENU가 입력 처리와 도움말 표시를 전담함
            int result = SECTOR_MENU("MAIN DASHBOARD", home_items, home_count, &cursor, HOME);
            
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
        }
    }
    return 0;
}