#include "TUIsearchTag.h"
#include "TUIcommon.h"     /* PROMPT_LINE, STATUS_MSG */
#include "UI_PRINT.h"      /* UI_CLEAR_WINDOW */
#include "core.h"          /* core_list_by_tags, core_file_path */

#include <string.h>
#include <stdio.h>

/* ---------- [1] Search Tag ---------- */
int TUI_SEARCH_TAG(WINDOW *data_win, archdb_t *db)
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

        int len = PROMPT_LINE(data_win, 4 + tag_count, 2, label, one_tag, sizeof(one_tag));
        if (len <= 0) break;

        memcpy(tags_buf[tag_count], one_tag, sizeof(one_tag));
        tag_ptrs[tag_count] = tags_buf[tag_count];
        tag_count++;
    }

    if (tag_count == 0) {
        STATUS_MSG(data_win, "CANCELLED: at least 1 tag is required.");
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
