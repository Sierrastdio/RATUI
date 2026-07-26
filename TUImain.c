#include <ncurses.h>
#include <stdlib.h>
#include "UI_PRINT.h"
#include "SECTOR_MENU.h"
#include "TUIfunc.h"
#include "PATH_CONFIG.h"
#include "core.h"

#define ARCHDB_BASE_DIR "."   /* files.db / file_tags.db 가 생성될 위치 */

void TUI_MAIN_LOOP(archdb_t *db)
{
    /* static으로 선언하여 함수를 나갔다 들어와도, r을 눌러도 커서 위치를 기억함 */
    static int tui_cursor = 0;

    static const char *main_menu_items[] = {
        "[1]. Search Tag",
        "[2]. Browse (Folder View)",
        "[3]. Browse Filesystem",
        "[4]. Exit"
    };

    const int main_menu_count = sizeof(main_menu_items) / sizeof(main_menu_items[0]);
    const int max_menu_item_len = 35;

    clear();
    refresh();

    UI_INIT_LAYOUT();

    WINDOW *footer_win = UI_CREATE_FOOTER_WINDOW();

    WINDOW *left_menu_win = UI_CREATE_WINDOW(UI_Win_Height, UI_Win_Width, UI_Start_Y, UI_Left_X);
    WINDOW *right_data_win = UI_CREATE_WINDOW(UI_Win_Height, UI_Win_Width, UI_Start_Y, UI_Right_X);

    UI_PRINT_IDLE(right_data_win);

    while (1) {
        int result = SECTOR_MENU_WIN(left_menu_win, "MAIN MENU", main_menu_items,
                                      main_menu_count, &tui_cursor, max_menu_item_len);

        if (result == SIGN_CANCEL) {
            tui_cursor = 0;
            break;
        }

        if (result == SIGN_REFRESH) {
            /* 새로고침 요청 시 루프 다시 돌기 */
            continue;
        }

        if (result == SIGN_TAG_ASSIGN || result == SIGN_DELETE) {
            /* 메인 메뉴에서는 't'/'d' 의미 없음 - 무시 */
            continue;
        }

        if (result == SIGN_KEY_CHANGED) {
            if (right_data_win) {
                UI_CLEAR_WINDOW(right_data_win);

                switch (tui_cursor) {
                    case 0:
                        UI_PRINT_CENTERED(right_data_win, UI_GET_WIN_CENTER_Y(-1), "[1] Search Tag Preview");
                        UI_PRINT_CENTERED(right_data_win, UI_GET_WIN_CENTER_Y(1), "Find existing Tags in files");
                        break;
                    case 1:
                        UI_PRINT_CENTERED(right_data_win, UI_GET_WIN_CENTER_Y(-1), "[2] Browse Preview");
                        UI_PRINT_CENTERED(right_data_win, UI_GET_WIN_CENTER_Y(1), "Explore tags like folders");
                        break;
                    case 2:
                        UI_PRINT_CENTERED(right_data_win, UI_GET_WIN_CENTER_Y(-1), "[3] Browse Filesystem Preview");
                        UI_PRINT_CENTERED(right_data_win, UI_GET_WIN_CENTER_Y(1), "Explore real files from HOME_PATH, press 't' to tag");
                        break;
                    case 3:
                        UI_PRINT_CENTERED(right_data_win, UI_GET_WIN_CENTER_Y(1), "Exit the application");
                        break;
                }
                wrefresh(right_data_win);
            }
            continue;
        }

        int status = 0;

        switch (tui_cursor) {
            case 0:
                status = TUI_handle_search_tag(right_data_win, db);
                break;
            case 1:
                status = TUI_handle_browse(right_data_win, db);
                break;
            case 2:
                status = TUI_handle_browse_fs(right_data_win, db);
                break;
            case 3:
                tui_cursor = 0;
                goto cleanup;
        }

        (void)status; /* 필요하면 여기서 실패 시 별도 처리 추가 가능 */

        UI_PRINT_IDLE(right_data_win);
    }

    cleanup:
        /* 할당했던 윈도우 리소스들 안전하게 해제 */
        if (left_menu_win) delwin(left_menu_win);
        if (right_data_win) delwin(right_data_win);
        if (footer_win) delwin(footer_win);
}

int main(void)
{
    LOAD_CONFIG();           /* config.ratui 에서 ARCHIVE_HOME_PATH 읽기 */
    ENSURE_HOME_DIRECTORY(); /* 없으면 생성 */

    archdb_t db;
    if (archdb_open(&db, ARCHDB_BASE_DIR) != 0) {
        fprintf(stderr, "Error: failed to open archive DB in '%s'\n", ARCHDB_BASE_DIR);
        return 1;
    }

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    TUI_MAIN_LOOP(&db);

    endwin();
    archdb_close(&db);
    return 0;
}