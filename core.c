/*
 * core.c
 *
 * db_* 저수준 함수들을 조합해 "가상 경로" 개념을 구현한다.
 */

#include "core.h"

#include <string.h>

/* ============================================================
 *  경로 파싱
 * ============================================================ */

int core_path_split(const char *path, char tags_out[][ARCHDB_TAG_LEN + 1], int max_tags)
{
    if (path == NULL) return 0;

    int count = 0;
    const char *p = path;

    while (*p != '\0' && count < max_tags) {
        while (*p == '/') p++;          /* 연속 슬래시("A//B") 건너뛰기 */
        if (*p == '\0') break;

        int len = 0;
        while (p[len] != '\0' && p[len] != '/') len++;

        int copy_len = (len < ARCHDB_TAG_LEN) ? len : ARCHDB_TAG_LEN;
        memcpy(tags_out[count], p, (size_t)copy_len);
        tags_out[count][copy_len] = '\0';
        count++;

        p += len;
    }
    return count;
}

/* db_tag_foreach_by_file 콜백으로 buf 에 "TAG1/TAG2" 이어붙이기 */
typedef struct {
    char   *buf;
    size_t  buf_len;
    size_t  written;
    int     failed;
} path_build_ctx_t;

static int build_path_cb(uint32_t file_id, const char tag[ARCHDB_TAG_LEN], void *ctx_v)
{
    (void)file_id;
    path_build_ctx_t *ctx = (path_build_ctx_t *)ctx_v;

    char tag_str[ARCHDB_TAG_LEN + 1];
    memcpy(tag_str, tag, ARCHDB_TAG_LEN);
    tag_str[ARCHDB_TAG_LEN] = '\0';
    /* 오른쪽 공백 트림 (정규화 때 채운 패딩 제거) */
    for (int i = ARCHDB_TAG_LEN - 1; i >= 0 && tag_str[i] == ' '; i--) tag_str[i] = '\0';

    size_t need = strlen(tag_str) + (ctx->written > 0 ? 1 : 0); /* 구분자 '/' 포함 */
    if (ctx->written + need >= ctx->buf_len) {
        ctx->failed = 1;
        return 1; /* 순회 중단 */
    }

    if (ctx->written > 0) {
        ctx->buf[ctx->written++] = '/';
    }
    memcpy(ctx->buf + ctx->written, tag_str, strlen(tag_str));
    ctx->written += strlen(tag_str);
    ctx->buf[ctx->written] = '\0';
    return 0;
}

int core_file_path(archdb_t *db, uint32_t file_id, char *buf, size_t buf_len)
{
    if (buf == NULL || buf_len == 0) return -1;
    buf[0] = '\0';

    path_build_ctx_t ctx = { buf, buf_len, 0, 0 };
    db_tag_foreach_by_file(db, file_id, build_path_cb, &ctx);

    return ctx.failed ? -1 : 0;
}

/* ============================================================
 *  등록
 * ============================================================ */

uint32_t core_register_file(archdb_t *db, const char *file_name, uint16_t version,
                             const char **tags, int tag_count)
{
    if (file_name == NULL || file_name[0] == '\0') return 0;
    if (tag_count < 1) return 0; /* 정책: 태그 없는 등록 금지 */

    uint32_t id = db_file_append(db, file_name, version);
    if (id == ARCHDB_INVALID_ID) return 0;

    for (int i = 0; i < tag_count; i++) {
        if (db_tag_add(db, id, tags[i]) != 0) {
            db_file_delete(db, id); /* 롤백 */
            return 0;
        }
    }
    return id;
}

/* ============================================================
 *  조회
 * ============================================================ */

typedef struct {
    uint32_t target_id;
    int      found;
} has_tag_ctx_t;

static int has_tag_cb(uint32_t file_id, const char tag[ARCHDB_TAG_LEN], void *ctx_v)
{
    (void)tag;
    has_tag_ctx_t *ctx = (has_tag_ctx_t *)ctx_v;
    if (file_id == ctx->target_id) {
        ctx->found = 1;
        return 1; /* 순회 중단 */
    }
    return 0;
}

int core_file_has_tag(archdb_t *db, uint32_t file_id, const char *tag)
{
    has_tag_ctx_t ctx = { file_id, 0 };
    db_tag_foreach_by_tag(db, tag, has_tag_cb, &ctx);
    return ctx.found;
}

typedef struct {
    uint32_t *out_ids;
    int       max;
    int       count;
} collect_ctx_t;

static int collect_cb(uint32_t file_id, const char tag[ARCHDB_TAG_LEN], void *ctx_v)
{
    (void)tag;
    collect_ctx_t *ctx = (collect_ctx_t *)ctx_v;
    if (ctx->count < ctx->max) ctx->out_ids[ctx->count++] = file_id;
    return 0;
}

int core_list_by_path(archdb_t *db, const char *path, uint32_t *out_ids, int max_ids)
{
    char tags[CORE_MAX_PATH_TAGS][ARCHDB_TAG_LEN + 1];
    int tag_count = core_path_split(path, tags, CORE_MAX_PATH_TAGS);
    if (tag_count == 0) return 0;

    /* 첫 태그로 후보 목록 수집 */
    collect_ctx_t ctx = { out_ids, max_ids, 0 };
    db_tag_foreach_by_tag(db, tags[0], collect_cb, &ctx);

    /* 나머지 태그로 AND 필터링 */
    int result_count = ctx.count;
    for (int t = 1; t < tag_count; t++) {
        int w = 0;
        for (int i = 0; i < result_count; i++) {
            if (core_file_has_tag(db, out_ids[i], tags[t])) {
                out_ids[w++] = out_ids[i];
            }
        }
        result_count = w;
    }
    return result_count;
}
