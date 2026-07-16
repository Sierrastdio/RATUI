#include <ncurses.h>
#include <string.h>
#include "UI_PRINT.h"
#include "SECTOR_MENU.h"
#include "ROSfunc.h"

void ROS_MAIN_LOOP() {
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

    // [연동] UI 출력에 필요한 전역 레이아웃 지표 계산 작동
    UI_INIT_LAYOUT();

    // UI_PRINT에 정의된 전역 변수를 그대로 갖다 윈도우 생성에 활용
    WINDOW *footer_win = newwin(3, COLS - 4, LINES - 4, 2);
    if (footer_win) {
        box(footer_win, 0, 0);
        mvwprintw(footer_win, 1, 2, "Use UP/DOWN to Navigate. Press [ENTER] to select. (ESC: Back)");
        wrefresh(footer_win);
    }

    WINDOW *left_menu_win = UI_CREATE_WINDOW(UI_Win_Height, UI_Win_Width, UI_Start_Y, UI_Left_X);
    WINDOW *right_data_win = UI_CREATE_WINDOW(UI_Win_Height, UI_Win_Width, UI_Start_Y, UI_Right_X);

    if (right_data_win) {
        mvwprintw(right_data_win, UI_Win_Height / 2, (UI_Win_Width - 20) / 2, "[ No Active Task ]");
        wrefresh(right_data_win);
    }

    while(1) {
        int result = SECTOR_MENU_WIN(left_menu_win, "ROS STORAGE MANAGEMENT", ros_items, ros_count, &ros_cursor, max_item_len);

        if (result == SIGN_CANCEL) break;

        // 실시간 방향키 이동 프리뷰 렌더링 영역도 전역 UI 변수 기반으로 정렬 위치 조정
        if (result == SIGN_KEY_CHANGED) {
            if (right_data_win) {
                werase(right_data_win);
                box(right_data_win, 0, 0);

                switch (ros_cursor) {
                    case 0:
                        mvwprintw(right_data_win, UI_Win_Height / 2 - 1, (UI_Win_Width - 24) / 2, "[1] STORAGE INFO PREVIEW");
                        mvwprintw(right_data_win, UI_Win_Height / 2 + 1, (UI_Win_Width - 26) / 2, "Press Enter to view status.");
                        break;
                    case 1:
                        mvwprintw(right_data_win, UI_Win_Height / 2 - 1, (UI_Win_Width - 24) / 2, "[2] FILE EXPLORER PREVIEW");
                        mvwprintw(right_data_win, UI_Win_Height / 2 + 1, (UI_Win_Width - 28) / 2, "Press Enter to explore files.");
                        break;
                    case 2:
                        mvwprintw(right_data_win, UI_Win_Height / 2 - 1, (UI_Win_Width - 24) / 2, "[3] COPY TO EDS PREVIEW ");
                        mvwprintw(right_data_win, UI_Win_Height / 2 + 1, (UI_Win_Width - 30) / 2, "Press Enter to trigger transfer.");
                        break;
                    case 3:
                        mvwprintw(right_data_win, UI_Win_Height / 2, (UI_Win_Width - 18) / 2, "[ Return to Home ]");
                        break;
                }
                wrefresh(right_data_win);
            }
            continue;
        }

        switch(result) {
            case 0:
                ROSfunc_show_info(right_data_win);
                break;

            case 1:
                ROSfunc_manage_storage(right_data_win);
                break;

            case 2:
                if (right_data_win) {
                    werase(right_data_win);
                    box(right_data_win, 0, 0);
                    mvwprintw(right_data_win, UI_Win_Height / 2, (UI_Win_Width - 24) / 2, ">> Transferring to EDS...");
                    wrefresh(right_data_win);
                }
                break;

            case 3:
                goto cleanup;
        }

        if (right_data_win) {
            werase(right_data_win);
            box(right_data_win, 0, 0);
            mvwprintw(right_data_win, UI_Win_Height / 2, (UI_Win_Width - 20) / 2, "[ No Active Task ]");
            wrefresh(right_data_win);
        }
    }

cleanup:
    if (left_menu_win) delwin(left_menu_win);
    if (right_data_win) delwin(right_data_win);
    if (footer_win) delwin(footer_win);
}
