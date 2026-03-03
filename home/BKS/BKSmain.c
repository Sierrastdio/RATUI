#include <ncurses.h>
#include "SECTOR_MENU.h"
#include "BKSfunc.h"

void BKS_MAIN_LOOP() {
    const char *bks_items[] = {
        "[1]",
        "[2]",
        "[3]",
        "[BACK] To Home",
    };

    int bks_count = 4;
    int bks_cursor = 0;

    while(1) {
        int result = SECTOR_MENU("BKS(BackUp Storage)", bks_items, bks_count, &bks_cursor);
        /*
        왜 &bks_cursor를 쓸까?
        SECTOR_MENU 함수가 실행되면서 사용자가 메뉴를 옮길 때마다 bks_cursor의 값을 업데이트해야 하기 때문입니다.
        bks_count (값만 보냄): 함수는 "아, 항목이 총 5개구나"라는 정보만 알면 됩니다. 함수 안에서 이 숫자를 바꿀 일이 없죠.
        &bks_cursor (주소를 보냄): 함수는 "현재 커서 위치가 담긴 방 주소가 여기네?"라고 인지합니다. 
        사용자가 키보드로 커서를 내리면, 함수가 그 주소로 직접 찾아가서 bks_cursor의 값을 0에서 1로, 1에서 2로 직접 바꿔버립니다.
        */

        if(result == -1) return;

        switch(result) {
            case 0: break;
            case 1: break;
            case 2: break;
            case 3: return;
        }
    }
}