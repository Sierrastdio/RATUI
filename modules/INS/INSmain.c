#include <ncurses.h>
#include "SECTOR_MENU.h"
#include "INSfunc.h"

void INS_MAIN_LOOP() {
    // [1] static으로 선언하여 함수를 나갔다 들어와도, r을 눌러도 커서 위치를 기억함
    static int ins_cursor = 0;

    const char *ins_items[] = {
        "[1] File add to ROS",
        "[2] INS File List View",
        "[3] Copy to EDS",
        "[4] Clone to BackUp(BKS)",
        "[5] Check for File Duplication",
        "[BACK] To Home"
    };
    int ins_count = 6;

    while (1) {
        // [2] SECTOR_MENU 호출 (ins_cursor의 주소 전달)
        int result = SECTOR_MENU("INS(Ingest Sector) MANAGEMENT", ins_items, ins_count, &ins_cursor, INS);

        if (result == SIGN_CANCEL) {
            ins_cursor = 0; // 완전히 종료할 때만 커서 초기화 (선택 사항)
            return;
        }

        if (result == SIGN_REFRESH) {
            // 아무것도 안 하고 continue만 해도 static 변수 덕분에
            // 현재 위치에서 메뉴가 다시 그려지며 '새로고침' 효과가 남
            continue;
        }

        // [4] 하위 기능 실행
        // 하위 함수에서 r을 눌렀을 때도 바로 리프레시되길 원한다면 status 체크를 추가하세요.
        int status = 0;
        switch(result) {
            case 0: status = INSfunc_handle_file_add(); break;
            case 1: status = INSfunc_list(); break;
            case 2: status = INS_copy_to_eds(); break;
            case 3: status = INS_copy_to_bks(); break;
            case 4: status = INS_quick_duplicate_check(); break;
            case 5: // [BACK] 선택 시
                ins_cursor = 0;
                return;
        }

        // 하위 메뉴에서 r을 눌러 -3이 반환되었다면 메인 메뉴도 즉시 갱신
        if (status == SIGN_REFRESH) continue;
    }
}
