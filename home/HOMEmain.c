#include <ncurses.h>
#include "SECTOR_MENU.h"

int main() {
    // 1. 사용할 메뉴 목록 정의 (문자열 배열)
    char *home_items[] = {"[ROS]", "[INS]", "[EDS]", "[BKS]", "[EXIT]"};
    int home_count = 5;

    int currentSEC = HOME;
    int cursor = 0; // 커서 위치 초기화

    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    while (1) {
        if (currentSEC == HOME) {
            int result = SECTOR_MENU("MAIN DASHBOARD", home_items, home_count, &cursor);
            
            if (result >= 0) {
                // 홈 메뉴에서는 인덱스 순서대로 섹션을 배치했다고 가정
                switch(result){
                    case 0: 
                        ROS_MAIN_LOOP();    //ROSmain.c가 실행됨.
                        currentSEC = HOME;  //돌아오면 다시 HOME 상태로 유지   
                        break;
                    case 1: 
                        INS_MAIN_LOOP();
                        currentSEC = HOME;  //돌아오면 다시 HOME 상태로 유지   
                        break;
                    case 2: currentSEC = EDS;   break;
                    case 3: currentSEC = BKS;   break;
                    case 4: 
                        currentSEC = EXIT;
                            // ncurses 종료
                            endwin();
                            return 0;
                    case 5: currentSEC = CANCEL;    break;
                }
                cursor = 0;
            }
        } 
    }
}