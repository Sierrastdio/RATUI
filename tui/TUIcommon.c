/*
 * TUIcommon.c
 */
#include "TUIcommon.h"
#include "UI_PRINT.h"      /* UI_CLEAR_WINDOW */
#include "SECTOR_MENU.h"   /* SECTOR_MENU_WIN, SIGN_* */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ---------- 내부 헬퍼 ---------- */

/* 하단에 메시지 띄우고 키 입력 대기 */
void STATUS_MSG(WINDOW *win, const char *msg)
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
int PROMPT_LINE(WINDOW *win, int y, int x, const char *label, char *out, int out_cap)
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

int TUI_TAG_ASSIGN(WINDOW *data_win, archdb_t *db, const char *full_path, const char *file_name)
{
    if (data_win == NULL || db == NULL || file_name == NULL) return -1;

    char ver_buf[16] = {0};

    UI_CLEAR_WINDOW(data_win);
    mvwprintw(data_win, 2, 2, "[ ASSIGN TAG ] %.100s", file_name);
    wrefresh(data_win);

    PROMPT_LINE(data_win, 4, 2, "Version (default 1) : ", ver_buf, sizeof(ver_buf));
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

        int len = PROMPT_LINE(data_win, 7 + tag_count, 2, label, one_tag, sizeof(one_tag));
        if (len <= 0) break; /* 빈 입력 -> 태그 추가 종료 */

        /* 이미 입력한 태그와 중복인지 확인 (대소문자 무시하고 정규화해서 비교) */
        char norm_new[ARCHDB_TAG_LEN];
        db_tag_normalize(norm_new, one_tag);

        int is_dup = 0;
        for (int i = 0; i < tag_count; i++) {
            char norm_existing[ARCHDB_TAG_LEN];
            db_tag_normalize(norm_existing, tags_buf[i]);
            if (memcmp(norm_new, norm_existing, ARCHDB_TAG_LEN) == 0) {
                is_dup = 1;
                break;
            }
        }

        if (is_dup) {
            mvwprintw(data_win, 7 + tag_count, 2 + (int)strlen(label) + ARCHDB_TAG_LEN + 2,
                      "-> 이미 입력한 태그입니다, 다시 입력하세요");
            wrefresh(data_win);
            continue; /* 같은 칸(tag_count 그대로)에서 다시 입력받기 */
        }

        memcpy(tags_buf[tag_count], one_tag, sizeof(one_tag));
        tag_ptrs[tag_count] = tags_buf[tag_count];
        tag_count++;
    }

    if (tag_count == 0) {
        STATUS_MSG(data_win, "CANCELLED: at least 1 tag is required.");
        return -1;
    }

    /* 실제 파일 내용을 해시해서, 이름 또는 내용이 겹치는 기존 등록이 있는지 확인 */
    uint64_t content_hash = CORE_HASH_FILE(full_path);
    core_dup_result_t dup = CORE_CHECK_DUP_CONTENT(db, content_hash, file_name);

    if (dup.kind == CORE_DUP_SAME_NAME_SAME_CONTENT) {
        char dup_msg[300];
        snprintf(dup_msg, sizeof(dup_msg),
                 "CANCELLED: identical file '%.100s' (same name & content) already registered (id=%u).",
                 file_name, dup.existing_file_id);
        STATUS_MSG(data_win, dup_msg);
        return -1;
    }
    if (dup.kind == CORE_DUP_DIFF_NAME_SAME_CONTENT) {
        char dup_msg[300];
        snprintf(dup_msg, sizeof(dup_msg),
                 "CANCELLED: this exact content is already registered under a different name (id=%u).",
                 dup.existing_file_id);
        STATUS_MSG(data_win, dup_msg);
        return -1;
    }

    uint32_t new_id = CORE_REGISTER_FILE(db, file_name, version, content_hash, tag_ptrs, tag_count);

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
    STATUS_MSG(data_win, result_msg);

    return (new_id != ARCHDB_INVALID_ID) ? 0 : -1;
}

/* 파일에 걸린 태그만을 트림된 문자열로 모으는 콜백 (TUI_UNTAG_FILE 전용) */
typedef struct {
    char tags[CORE_MAX_PATH_TAGS][ARCHDB_TAG_LEN + 1];
    int  count;
} untag_collect_ctx_t;

