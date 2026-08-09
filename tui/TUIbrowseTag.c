#include "TUIbrowseTag.h"
#include "TUIcommon.h"     /* STATUS_MSG, TUI_UNTAG_FILE, TUI_UNREGISTER_FILE */
#include "UI_PRINT.h"
#include "SECTOR_MENU.h"   /* SECTOR_MENU_WIN, SIGN_* */
#include "core.h"          /* core_list_child_tags, core_list_files_in_view */

#include <string.h>
#include <stdio.h>

/* ---------- [3] Browse (TAG Folder View) ---------- */
#define BROWSE_MAX_FILES  128
#define BROWSE_MAX_ITEMS  (1 + CORE_MAX_CHILD_TAGS + BROWSE_MAX_FILES)

int TUI_BROWSE_TAG_FOLDER_VIEW(WINDOW *data_win, archdb_t *db)
{
    if (data_win == NULL || db == NULL) return -1;

    char selected_buf[CORE_MAX_PATH_TAGS][ARCHDB_TAG_LEN + 1];
    const char *selected_ptrs[CORE_MAX_PATH_TAGS];
    int selected_count = 0;

    int cursor = 0;

    while (1) {
        /* 현재 위치(selected_ptrs) 기준으로 하위 태그 + 파일 목록 다시 계산 */
        core_child_tag_t child_tags[CORE_MAX_CHILD_TAGS];
        int child_n = core_list_child_tags(db, selected_ptrs, selected_count, child_tags, CORE_MAX_CHILD_TAGS);

        uint32_t file_ids[BROWSE_MAX_FILES];
        int file_n = (selected_count > 0)
            ? core_list_files_in_view(db, selected_ptrs, selected_count, file_ids, BROWSE_MAX_FILES)
            : 0;

        int has_up = (selected_count > 0);

        /* 화면에 뿌릴 아이템 라벨 구성: [..] -> [DIR] 태그들 -> 파일들 */
        char item_bufs[BROWSE_MAX_ITEMS][150];
        const char *item_ptrs[BROWSE_MAX_ITEMS];
        int idx = 0;

        if (has_up) {
            snprintf(item_bufs[idx], sizeof(item_bufs[idx]), "[ .. ] (up one level)");
            item_ptrs[idx] = item_bufs[idx];
            idx++;
        }

        int child_start = idx;
        for (int i = 0; i < child_n && idx < BROWSE_MAX_ITEMS; i++) {
            snprintf(item_bufs[idx], sizeof(item_bufs[idx]), "[DIR] %.4s (%d)", child_tags[i].tag, child_tags[i].file_count);
            item_ptrs[idx] = item_bufs[idx];
            idx++;
        }

        int file_start = idx;
        for (int i = 0; i < file_n && idx < BROWSE_MAX_ITEMS; i++) {
            file_record_t rec;
            if (db_file_read(db, file_ids[i], &rec) != 0) continue;
            snprintf(item_bufs[idx], sizeof(item_bufs[idx]), "%s  (v%u)", rec.file_name, rec.version);
            item_ptrs[idx] = item_bufs[idx];
            idx++;
        }
        int item_count = idx;

        /* 타이틀 = 현재 경로 */
        char title[160];
        if (selected_count == 0) {
            snprintf(title, sizeof(title), "ROOT");
        } else {
            int tw = 0;
            title[0] = '\0';
            for (int i = 0; i < selected_count && tw < (int)sizeof(title) - 6; i++) {
                tw += snprintf(title + tw, sizeof(title) - tw, "/%s", selected_buf[i]);
            }
        }

        if (item_count == 0) {
            /* 빈 폴더: q/ESC로 상위 이동, 그 외엔 계속 대기 */
            UI_CLEAR_WINDOW(data_win);
            mvwprintw(data_win, 2, 2, "[ %s ]", title);
            mvwprintw(data_win, 4, 2, "(empty)");
            wrefresh(data_win);
            int ch = wgetch(data_win);
            if (ch == 27 || ch == 'q' || ch == 'Q') {
                if (selected_count > 0) { selected_count--; cursor = 0; continue; }
                break;
            }
            continue;
        }

        if (cursor >= item_count) cursor = item_count - 1;
        if (cursor < 0) cursor = 0;

        int result = SECTOR_MENU_WIN(data_win, title, item_ptrs, item_count, &cursor, SIGN_LEFT_ALIGN);

        if (result == SIGN_KEY_CHANGED || result == SIGN_REFRESH) {
            continue; /* 커서 이동/새로고침 - 같은 레벨 다시 그림 */
        }

        if (result == SIGN_CANCEL) {
            if (selected_count > 0) {
                selected_count--; /* 상위 폴더로 */
                cursor = 0;
                continue;
            }
            break; /* 최상위에서 취소 -> browse 종료, 메인 메뉴로 */
        }

        if (result == SIGN_DELETE) {
            continue; /* 삭제는 아직 미구현 - 필요하면 여기서 db_file_delete/db_tag_remove 연결 */
        }

        if (result == SIGN_TAG_ASSIGN) {
            /* 't' 키: 커서가 파일을 가리키고 있으면 그 파일의 태그 중 하나를 골라 제거 */
            int sel = cursor;
            if (sel >= file_start && sel < file_start + file_n) {
                int fi = sel - file_start;
                file_record_t rec;
                if (db_file_read(db, file_ids[fi], &rec) == 0) {
                    TUI_UNTAG_FILE(data_win, db, file_ids[fi], rec.file_name);
                }
            }
            continue;
        }

        if (result == SIGN_UNREGISTER) {
            /* 'x' 키: 커서가 파일을 가리키고 있으면 완전 삭제(태그 전부 + 파일 레코드) */
            int sel = cursor;
            if (sel >= file_start && sel < file_start + file_n) {
                int fi = sel - file_start;
                file_record_t rec;
                if (db_file_read(db, file_ids[fi], &rec) == 0) {
                    TUI_UNREGISTER_FILE(data_win, db, file_ids[fi], rec.file_name);
                }
            }
            continue;
        }

        /* ENTER로 아이템 선택 */
        int sel = result;

        if (has_up && sel == 0) {
            selected_count--;
            cursor = 0;
            continue;
        }

        if (sel >= child_start && sel < child_start + child_n) {
            int ci = sel - child_start;
            if (selected_count < CORE_MAX_PATH_TAGS) {
                strncpy(selected_buf[selected_count], child_tags[ci].tag, ARCHDB_TAG_LEN);
                selected_buf[selected_count][ARCHDB_TAG_LEN] = '\0';
                selected_ptrs[selected_count] = selected_buf[selected_count];
                selected_count++;
                cursor = 0;
            }
            continue;
        }

        if (sel >= file_start && sel < file_start + file_n) {
            int fi = sel - file_start;
            file_record_t rec;
            if (db_file_read(db, file_ids[fi], &rec) == 0) {
                char full_path[128] = {0};
                core_file_path(db, file_ids[fi], full_path, sizeof(full_path));
                char msg[400];
                snprintf(msg, sizeof(msg), "id=%u  %s  v%u  path=/%s",
                         rec.file_id, rec.file_name, rec.version, full_path);
                STATUS_MSG(data_win, msg);
            }
            continue;
        }
    }

    return 0;
}
