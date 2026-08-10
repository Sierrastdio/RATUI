/*
 * core.c
 *
 * db_* 저수준 함수들을 조합해 "가상 경로" 개념을 구현한다.
 */

#include "core.h"

#include <string.h>
#include <stdio.h>

/* ============================================================
 *  경로 파싱
 * ============================================================ */

int CORE_PATH_SPLIT(const char *path, char tags_out[][ARCHDB_TAG_LEN + 1], int max_tags)
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

int CORE_FILE_PATH(archdb_t *db, uint32_t file_id, char *buf, size_t buf_len)
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

uint32_t CORE_REGISTER_FILE(archdb_t *db, const char *file_name, uint16_t version,
                             uint64_t content_hash, const char **tags, int tag_count)
{
    if (file_name == NULL || file_name[0] == '\0') return 0;
    if (tag_count < 1) return 0; /* 정책: 태그 없는 등록 금지 */

    uint32_t id = db_file_append(db, file_name, version, content_hash);
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

int CORE_FILE_HAS_TAG(archdb_t *db, uint32_t file_id, const char *tag)
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

int CORE_LIST_BY_TAGS(archdb_t *db, const char **tags, int tag_count, uint32_t *out_ids, int max_ids)
{
    if (tag_count <= 0) return 0;

    /* 첫 태그로 후보 목록 수집 */
    collect_ctx_t ctx = { out_ids, max_ids, 0 };
    db_tag_foreach_by_tag(db, tags[0], collect_cb, &ctx);

    /* 나머지 태그로 AND 필터링 */
    int result_count = ctx.count;
    for (int t = 1; t < tag_count; t++) {
        int w = 0;
        for (int i = 0; i < result_count; i++) {
            if (CORE_FILE_HAS_TAG(db, out_ids[i], tags[t])) {
                out_ids[w++] = out_ids[i];
            }
        }
        result_count = w;
    }
    return result_count;
}

int CORE_LIST_BY_PATH(archdb_t *db, const char *path, uint32_t *out_ids, int max_ids)
{
    char tags_buf[CORE_MAX_PATH_TAGS][ARCHDB_TAG_LEN + 1];
    int tag_count = CORE_PATH_SPLIT(path, tags_buf, CORE_MAX_PATH_TAGS);
    if (tag_count == 0) return 0;

    const char *tag_ptrs[CORE_MAX_PATH_TAGS];
    for (int i = 0; i < tag_count; i++) tag_ptrs[i] = tags_buf[i];

    return CORE_LIST_BY_TAGS(db, tag_ptrs, tag_count, out_ids, max_ids);
}

/* ============================================================
 *  폴더뷰(파일 탐색기 스타일 브라우징)
 * ============================================================ */

int CORE_LIST_FILES_IN_VIEW(archdb_t *db, const char **selected_tags, int selected_count,
                             uint32_t *out_ids, int max_ids)
{
    if (selected_count <= 0) return 0; /* 정책: 태그 없는 파일은 없으므로 최상위엔 파일 없음 */
    return CORE_LIST_BY_TAGS(db, selected_tags, selected_count, out_ids, max_ids);
}

/* out_tags 안에서 트림된 태그 문자열로 이미 등록돼 있는 인덱스를 찾는다. 없으면 -1 */
static int find_child_tag_index(core_child_tag_t *out_tags, int count, const char *trimmed)
{
    for (int i = 0; i < count; i++) {
        if (strcmp(out_tags[i].tag, trimmed) == 0) return i;
    }
    return -1;
}

/* 4바이트 태그(공백 패딩)를 NUL 종료 트림 문자열로 변환 */
static void trim_tag(char out[ARCHDB_TAG_LEN + 1], const char tag[ARCHDB_TAG_LEN])
{
    memcpy(out, tag, ARCHDB_TAG_LEN);
    out[ARCHDB_TAG_LEN] = '\0';
    for (int i = ARCHDB_TAG_LEN - 1; i >= 0 && out[i] == ' '; i--) out[i] = '\0';
}

/* selected_tags 안에 이미 있는 태그인지 확인 (정규화 비교) */
static int tag_is_selected(const char tag[ARCHDB_TAG_LEN], const char **selected_tags, int selected_count)
{
    for (int i = 0; i < selected_count; i++) {
        char norm[ARCHDB_TAG_LEN];
        db_tag_normalize(norm, selected_tags[i]);
        if (memcmp(tag, norm, ARCHDB_TAG_LEN) == 0) return 1;
    }
    return 0;
}

/* ---- 최상위(selected_count==0) 케이스: DB 전체 태그 열거 ---- */

typedef struct {
    core_child_tag_t *out;
    int max;
    int count;
} all_tags_ctx_t;

static int all_tags_cb(uint32_t file_id, const char tag[ARCHDB_TAG_LEN], void *ctx_v)
{
    (void)file_id;
    all_tags_ctx_t *ctx = (all_tags_ctx_t *)ctx_v;

    char trimmed[ARCHDB_TAG_LEN + 1];
    trim_tag(trimmed, tag);

    int idx = find_child_tag_index(ctx->out, ctx->count, trimmed);
    if (idx >= 0) {
        ctx->out[idx].file_count++;
        return 0;
    }
    if (ctx->count < ctx->max) {
        strcpy(ctx->out[ctx->count].tag, trimmed);
        ctx->out[ctx->count].file_count = 1;
        ctx->count++;
    }
    return 0;
}

/* ---- 하위 레벨(selected_count>0) 케이스: 현재 결과 파일들의 나머지 태그 열거 ---- */

typedef struct {
    core_child_tag_t *out;
    int max;
    int count;
    const char **exclude;
    int exclude_count;
} child_tags_ctx_t;

static int child_tags_cb(uint32_t file_id, const char tag[ARCHDB_TAG_LEN], void *ctx_v)
{
    (void)file_id;
    child_tags_ctx_t *ctx = (child_tags_ctx_t *)ctx_v;

    if (tag_is_selected(tag, ctx->exclude, ctx->exclude_count)) return 0; /* 이미 선택된 태그는 제외 */

    char trimmed[ARCHDB_TAG_LEN + 1];
    trim_tag(trimmed, tag);

    int idx = find_child_tag_index(ctx->out, ctx->count, trimmed);
    if (idx >= 0) {
        ctx->out[idx].file_count++;
        return 0;
    }
    if (ctx->count < ctx->max) {
        strcpy(ctx->out[ctx->count].tag, trimmed);
        ctx->out[ctx->count].file_count = 1;
        ctx->count++;
    }
    return 0;
}

int CORE_LIST_CHILD_TAGS(archdb_t *db, const char **selected_tags, int selected_count,
                          core_child_tag_t *out_tags, int max_tags)
{
    if (selected_count <= 0) {
        all_tags_ctx_t ctx = { out_tags, max_tags, 0 };
        db_tag_link_foreach_all(db, all_tags_cb, &ctx);
        return ctx.count;
    }

    /* 현재 선택된 태그를 모두 만족하는 파일들을 찾고, 그 파일들이 가진 다른 태그를 모은다 */
    uint32_t file_ids[256];
    int file_n = CORE_LIST_BY_TAGS(db, selected_tags, selected_count, file_ids, 256);

    child_tags_ctx_t ctx = { out_tags, max_tags, 0, selected_tags, selected_count };
    for (int i = 0; i < file_n; i++) {
        db_tag_foreach_by_file(db, file_ids[i], child_tags_cb, &ctx);
    }
    return ctx.count;
}

/* ============================================================
 *  등록 해제
 * ============================================================ */

typedef struct {
    char tags[CORE_MAX_PATH_TAGS][ARCHDB_TAG_LEN + 1];
    int  count;
} all_file_tags_ctx_t;

static int collect_all_tags_cb(uint32_t file_id, const char tag[ARCHDB_TAG_LEN], void *ctx_v)
{
    (void)file_id;
    all_file_tags_ctx_t *ctx = (all_file_tags_ctx_t *)ctx_v;
    if (ctx->count < CORE_MAX_PATH_TAGS) {
        trim_tag(ctx->tags[ctx->count], tag);
        ctx->count++;
    }
    return 0;
}

int CORE_DELETE_FILE(archdb_t *db, uint32_t file_id)
{
    if (db == NULL || file_id == ARCHDB_INVALID_ID) return -1;

    all_file_tags_ctx_t all_tags = { .count = 0 };
    db_tag_foreach_by_file(db, file_id, collect_all_tags_cb, &all_tags);

    for (int i = 0; i < all_tags.count; i++) {
        db_tag_remove(db, file_id, all_tags.tags[i]);
    }

    return db_file_delete(db, file_id);
}

uint32_t CORE_UNREGISTER_FILE(archdb_t *db, const char *file_name, uint16_t version,
                               const char **tags, int tag_count)
{
    if (file_name == NULL || file_name[0] == '\0') return 0;
    if (tag_count < 1) return 0; /* 후보를 좁히려면 태그가 최소 1개 필요 */

    /* 주어진 태그를 모두 가진 파일들 중에서 이름+버전이 일치하는 것만 후보로 좁힌다 */
    uint32_t candidates[64];
    int n = CORE_LIST_BY_TAGS(db, tags, tag_count, candidates, 64);

    uint32_t target = ARCHDB_INVALID_ID;
    int match_count = 0;

    for (int i = 0; i < n; i++) {
        file_record_t rec;
        if (db_file_read(db, candidates[i], &rec) != 0) continue;
        if (strcmp(rec.file_name, file_name) == 0 && rec.version == version) {
            target = candidates[i];
            match_count++;
        }
    }

    /* 정확히 1개로 특정되지 않으면(못 찾았거나, 조건이 겹쳐서 여러 개 걸리면) 안전하게 취소 */
    if (match_count != 1) return 0;

    if (CORE_DELETE_FILE(db, target) != 0) return 0;

    return target;
}

/* ============================================================
 *  태그 하나만 떼어내기 (파일은 유지)
 * ============================================================ */

typedef struct { int count; } count_tags_ctx_t;

static int count_tags_cb(uint32_t file_id, const char tag[ARCHDB_TAG_LEN], void *ctx_v)
{
    (void)file_id;
    (void)tag;
    count_tags_ctx_t *ctx = (count_tags_ctx_t *)ctx_v;
    ctx->count++;
    return 0;
}

int CORE_UNTAG_FILE(archdb_t *db, uint32_t file_id, const char *tag)
{
    if (db == NULL || file_id == ARCHDB_INVALID_ID || tag == NULL) return -1;

    count_tags_ctx_t cnt = { 0 };
    db_tag_foreach_by_file(db, file_id, count_tags_cb, &cnt);

    if (cnt.count <= 1) return -1; /* 마지막 남은 태그 - 정책상 거부 */

    return db_tag_remove(db, file_id, tag);
}

/* ============================================================
 *  내용 기반 중복 검사
 * ============================================================ */

uint64_t CORE_HASH_FILE(const char *full_path)
{
    if (full_path == NULL) return 0;

    FILE *fp = fopen(full_path, "rb");
    if (fp == NULL) return 0;

    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return 0; }
    long size = ftell(fp);
    if (size < 0) { fclose(fp); return 0; }

    /* FNV-1a 64비트 해시 */
    uint64_t hash = 14695981039346656037ULL;
    const uint64_t prime = 1099511628211ULL;
    unsigned char buf[4096];

    /* 앞 4KB */
    if (fseek(fp, 0, SEEK_SET) == 0) {
        size_t to_read = (size < (long)sizeof(buf)) ? (size_t)size : sizeof(buf);
        size_t got = fread(buf, 1, to_read, fp);
        for (size_t i = 0; i < got; i++) { hash ^= buf[i]; hash *= prime; }
    }

    /* 뒤 4KB (파일이 8KB보다 커서 앞부분과 안 겹칠 때만) */
    if (size > (long)(sizeof(buf) * 2)) {
        long tail_off = size - (long)sizeof(buf);
        if (fseek(fp, tail_off, SEEK_SET) == 0) {
            size_t got = fread(buf, 1, sizeof(buf), fp);
            for (size_t i = 0; i < got; i++) { hash ^= buf[i]; hash *= prime; }
        }
    }

    /* 파일 크기도 섞어서, 앞/뒤 4KB가 우연히 같아도 크기가 다르면 다른 해시가 되게 */
    unsigned char size_bytes[sizeof(long)];
    memcpy(size_bytes, &size, sizeof(size));
    for (size_t i = 0; i < sizeof(size_bytes); i++) { hash ^= size_bytes[i]; hash *= prime; }

    fclose(fp);

    return (hash == 0) ? 1 : hash; /* 0은 "계산 안 됨" sentinel이라 실제 결과가 0이면 1로 치환 */
}

