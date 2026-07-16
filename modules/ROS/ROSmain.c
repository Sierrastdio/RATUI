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

    // 1. 초기 셸 클리어 및 가상화면 리프레시
    clear();
    refresh();

    // 2. 가로 1:1 분할 레이아웃 계산
    int total_usable_w = COLS - 4;
    int win_w = total_usable_w / 2;
    int win_h = LINES - 6;
    int start_y = 2;

    int left_start_x = 2;
    int right_start_x = left_start_x + win_w;

    // 3. 하단 단독 알림 윈도우
    WINDOW *footer_win = newwin(3, COLS - 4, LINES - 4, 2);
    if (footer_win) {
        box(footer_win, 0, 0);
        mvwprintw(footer_win, 1, 2, "Use UP/DOWN to Navigate. Press [ENTER] to select. (ESC: Back)");
        wrefresh(footer_win);
    }

    // 4. 왼쪽 제어 윈도우 & 오른쪽 디스플레이 윈도우 분리 생성
    WINDOW *left_menu_win = UI_CREATE_WINDOW(win_h, win_w, start_y, left_start_x);
    WINDOW *right_data_win = UI_CREATE_WINDOW(win_h, win_w, start_y, right_start_x);

    // 오른쪽 윈도우 대기상태 정보 채우기
    if (right_data_win) {
        mvwprintw(right_data_win, win_h / 2, (win_w - 20) / 2, "[ No Active Task ]");
        wrefresh(right_data_win);
    }

    while(1) {
        // 왼쪽 윈도우 세션 내부에서 상태 입력 가로채기
        int result = SECTOR_MENU_WIN(left_menu_win, "ROS STORAGE MANAGEMENT", ros_items, ros_count, &ros_cursor, max_item_len);

        if (result == SIGN_CANCEL) break;

        // [이벤트 분기 1] 방향키 입력을 받아 커서 인덱스가 재지정되었을 경우
        if (result == SIGN_KEY_CHANGED) {
            if (right_data_win) {
                werase(right_data_win);
                box(right_data_win, 0, 0);

                if (ros_cursor == 0) {
                    mvwprintw(right_data_win, win_h / 2 - 1, (win_w - 24) / 2, "[1] STORAGE INFO PREVIEW");
                    mvwprintw(right_data_win, win_h / 2 + 1, (win_w - 26) / 2, "Press Enter to view status.");
                }
                else if (ros_cursor == 1) {
                    mvwprintw(right_data_win, win_h / 2 - 1, (win_w - 24) / 2, "[2] FILE EXPLORER PREVIEW");
                    mvwprintw(right_data_win, win_h / 2 + 1, (win_w - 28) / 2, "Press Enter to explore files.");
                }
                else if (ros_cursor == 2) {
                    mvwprintw(right_data_win, win_h / 2 - 1, (win_w - 24) / 2, "[3] COPY TO EDS PREVIEW ");
                    mvwprintw(right_data_win, win_h / 2 + 1, (win_w - 30) / 2, "Press Enter to trigger transfer.");
                }
                else if (ros_cursor == 3) {
                    mvwprintw(right_data_win, win_h / 2, (win_w - 18) / 2, "[ Return to Home ]");
                }

                wrefresh(right_data_win);
            }
            continue;
        }

        // [이벤트 분기 2] 사용자가 Enter를 쳐서 기능 진입을 확정했을 경우
        switch(result) {
            case 0:
                // 오른쪽 윈도우 인자를 패스하여 해당 구역 안에서 드로잉
                ROSfunc_show_info(right_data_win);
                break;

            case 1:
                // 오른쪽 윈도우 내부에서 파일 탐색기 및 관리 기능 가동
                ROSfunc_manage_storage(right_data_win);
                break;

            case 2:
                if (right_data_win) {
                    werase(right_data_win);
                    box(right_data_win, 0, 0);
                    mvwprintw(right_data_win, win_h / 2, (win_w - 24) / 2, ">> Transferring to EDS...");
                    wrefresh(right_data_win);
                }
                break;

            case 3:
                goto cleanup;
        }

        // 특정 세션 실행이 종료되어 복귀했을 때 오른쪽 상태창 레이아웃 디폴트 복원
        if (right_data_win) {
            werase(right_data_win);
            box(right_data_win, 0, 0);
            mvwprintw(right_data_win, win_h / 2, (win_w - 20) / 2, "[ No Active Task ]");
            wrefresh(right_data_win);
        }
    }

cleanup:
    // 동적 생성했던 구조들의 메모리를 완벽 회수하여 릭 방지
    if (left_menu_win) delwin(left_menu_win);
    if (right_data_win) delwin(right_data_win);
    if (footer_win) delwin(footer_win);
}
