#include <ncurses.h>
#include <string.h>
#include "UI_PRINT.h"
#include "SECTOR_MENU.h"
#include "PATH_CONFIG.h"
#include "../INS/INSfunc.h"
#include "ROSfunc.h"

void ROS_MAIN_LOOP(void) {
    const char *ros_items[] = {
        "[1] Storage Info & Status",
        "[2] Manage Stored Files (View/Delete)",
        "[3] Copy to EDS",
        "[BACK] To Home"
    };
    int ros_count = 4;
    int ros_cursor = 0;
    int max_item_len = 37;

    clear();
    refresh();

    // UI_PRINT에 정의된 전역 레이아웃 지표 일괄 계산
    UI_INIT_LAYOUT();

    // 하단 푸터 윈도우 생성 및 가이드 출력
    WINDOW *footer_win = newwin(3, COLS - 4, LINES - 4, 2);
    if (footer_win) {
        box(footer_win, 0, 0);
        mvwprintw(footer_win, 1, 2, "Use UP/DOWN to Navigate. Press [ENTER] to select. (ESC: Back)");
        wrefresh(footer_win);
    }

    // UI_PRINT의 전역 위치 변수(UI_Win_Height, UI_Win_Width 등)를 그대로 사용하여 좌/우 윈도우 생성
    WINDOW *left_menu_win = UI_CREATE_WINDOW(UI_Win_Height, UI_Win_Width, UI_Start_Y, UI_Left_X);
    WINDOW *right_data_win = UI_CREATE_WINDOW(UI_Win_Height, UI_Win_Width, UI_Start_Y, UI_Right_X);

    if (right_data_win) {
        const char *default_msg = "[ No Active Task ]";
        int msg_len = (int)strlen(default_msg);
        mvwprintw(right_data_win, UI_Win_Height / 2, (UI_Win_Width - msg_len) / 2, "%s", default_msg);
        wrefresh(right_data_win);
    }

    while (1) {
        int result = SECTOR_MENU_WIN(left_menu_win, "ROS STORAGE MANAGEMENT", ros_items, ros_count, &ros_cursor, max_item_len);

        if (result == SIGN_CANCEL) break;

        // 실시간 방향키 이동 프리뷰 렌더링 영역 (전역 변수 활용 정렬)
        if (result == SIGN_KEY_CHANGED) {
            if (right_data_win) {
                werase(right_data_win);
                box(right_data_win, 0, 0);

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
                    mvwprintw(right_data_win, UI_Win_Height / 2, (UI_Win_Width - (int)strlen(line1)) / 2, "%s", line1);
                } else {
                    mvwprintw(right_data_win, UI_Win_Height / 2 - 1, (UI_Win_Width - (int)strlen(line1)) / 2, "%s", line1);
                    mvwprintw(right_data_win, UI_Win_Height / 2 + 1, (UI_Win_Width - (int)strlen(line2)) / 2, "%s", line2);
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
                INS_copy_to_sector(EDS_PATH, "EDS");
                break;

            case 3:
                goto cleanup;
        }

        // 동작 완료 후 우측 데이터 윈도우 기본 화면으로 초기화
        if (right_data_win) {
            werase(right_data_win);
            box(right_data_win, 0, 0);
            const char *default_msg = "[ No Active Task ]";
            mvwprintw(right_data_win, UI_Win_Height / 2, (UI_Win_Width - (int)strlen(default_msg)) / 2, "%s", default_msg);
            wrefresh(right_data_win);
        }
    }

cleanup:
    if (left_menu_win) delwin(left_menu_win);
    if (right_data_win) delwin(right_data_win);
    if (footer_win) delwin(footer_win);
}