typedef struct {
    const char *file_name;
    uint64_t hash;
    core_dup_result_t result;
} dup_check_ctx_t;

static int dup_check_cb(const file_record_t *rec, void *ctx_v)
{
    dup_check_ctx_t *ctx = (dup_check_ctx_t *)ctx_v;

    int same_name = (strcmp(rec->file_name, ctx->file_name) == 0);
    int same_content = (rec->content_hash != 0 && rec->content_hash == ctx->hash);

    if (same_name && same_content) {
        ctx->result.kind = CORE_DUP_SAME_NAME_SAME_CONTENT;
        ctx->result.existing_file_id = rec->file_id;
        return 1; /* 가장 강한 매치 - 바로 순회 중단 */
    }

    if (!same_name && same_content && ctx->result.kind == CORE_DUP_NONE) {
        /* 더 약한 매치라 계속 찾아보되(더 강한 매치가 있으면 그걸 우선하려고),
         * 아직 아무 것도 못 찾았으면 일단 기록해둔다 */
        ctx->result.kind = CORE_DUP_DIFF_NAME_SAME_CONTENT;
        ctx->result.existing_file_id = rec->file_id;
    }

    /* same_name && !same_content -> 이름은 같은데 내용이 다름: 통과 대상, 아무 것도 안 함 */
    return 0;
}

core_dup_result_t CORE_CHECK_DUP_CONTENT(archdb_t *db, uint64_t content_hash, const char *file_name)
{
    core_dup_result_t result = { CORE_DUP_NONE, ARCHDB_INVALID_ID };
    if (content_hash == 0 || file_name == NULL) return result; /* 해시 계산 실패 - 검사 스킵 */

    dup_check_ctx_t ctx = { file_name, content_hash, result };
    db_file_foreach(db, dup_check_cb, &ctx);

    return ctx.result;
}