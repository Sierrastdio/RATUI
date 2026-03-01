#include "SECTOR_MENU.h"

void INS_MAIN_LOOP() {
    // 1. INS 섹션에서만 쓸 메뉴들
    char *ros_items[] = {
        "[1] File add", //파일 검사(예: 중복검사) 로직은 이 항목안에서 다룰 예정
        "[2] INS File List View", 
        "[3] Copy to EDS", 
        "[4] Clone to BackUp(BKS)",
        "[5] Check for File Duplication",    // 파일중복검사만 따로 해주는 항목
        "[BACK] To Home"
    };
    int ros_count = 6;
    int ros_cursor = 0; // 이 커서는 HOME의 커서와 별개로 움직입니다.

    while (1) {
        // 2. 똑같은 라이브러리 함수 호출 (제목과 배열만 변경)
        int result = SECTOR_MENU("INS(Ingest Sector) MANAGEMENT", ros_items, ros_count, &ros_cursor);

        if (result >= 0) {
            switch(result) {
                case 0:
                    // 저장소에 추가할 파일을 받고 특정 파일 검사 함수가 동작함. 
                    break;
                case 1:
                    // 현재 위치 파일 목록 (왜있지?)
                    break;
                case 2:
                    //EDS(EditSector)로 클론하는 함수
                    break;
                case 3: 
                    //BKS(EditSector)로 클론하는 함수. 파일 클론하는 함수와 경로로 옮기는 함수 따로 만드는게 좋을까?
                    break;
                case 4:
                    // 파일 중복검사 함수
                    break;
                case 5:
                    return; // 함수 종료 -> homemain.c의 루프로 복귀
            }
        }

        if (result == -1) return; // 'Q' 누르면 종료 (혹은 메인으로)
    }
}