/*
 * TUIfunc.c
 *
 * TUImain.c 의 메인 메뉴 case 들이 호출하는 실제 작업 함수들.
 * 여기서 core_* 함수를 직접 부른다 (db_* 는 직접 호출하지 않는다).
 */

#include "TUIfunc.h"
#include "UI_PRINT.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

/* ---------- [1] Register Tag ---------- */

int TUI_handle_register_tag(WINDOW *data_win, archdb_t *db)
{
    if (data_win == NULL || db == NULL) return -1;

    char file_name[ARCHDB_NAME_LEN] = {0};
    char ver_buf[16] = {0};

    UI_CLEAR_WINDOW(data_win);
    mvwprintw(data_win, 2, 2, "[ REGISTER TAG ]");
    wrefresh(data_win);

    if (prompt_line(data_win, 4, 2, "File name : ", file_name, sizeof(file_name)) <= 0) {
        status_msg(data_win, "CANCELLED: file name is required.");
        return -1;
    }

    prompt_line(data_win, 5, 2, "Version (default 1) : ", ver_buf, sizeof(ver_buf));
    uint16_t version = (strlen(ver_buf) > 0) ? (uint16_t)atoi(ver_buf) : 1;
    if (version == 0) version = 1;

    /* 태그를 한 번에 하나씩 반복 입력받기 (빈 입력이면 종료) */
    char tags_buf[CORE_MAX_PATH_TAGS][ARCHDB_TAG_LEN + 1];
    const char *tag_ptrs[CORE_MAX_PATH_TAGS];
    int tag_count = 0;

    mvwprintw(data_win, 7, 2, "Enter tags one by one (blank to finish):");
    wrefresh(data_win);

    while (tag_count < CORE_MAX_PATH_TAGS) {
        char one_tag[ARCHDB_TAG_LEN + 1] = {0};
        char label[32];
        snprintf(label, sizeof(label), "  Tag #%d : ", tag_count + 1);

        int len = prompt_line(data_win, 8 + tag_count, 2, label, one_tag, sizeof(one_tag));
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

/* ---------- [2] Search Tag ---------- */

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