static int UNTAG_COLLECT_CB(uint32_t file_id, const char tag[ARCHDB_TAG_LEN], void *ctx_v)
{
    (void)file_id;
    untag_collect_ctx_t *ctx = (untag_collect_ctx_t *)ctx_v;

    if (ctx->count >= CORE_MAX_PATH_TAGS) return 0;

    char trimmed[ARCHDB_TAG_LEN + 1];
    memcpy(trimmed, tag, ARCHDB_TAG_LEN);
    trimmed[ARCHDB_TAG_LEN] = '\0';
    for (int i = ARCHDB_TAG_LEN - 1; i >= 0 && trimmed[i] == ' '; i--) trimmed[i] = '\0';

    memcpy(ctx->tags[ctx->count], trimmed, sizeof(trimmed));
    ctx->count++;
    return 0;
}

/* 파일에 걸린 태그 중 하나를 골라서 떼어낸다 (파일 자체는 삭제 안 함).
 * 태그 목록을 SECTOR_MENU_WIN으로 보여주고 하나 고르면 CORE_UNTAG_FILE 호출. */
void TUI_UNTAG_FILE(WINDOW *data_win, archdb_t *db, uint32_t file_id, const char *file_name)
{
    untag_collect_ctx_t ctx = { .count = 0 };
    db_tag_foreach_by_file(db, file_id, UNTAG_COLLECT_CB, &ctx);

    if (ctx.count == 0) {
        STATUS_MSG(data_win, "ERROR: no tags found for this file.");
        return;
    }

    const char *tag_ptrs[CORE_MAX_PATH_TAGS];
    for (int i = 0; i < ctx.count; i++) tag_ptrs[i] = ctx.tags[i];

    int cursor = 0;
    char title[160];
    snprintf(title, sizeof(title), "REMOVE TAG - %.100s", file_name);

    while (1) {
        int result = SECTOR_MENU_WIN(data_win, title, tag_ptrs, ctx.count, &cursor, SIGN_LEFT_ALIGN);

        if (result == SIGN_KEY_CHANGED || result == SIGN_REFRESH) {
            continue; /* 방향키로 커서만 움직인 경우 - 같은 목록 다시 그림 */
        }

        if (result == SIGN_CANCEL) {
            return; /* q/ESC - 아무 것도 안 하고 나가기 */
        }

        if (result == SIGN_DELETE || result == SIGN_TAG_ASSIGN || result == SIGN_UNREGISTER) {
            continue; /* 여기선 의미 없는 키 - 무시하고 계속 대기 */
        }

        if (result < 0 || result >= ctx.count) return;

        int rc = CORE_UNTAG_FILE(db, file_id, ctx.tags[result]);
        char msg[200];
        if (rc == 0) {
            snprintf(msg, sizeof(msg), "REMOVED: tag '%s' from '%.100s'", ctx.tags[result], file_name);
        } else {
            snprintf(msg, sizeof(msg), "ERROR: cannot remove last remaining tag from '%.100s'.", file_name);
        }
        STATUS_MSG(data_win, msg);
        return;
    }
}

/* 파일을 완전히 삭제한다 (태그 전부 해제 + 파일 레코드 자체 삭제).
 * 되돌릴 수 없는 동작이라 y/n 확인을 받는다. */
void TUI_UNREGISTER_FILE(WINDOW *data_win, archdb_t *db, uint32_t file_id, const char *file_name)
{
    char prompt[220];
    snprintf(prompt, sizeof(prompt), "Delete '%.100s' from archive? This cannot be undone. (y/n)", file_name);

    UI_CLEAR_WINDOW(data_win);
    mvwprintw(data_win, 2, 2, "%s", prompt);
    wrefresh(data_win);

    int ch = wgetch(data_win);
    if (ch != 'y' && ch != 'Y') {
        STATUS_MSG(data_win, "CANCELLED.");
        return;
    }

    int rc = CORE_DELETE_FILE(db, file_id);
    char msg[220];
    if (rc == 0) {
        snprintf(msg, sizeof(msg), "DELETED: '%.100s' removed from archive (tags + record).", file_name);
    } else {
        snprintf(msg, sizeof(msg), "ERROR: failed to delete '%.100s'.", file_name);
    }
    STATUS_MSG(data_win, msg);
}
