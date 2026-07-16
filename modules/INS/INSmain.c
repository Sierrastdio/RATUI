#include <ncurses.h>
#include <string.h>
#include "UI_PRINT.h"
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
    int max_item_len = 35; // 가장 긴 아이템 길이 기준 버퍼 설정

    clear();
    refresh();

    // [연동] UI_PRINT의 전역 변수 레이아웃 지표 갱신
    UI_INIT_LAYOUT();

    // 하단 푸터 윈도우 생성 (전체 가로폭 반영)
    WINDOW *footer_win = newwin(3, COLS - 4, LINES - 4, 2);
    if (footer_win) {
        box(footer_win, 0, 0);
        mvwprintw(footer_win, 1, 2, "Use UP/DOWN to Navigate. Press [ENTER] to select. (ESC: Back)");
        wrefresh(footer_win);
    }

    // UI_PRINT에 정의된 전역 변수를 활용하여 좌/우 분할 윈도우 생성
    WINDOW *left_menu_win = UI_CREATE_WINDOW(UI_Win_Height, UI_Win_Width, UI_Start_Y, UI_Left_X);
    WINDOW *right_data_win = UI_CREATE_WINDOW(UI_Win_Height, UI_Win_Width, UI_Start_Y, UI_Right_X);

    if (right_data_win) {
        mvwprintw(right_data_win, UI_Win_Height / 2, (UI_Win_Width - 20) / 2, "[ No Active Task ]");
        wrefresh(right_data_win);
    }

    while (1) {
        // 분할 화면용 윈도우 메뉴 함수 호출
        int result = SECTOR_MENU_WIN(left_menu_win, "INS(Ingest Sector) MANAGEMENT", ins_items, ins_count, &ins_cursor, max_item_len);

        if (result == SIGN_CANCEL) {
            ins_cursor = 0; // 완전히 탈출할 때 커서 초기화
            break;
        }

        if (result == SIGN_REFRESH) {
            // 새로고침 요청 시 루프 다시 돌기
            continue;
        }

        // 방향키 조작 시 오른쪽 화면에 실시간 프리뷰 표출
        if (result == SIGN_KEY_CHANGED) {
            if (right_data_win) {
                werase(right_data_win);
                box(right_data_win, 0, 0);

                switch (ins_cursor) {
                    case 0:
                        mvwprintw(right_data_win, UI_Win_Height / 2 - 1, (UI_Win_Width - 24) / 2, "[1] FILE INGEST PREVIEW");
                        mvwprintw(right_data_win, UI_Win_Height / 2 + 1, (UI_Win_Width - 24) / 2, "Add new files into ROS.");
                        break;
                    case 1:
                        mvwprintw(right_data_win, UI_Win_Height / 2 - 1, (UI_Win_Width - 24) / 2, "[2] LIST VIEW PREVIEW  ");
                        mvwprintw(right_data_win, UI_Win_Height / 2 + 1, (UI_Win_Width - 26) / 2, "Show active file listings.");
                        break;
                    case 2:
                        mvwprintw(right_data_win, UI_Win_Height / 2 - 1, (UI_Win_Width - 24) / 2, "[3] COPY TO EDS PREVIEW ");
                        mvwprintw(right_data_win, UI_Win_Height / 2 + 1, (UI_Win_Width - 28) / 2, "Transfer assets to EDS sector.");
                        break;
                    case 3:
                        mvwprintw(right_data_win, UI_Win_Height / 2 - 1, (UI_Win_Width - 24) / 2, "[4] BACKUP CLONE PREVIEW");
                        mvwprintw(right_data_win, UI_Win_Height / 2 + 1, (UI_Win_Width - 28) / 2, "Replicate sector state to BKS.");
                        break;
                    case 4:
                        mvwprintw(right_data_win, UI_Win_Height / 2 - 1, (UI_Win_Width - 24) / 2, "[5] DUPLICATE DETECTOR ");
                        mvwprintw(right_data_win, UI_Win_Height / 2 + 1, (UI_Win_Width - 26) / 2, "Scan system for redundancy.");
                        break;
                    case 5:
                        mvwprintw(right_data_win, UI_Win_Height / 2, (UI_Win_Width - 18) / 2, "[ Return to Home ]");
                        break;
                }
                wrefresh(right_data_win);
            }
            continue;
        }

        // 엔터 입력 시 실제 하위 기능 실행
        int status = 0;
        switch (result) {
            case 0: status = INSfunc_handle_file_add(); break;
            case 1: status = INSfunc_list(); break;
            case 2: status = INS_copy_to_eds(); break;
            case 3: status = INS_copy_to_bks(); break;
            case 4: status = INS_quick_duplicate_check(); break;
            case 5: // [BACK] 선택 시
                ins_cursor = 0;
                goto cleanup;
        }

        // 하위 메뉴에서 r을 눌러 SIGN_REFRESH가 반환되었다면 메인 메뉴도 즉시 갱신
        if (status == SIGN_REFRESH) {
            continue;
        }

        // 기능 수행 후 복귀 시 오른쪽 데이터 윈도우 상태 초기화
        if (right_data_win) {
            werase(right_data_win);
            box(right_data_win, 0, 0);
            mvwprintw(right_data_win, UI_Win_Height / 2, (UI_Win_Width - 20) / 2, "[ No Active Task ]");
            wrefresh(right_data_win);
        }
    }

cleanup:
    // 할당했던 윈도우 리소스들 안전하게 해제
    if (left_menu_win) delwin(left_menu_win);
    if (right_data_win) delwin(right_data_win);
    if (footer_win) delwin(footer_win);
}
