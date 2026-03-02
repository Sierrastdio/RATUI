#include "SECTOR_MENU.h"

void ROS_MAIN_LOOP() {
    // 1. ROS 섹션에서만 쓸 메뉴들
    const char *ros_items[] = {
        "[1] Scan Directory", 
        "[2] File List View", 
        "[3] Copy to EDS", 
        "[BACK] To Home"
    };
    int ros_count = 4;
    int ros_cursor = 0; // 이 커서는 HOME의 커서와 별개로 움직입니다.

    while (1) {
        // 2. 똑같은 라이브러리 함수 호출 (제목과 배열만 변경)
        int result = SECTOR_MENU("ROS STORAGE MANAGEMENT", ros_items, ros_count, &ros_cursor);

        if (result >= 0) {
            switch(result) {
                case 0:
                    // 스캔 로직 실행
                    break;
                case 1:
                    // 파일 목록 보기 (여기서 또 SECTOR_MENU를 쓸 수 있음!)
                    break;
                case 2:
                    // 복사 로직 실행
                    break;
                case 3: 
                    return; // 함수 종료 -> homemain.c의 루프로 복귀
            }
        }

        if (result == -1) return; // 'Q' 누르면 종료 (혹은 메인으로)
    }
}