/*
 * TUIfunc.c
 *
 * TUImain.c 의 메인 메뉴 case 들이 호출하는 실제 작업 함수들.
 * 여기서 core_* 함수를 직접 부른다 (db_* 는 직접 호출하지 않는다).
 */

#include "TUIfunc.h"
#include "UI_PRINT.h"
#include "SECTOR_MENU.h"
#include "PATH_CONFIG.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>

/* ---------- 내부 헬퍼 ---------- */

/* 하단에 메시지 띄우고 키 입력 대기 */
static void status_msg(WINDOW *win, const char *msg)
{
    if (win == NULL) return;

    int h = getmaxy(win);
    int w = getmaxx(win);
    int print_y = (h > 2) ? h - 2 : 0;
    int print_w = (w > 4) ? w - 4 : 10;

    mvwprintw(win, print_y, 2, "%-*s", print_w, msg);
    wrefresh(win);
    wgetch(win);
}

/* "라벨: " 출력 후 그 자리에서 한 줄 입력받기. 반환값은 입력된 글자 수 */
static int prompt_line(WINDOW *win, int y, int x, const char *label, char *out, int out_cap)
{
    echo();
    curs_set(1);

    mvwprintw(win, y, x, "%s", label);
    wrefresh(win);

    int label_len = (int)strlen(label);
    mvwgetnstr(win, y, x + label_len, out, out_cap - 1);

    noecho();
    curs_set(0);
    return (int)strlen(out);
}

/* ---------- 태그 할당 (파일명은 이미 정해진 상태로 호출됨) ----------
 * Browse Filesystem 에서 파일을 고르고 't'를 눌렀을 때 호출된다.
 * 파일명은 파라미터로 받으므로 여기서 따로 입력받지 않는다. */

static int TUI_do_tag_assign(WINDOW *data_win, archdb_t *db, const char *file_name)
{
    if (data_win == NULL || db == NULL || file_name == NULL) return -1;

    char ver_buf[16] = {0};

    UI_CLEAR_WINDOW(data_win);
    mvwprintw(data_win, 2, 2, "[ ASSIGN TAG ] %.100s", file_name);
    wrefresh(data_win);

    prompt_line(data_win, 4, 2, "Version (default 1) : ", ver_buf, sizeof(ver_buf));
    uint16_t version = (strlen(ver_buf) > 0) ? (uint16_t)atoi(ver_buf) : 1;
    if (version == 0) version = 1;

    /* 태그를 한 번에 하나씩 반복 입력받기 (빈 입력이면 종료) */
    char tags_buf[CORE_MAX_PATH_TAGS][ARCHDB_TAG_LEN + 1];
    const char *tag_ptrs[CORE_MAX_PATH_TAGS];
    int tag_count = 0;

    mvwprintw(data_win, 6, 2, "Enter tags one by one (blank to finish):");
    wrefresh(data_win);

    while (tag_count < CORE_MAX_PATH_TAGS) {
        char one_tag[ARCHDB_TAG_LEN + 1] = {0};
        char label[32];
        snprintf(label, sizeof(label), "  Tag #%d : ", tag_count + 1);

        int len = prompt_line(data_win, 7 + tag_count, 2, label, one_tag, sizeof(one_tag));
        if (len <= 0) break; /* 빈 입력 -> 태그 추가 종료 */

        memcpy(tags_buf[tag_count], one_tag, sizeof(one_tag));
        tag_ptrs[tag_count] = tags_buf[tag_count];
        tag_count++;
    }

    if (tag_count == 0) {
        status_msg(data_win, "CANCELLED: at least 1 tag is required.");
        return -1;
    }

    uint32_t new_id = core_register_file(db, file_name, version, tag_ptrs, tag_count);

    char result_msg[512];
    if (new_id != ARCHDB_INVALID_ID) {
        int written = snprintf(result_msg, sizeof(result_msg),
                                "SUCCESS: '%s' registered (id=%u, v%u, tags=", file_name, new_id, version);
        for (int i = 0; i < tag_count && written < (int)sizeof(result_msg) - 8; i++) {
            written += snprintf(result_msg + written, sizeof(result_msg) - written,
                                 "%s%s", tags_buf[i], (i < tag_count - 1) ? "," : ")");
        }
    } else {
        snprintf(result_msg, sizeof(result_msg),
                 "ERROR: failed to register '%s'.", file_name);
    }
    status_msg(data_win, result_msg);

    return (new_id != ARCHDB_INVALID_ID) ? 0 : -1;
}

/* ---------- [1] Search Tag ---------- */

