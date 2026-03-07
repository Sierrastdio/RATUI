#include <ncurses.h>
#include <stdlib.h> // setenv 함수를 위해 필요
#include "SECTOR_MENU.h"
#include "PathConfig.h" // 경로 설정을 위해 헤더 포함

int main() {

    // 1. 프로그램 시작하자마자 설정 읽기 (최우선 순위)
    LOAD_CONFIG();
    ENSURE_DIRECTORIES(); // 모든 폴더는 이 함수가 알아서 만듦

    // 1. 사용할 메뉴 목록 정의 (문자열 배열)
    const char *home_items[] = {"[ROS]", "[INS]", "[EDS]", "[BKS]", "[TRS]", "[EXIT]"};
    int home_count = 6;

    int currentSEC = HOME;
    int cursor = 0; // 커서 위치 초기화

    setenv("ESCDELAY", "25", 1);    /*ESC 키 반응 1000ms를 25ms로 줄여줌*/

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
                    case 2: 
                        EDS_MAIN_LOOP();
                        currentSEC = HOME;
                        break;

                    case 3:
                        BKS_MAIN_LOOP();
                        currentSEC = HOME;
                        break;

                    case 4:
                        TRS_MAIN_LOOP();
                        currentSEC = HOME;
                        break;
                        
                    case 5: 
                        currentSEC = EXIT;
                            // ncurses 종료
                            endwin();
                            return 0;

                    case 6: currentSEC = CANCEL;    break;
                }
                cursor = 0;
            }
        } 
    }
}