#include "TUIbrowseFS.h"
#include "TUIcommon.h"     /* STATUS_MSG, TUI_TAG_ASSIGN */
#include "UI_PRINT.h"
#include "SECTOR_MENU.h"
#include "PATH_CONFIG.h"   /* ARCHIVE_HOME_PATH */
#include "core.h"

#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

/* ---------- [4] Browse Filesystem (절대경로 파일 탐색기) ---------- */
#define FS_MAX_ITEMS  128
#define FS_PATH_MAX   1024

/* 파일을 선택했을 때 호출되는 지점.
 * TODO: 나중에 여기서 태그 할당 / 태그 수정 / 삭제 메뉴를 띄울 예정.
 * 지금은 정보만 보여준다. FS_FILE_SELECTED*/
void FS_FILE_SELECTED(WINDOW *data_win, archdb_t *db, const char *full_path, const char *file_name)
{
    (void)db; /* 태그 기능 붙이면 여기서 core_register_file 등에 사용 */

    char msg[1600];
    snprintf(msg, sizeof(msg), "SELECTED: %.255s  (path=%.1200s)  [tag 기능은 추후 지원]", file_name, full_path);
    STATUS_MSG(data_win, msg);
}

int TUI_BROWSE_FS(WINDOW *data_win, archdb_t *db)
{
    if (data_win == NULL || db == NULL) return -1;
    if (ARCHIVE_HOME_PATH == NULL) {
        STATUS_MSG(data_win, "ERROR: ARCHIVE_HOME_PATH not loaded (LOAD_CONFIG 먼저 호출 필요).");
        return -1;
    }

    char current_path[FS_PATH_MAX];
    strncpy(current_path, ARCHIVE_HOME_PATH, sizeof(current_path) - 1);
    current_path[sizeof(current_path) - 1] = '\0';

    int cursor = 0;

    while (1) {
        DIR *d = opendir(current_path);
        if (d == NULL) {
            STATUS_MSG(data_win, "ERROR: cannot open directory.");
            return -1;
        }

        char names[FS_MAX_ITEMS][256];
        int  is_dir[FS_MAX_ITEMS];
        int  count = 0;

        int at_home = (strcmp(current_path, ARCHIVE_HOME_PATH) == 0);

        struct dirent *entry;
        while ((entry = readdir(d)) != NULL && count < FS_MAX_ITEMS) {
            if (strcmp(entry->d_name, ".") == 0) continue;
            if (strcmp(entry->d_name, "..") == 0 && at_home) continue; /* 홈 밖으로는 못 나가게 */

            char full[FS_PATH_MAX + 256];
            snprintf(full, sizeof(full), "%s/%s", current_path, entry->d_name);

            struct stat st;
            if (stat(full, &st) != 0) continue;

            strncpy(names[count], entry->d_name, sizeof(names[count]) - 1);
            names[count][sizeof(names[count]) - 1] = '\0';
            is_dir[count] = S_ISDIR(st.st_mode);
            count++;
        }
        closedir(d);

        char item_bufs[FS_MAX_ITEMS][260];
        const char *item_ptrs[FS_MAX_ITEMS];
        for (int i = 0; i < count; i++) {
            snprintf(item_bufs[i], sizeof(item_bufs[i]), "%s%s", names[i], is_dir[i] ? "/" : "");
            item_ptrs[i] = item_bufs[i];
        }

        if (count == 0) {
            UI_CLEAR_WINDOW(data_win);
            mvwprintw(data_win, 2, 2, "[ %s ]", current_path);
            mvwprintw(data_win, 4, 2, "(empty)");
            wrefresh(data_win);
            int ch = wgetch(data_win);
            if (ch == 27 || ch == 'q' || ch == 'Q') break; /* 파일탐색기 종료 -> 메인 메뉴 */
            continue;
        }

        if (cursor >= count) cursor = count - 1;
        if (cursor < 0) cursor = 0;

        int result = SECTOR_MENU_WIN(data_win, current_path, item_ptrs, count, &cursor, SIGN_LEFT_ALIGN);

        switch (result) {
            case SIGN_KEY_CHANGED:
            case SIGN_REFRESH:
            case SIGN_DELETE:       /* 삭제는 아직 미구현 */
            case SIGN_UNREGISTER:   /* 파일탐색기에서는 'x' 동작 없음 (등록 해제는 태그 폴더뷰에서) */
                continue;

            case SIGN_CANCEL:       /* 파일탐색기 종료 -> 메인 메뉴 */
                return 0;

            case SIGN_TAG_ASSIGN: { /* 't' 키: 현재 커서가 가리키는 항목이 파일이면 태그 할당 창 호출 */
                int sel = cursor;
                if (sel >= 0 && sel < count && !is_dir[sel] && strcmp(names[sel], "..") != 0) {
                    char full[FS_PATH_MAX + 256];
                    snprintf(full, sizeof(full), "%s/%s", current_path, names[sel]);
                    TUI_TAG_ASSIGN(data_win, db, full, names[sel]);
                }
                continue;
            }

            default:
                break;
        }

        int sel = result;
        if (sel < 0 || sel >= count) continue;

        if (strcmp(names[sel], "..") == 0) {
            char *slash = strrchr(current_path, '/');
            if (slash != NULL && slash != current_path) {
                *slash = '\0';
            } else if (slash == current_path) {
                current_path[1] = '\0'; /* 루트("/") 보호 */
            }
            cursor = 0;
            continue;
        }

        if (is_dir[sel]) {
            char next_path[FS_PATH_MAX];
            int written = snprintf(next_path, sizeof(next_path), "%s/%s", current_path, names[sel]);
            if (written < 0 || written >= (int)sizeof(next_path)) {
                STATUS_MSG(data_win, "ERROR: path too long, cannot enter this directory.");
                continue;
            }
            memcpy(current_path, next_path, (size_t)written + 1);
            cursor = 0;
            continue;
        }

        /* 파일 선택 */
        {
            char full[FS_PATH_MAX + 256];
            snprintf(full, sizeof(full), "%s/%s", current_path, names[sel]);
            FS_FILE_SELECTED(data_win, db, full, names[sel]);
        }
    }

    return 0;
}