int TUI_handle_search_tag(WINDOW *data_win, archdb_t *db)
{
    if (data_win == NULL || db == NULL) return -1;

    char tags_buf[CORE_MAX_PATH_TAGS][ARCHDB_TAG_LEN + 1];
    const char *tag_ptrs[CORE_MAX_PATH_TAGS];
    int tag_count = 0;

    UI_CLEAR_WINDOW(data_win);
    mvwprintw(data_win, 2, 2, "[ SEARCH TAG ]");
    mvwprintw(data_win, 3, 2, "Enter tags one by one (blank to finish, AND search):");
    wrefresh(data_win);

    while (tag_count < CORE_MAX_PATH_TAGS) {
        char one_tag[ARCHDB_TAG_LEN + 1] = {0};
        char label[32];
        snprintf(label, sizeof(label), "  Tag #%d : ", tag_count + 1);

        int len = prompt_line(data_win, 4 + tag_count, 2, label, one_tag, sizeof(one_tag));
        if (len <= 0) break;

        memcpy(tags_buf[tag_count], one_tag, sizeof(one_tag));
        tag_ptrs[tag_count] = tags_buf[tag_count];
        tag_count++;
    }

    if (tag_count == 0) {
        status_msg(data_win, "CANCELLED: at least 1 tag is required.");
        return -1;
    }

    uint32_t ids[64];
    int n = core_list_by_tags(db, tag_ptrs, tag_count, ids, 64);

    char joined_tags[128] = {0};
    int jw = 0;
    for (int i = 0; i < tag_count && jw < (int)sizeof(joined_tags) - 6; i++) {
        jw += snprintf(joined_tags + jw, sizeof(joined_tags) - jw,
                        "%s%s", tags_buf[i], (i < tag_count - 1) ? "+" : "");
    }

    UI_CLEAR_WINDOW(data_win);
    mvwprintw(data_win, 2, 2, "[ RESULT: %s ] (%d found)", joined_tags, n);

    int max_show = getmaxy(data_win) - 5;
    if (max_show < 1) max_show = 1;

    for (int i = 0; i < n && i < max_show; i++) {
        file_record_t rec;
        if (db_file_read(db, ids[i], &rec) != 0) continue;

        char full_path[128] = {0};
        core_file_path(db, ids[i], full_path, sizeof(full_path));

        mvwprintw(data_win, 4 + i, 2, "[id=%u] %-30s v%u  (%s)",
                  rec.file_id, rec.file_name, rec.version, full_path);
    }

    wrefresh(data_win);
    wgetch(data_win);

    return 0;
}

/* ---------- [3] Browse (Folder View) ---------- */

#define BROWSE_MAX_FILES  256
#define BROWSE_MAX_ITEMS  (1 + CORE_MAX_CHILD_TAGS + BROWSE_MAX_FILES)

int TUI_handle_browse(WINDOW *data_win, archdb_t *db)
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
        char item_bufs[BROWSE_MAX_ITEMS][200];
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
            continue; /* 태그 폴더뷰에서는 't' 동작 없음 (파일 등록은 Browse Filesystem에서) */
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
                status_msg(data_win, msg);
            }
            continue;
        }
    }

    return 0;
}

/* ---------- [4] Browse Filesystem (절대경로 파일 탐색기) ---------- */

#define FS_MAX_ITEMS  256
#define FS_PATH_MAX   1024

/* 파일을 선택했을 때 호출되는 지점.
 * TODO: 나중에 여기서 태그 할당 / 태그 수정 / 삭제 메뉴를 띄울 예정.
 * 지금은 정보만 보여준다. */
static void fs_on_file_selected(WINDOW *data_win, archdb_t *db, const char *full_path, const char *file_name)
{
    (void)db; /* 태그 기능 붙이면 여기서 core_register_file 등에 사용 */

    char msg[1600];
    snprintf(msg, sizeof(msg), "SELECTED: %.255s  (path=%.1200s)  [tag 기능은 추후 지원]", file_name, full_path);
    status_msg(data_win, msg);
}

int TUI_handle_browse_fs(WINDOW *data_win, archdb_t *db)
{
    if (data_win == NULL || db == NULL) return -1;
    if (ARCHIVE_HOME_PATH == NULL) {
        status_msg(data_win, "ERROR: ARCHIVE_HOME_PATH not loaded (LOAD_CONFIG 먼저 호출 필요).");
        return -1;
    }

    char current_path[FS_PATH_MAX];
    strncpy(current_path, ARCHIVE_HOME_PATH, sizeof(current_path) - 1);
    current_path[sizeof(current_path) - 1] = '\0';

    int cursor = 0;

    while (1) {
        DIR *d = opendir(current_path);
        if (d == NULL) {
            status_msg(data_win, "ERROR: cannot open directory.");
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

        char item_bufs[FS_MAX_ITEMS][300];
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

        if (result == SIGN_KEY_CHANGED || result == SIGN_REFRESH) continue;

        if (result == SIGN_CANCEL) break; /* 파일탐색기 종료 -> 메인 메뉴 */

        if (result == SIGN_DELETE) continue; /* 삭제는 아직 미구현 */

        if (result == SIGN_TAG_ASSIGN) {
            /* 't' 키: 현재 커서가 가리키는 항목이 파일이면 태그 할당 창 호출 */
            int sel = cursor;
            if (sel >= 0 && sel < count && !is_dir[sel] && strcmp(names[sel], "..") != 0) {
                TUI_do_tag_assign(data_win, db, names[sel]);
            }
            continue;
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
                status_msg(data_win, "ERROR: path too long, cannot enter this directory.");
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
            fs_on_file_selected(data_win, db, full, names[sel]);
        }
    }

    return 0;
}