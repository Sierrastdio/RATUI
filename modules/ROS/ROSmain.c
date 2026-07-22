#include <ncurses.h>
#include "UI_PRINT.h"
#include "SECTOR_MENU.h"
#include "PATH_CONFIG.h"
#include "../INS/INSfunc.h"
#include "ROSfunc.h"

void ROS_MAIN_LOOP(void) {
    static const char *ros_items[] = {
        "[1] Storage Info & Status",
        "[2] Manage Stored Files (View/Delete)",
        "[3] Copy to EDS",
        "[BACK] To Home"
    };
    const int ros_count = (int)(sizeof(ros_items) / sizeof(ros_items[0]));
    int ros_cursor = 0;
    const int max_item_len = 37;

    clear();
    refresh();

    // UI_PRINT에 정의된 전역 레이아웃 지표 일괄 계산
    UI_INIT_LAYOUT();

    // 하단 푸터 윈도우 생성 및 가이드 출력
    WINDOW *footer_win = UI_CREATE_FOOTER_WINDOW();

    // UI_PRINT의 전역 위치 변수(UI_Win_Height, UI_Win_Width 등)를 그대로 사용하여 좌/우 윈도우 생성
    WINDOW *left_menu_win = UI_CREATE_WINDOW(UI_Win_Height, UI_Win_Width, UI_Start_Y, UI_Left_X);
    WINDOW *right_data_win = UI_CREATE_WINDOW(UI_Win_Height, UI_Win_Width, UI_Start_Y, UI_Right_X);

    UI_PRINT_IDLE(right_data_win);

    while (1) {
        int result = SECTOR_MENU_WIN(left_menu_win, "ROS STORAGE MANAGEMENT", ros_items, ros_count, &ros_cursor, max_item_len);

        if (result == SIGN_CANCEL) break;

        // 실시간 방향키 이동 프리뷰 렌더링 영역 (전역 변수 활용 정렬)
        if (result == SIGN_KEY_CHANGED) {
            if (right_data_win) {
                UI_CLEAR_WINDOW(right_data_win);

                const char *line1 = "";
                const char *line2 = "";

                switch (ros_cursor) {
                    case 0:
                        line1 = "[1] STORAGE INFO PREVIEW";
                        line2 = "Press Enter to view status.";
                        break;
                    case 1:
                        line1 = "[2] FILE EXPLORER PREVIEW";
                        line2 = "Press Enter to explore files.";
                        break;
                    case 2:
                        line1 = "[3] COPY TO EDS PREVIEW";
                        line2 = "Press Enter to trigger transfer.";
                        break;
                    case 3:
                        line1 = "[ Return to Home ]";
                        break;
                }

                if (ros_cursor == 3) {
                    UI_PRINT_CENTERED(right_data_win, UI_GET_WIN_CENTER_Y(0), line1);
                } else {
                    UI_PRINT_CENTERED(right_data_win, UI_GET_WIN_CENTER_Y(-1), line1);
                    UI_PRINT_CENTERED(right_data_win, UI_GET_WIN_CENTER_Y(1), line2);
                }

                wrefresh(right_data_win);
            }
            continue;
        }

        switch (result) {
            case 0:
                ROSfunc_show_info(right_data_win);
                break;

            case 1:
                ROSfunc_manage_storage(right_data_win);
                break;

            case 2:
                // INSfunc의 복사 로직 재활용
                INS_copy_to_sector(right_data_win, EDS_PATH, "EDS");
                break;

            case 3:
                goto cleanup;
        }

        // 동작 완료 후 우측 데이터 윈도우 기본 화면으로 초기화
        UI_PRINT_IDLE(right_data_win);
    }

cleanup:
    if (left_menu_win) delwin(left_menu_win);
    if (right_data_win) delwin(right_data_win);
    if (footer_win) delwin(footer_win);
}
