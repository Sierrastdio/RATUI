/*
 * db.c
 *
 * db.h 에서 선언한 저수준 저장 함수들의 구현.
 * "바이트를 어디에 읽고 쓰는가"만 다루고, 의미론적 판단(가상 경로 조립,
 * 중복 파일명 처리 등)은 여기서 하지 않는다.
 */

#include "db.h"

#include <stdlib.h>
#include <string.h>

/* ============================================================
 *  내부 유틸
 * ============================================================ */

/* 파일이 없으면 빈 파일로 새로 만들고, 있으면 그냥 연다 (r+b) */
static FILE *open_or_create(const char *path)
{
    FILE *fp = fopen(path, "r+b");
    if (fp != NULL) return fp;

    fp = fopen(path, "w+b"); /* 없으니 새로 만든다 - 내용은 비어있음 */
    return fp;
}

static int join_path(char *out, size_t out_len, const char *dir, const char *name)
{
    int n = snprintf(out, out_len, "%s/%s", dir, name);
    return (n > 0 && (size_t)n < out_len) ? 0 : -1;
}

/* 정규화된 4글자 태그로 복사 (남는 자리는 공백, 소문자는 대문자로) */
void db_tag_normalize(char out[ARCHDB_TAG_LEN], const char *tag)
{
    memset(out, ' ', ARCHDB_TAG_LEN);
    for (int i = 0; i < ARCHDB_TAG_LEN && tag[i] != '\0'; i++) {
        char c = tag[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        out[i] = c;
    }
}

static size_t file_count(FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    if (size < 0) return 0;
    return (size_t)size / sizeof(file_record_t);
}

static size_t link_count(FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    if (size < 0) return 0;
    return (size_t)size / sizeof(file_tag_link_t);
}

static long file_offset(uint32_t file_id)
{
    return (long)(file_id - 1) * (long)sizeof(file_record_t);
}

static long link_offset(size_t index /* 0-based */)
{
    return (long)(index * sizeof(file_tag_link_t));
}

/* ============================================================
 *  라이프사이클
 * ============================================================ */

int archdb_open(archdb_t *db, const char *base_dir)
{
    if (db == NULL || base_dir == NULL) return -1;
    memset(db, 0, sizeof(*db));

    char path[512];

    if (join_path(path, sizeof(path), base_dir, "files.db") != 0) return -1;
    db->files_fp = open_or_create(path);
    if (db->files_fp == NULL) return -1;

    if (join_path(path, sizeof(path), base_dir, "file_tags.db") != 0) goto fail;
    db->file_tags_fp = open_or_create(path);
    if (db->file_tags_fp == NULL) goto fail;

    return 0;

fail:
    archdb_close(db);
    return -1;
}

void archdb_close(archdb_t *db)
{
    if (db == NULL) return;
    if (db->files_fp)     { fclose(db->files_fp);     db->files_fp = NULL; }
    if (db->file_tags_fp) { fclose(db->file_tags_fp); db->file_tags_fp = NULL; }
}

/* ============================================================
 *  files.db
 * ============================================================ */

uint32_t db_file_append(archdb_t *db, const char *file_name, uint16_t version)
{
    FILE *fp = db->files_fp;
    size_t count = file_count(fp);
    uint32_t new_id = (uint32_t)count + 1;

    file_record_t rec;
    memset(&rec, 0, sizeof(rec));
    rec.file_id = new_id;
    strncpy(rec.file_name, file_name, ARCHDB_NAME_LEN - 1);
    rec.version = version;

    if (fseek(fp, file_offset(new_id), SEEK_SET) != 0) return 0;
    if (fwrite(&rec, sizeof(rec), 1, fp) != 1) return 0;
    fflush(fp);

    return new_id;
}

int db_file_read(archdb_t *db, uint32_t file_id, file_record_t *out)
{
    if (file_id == ARCHDB_INVALID_ID) return -1;
    FILE *fp = db->files_fp;

    if (fseek(fp, file_offset(file_id), SEEK_SET) != 0) return -1;
    if (fread(out, sizeof(*out), 1, fp) != 1) return -1;
    if (out->file_id != file_id) return -1; /* 삭제되었거나 범위 밖 */
    return 0;
}

int db_file_update(archdb_t *db, uint32_t file_id, const char *file_name, uint16_t version)
{
    file_record_t rec;
    if (db_file_read(db, file_id, &rec) != 0) return -1;

    memset(rec.file_name, 0, ARCHDB_NAME_LEN);
    strncpy(rec.file_name, file_name, ARCHDB_NAME_LEN - 1);
    rec.version = version;

    FILE *fp = db->files_fp;
    if (fseek(fp, file_offset(file_id), SEEK_SET) != 0) return -1;
    if (fwrite(&rec, sizeof(rec), 1, fp) != 1) return -1;
    fflush(fp);
    return 0;
}

int db_file_delete(archdb_t *db, uint32_t file_id)
{
    file_record_t rec;
    if (db_file_read(db, file_id, &rec) != 0) return -1;
    rec.file_id = 0; /* 논리 삭제 */

    FILE *fp = db->files_fp;
    if (fseek(fp, file_offset(file_id), SEEK_SET) != 0) return -1;
    if (fwrite(&rec, sizeof(rec), 1, fp) != 1) return -1;
    fflush(fp);
    return 0;
}

void db_file_foreach(archdb_t *db, file_visit_cb cb, void *ctx)
{
    FILE *fp = db->files_fp;
    size_t count = file_count(fp);

    if (fseek(fp, 0, SEEK_SET) != 0) return;

    for (size_t i = 0; i < count; i++) {
        file_record_t rec;
        if (fread(&rec, sizeof(rec), 1, fp) != 1) break;
        if (rec.file_id == 0) continue; /* 삭제된 슬롯 skip */
        if (cb(&rec, ctx) != 0) return;
    }
}

/* ============================================================
 *  file_tags.db (다대다 매핑)
 * ============================================================ */

int db_tag_add(archdb_t *db, uint32_t file_id, const char *tag)
{
    FILE *fp = db->file_tags_fp;
    char norm_tag[ARCHDB_TAG_LEN];
    db_tag_normalize(norm_tag, tag);

    size_t count = link_count(fp);

    /* 중복 체크 */
    if (fseek(fp, 0, SEEK_SET) != 0) return -1;
    for (size_t i = 0; i < count; i++) {
        file_tag_link_t link;
        if (fread(&link, sizeof(link), 1, fp) != 1) break;
        if (link.file_id == file_id && memcmp(link.tag, norm_tag, ARCHDB_TAG_LEN) == 0) {
            return 0; /* 이미 있음 */
        }
    }

    /* append */
    file_tag_link_t new_link;
    new_link.file_id = file_id;
    memcpy(new_link.tag, norm_tag, ARCHDB_TAG_LEN);

    if (fseek(fp, link_offset(count), SEEK_SET) != 0) return -1;
    if (fwrite(&new_link, sizeof(new_link), 1, fp) != 1) return -1;
    fflush(fp);
    return 0;
}

int db_tag_remove(archdb_t *db, uint32_t file_id, const char *tag)
{
    FILE *fp = db->file_tags_fp;
    char norm_tag[ARCHDB_TAG_LEN];
    db_tag_normalize(norm_tag, tag);

    size_t count = link_count(fp);

    for (size_t i = 0; i < count; i++) {
        long off = link_offset(i);
        file_tag_link_t link;
        if (fseek(fp, off, SEEK_SET) != 0) return -1;
        if (fread(&link, sizeof(link), 1, fp) != 1) break;

        if (link.file_id == file_id && memcmp(link.tag, norm_tag, ARCHDB_TAG_LEN) == 0) {
            link.file_id = 0; /* 논리 삭제 */
            if (fseek(fp, off, SEEK_SET) != 0) return -1;
            if (fwrite(&link, sizeof(link), 1, fp) != 1) return -1;
            fflush(fp);
            return 0;
        }
    }
    return -1; /* 못 찾음 */
}

void db_tag_foreach_by_file(archdb_t *db, uint32_t file_id, link_visit_cb cb, void *ctx)
{
    FILE *fp = db->file_tags_fp;
    size_t count = link_count(fp);

    if (fseek(fp, 0, SEEK_SET) != 0) return;

    for (size_t i = 0; i < count; i++) {
        file_tag_link_t link;
        if (fread(&link, sizeof(link), 1, fp) != 1) break;
        if (link.file_id != file_id) continue; /* 0(삭제됨)도 자동으로 걸러짐 */
        if (cb(link.file_id, link.tag, ctx) != 0) return;
    }
}

void db_tag_foreach_by_tag(archdb_t *db, const char *tag, link_visit_cb cb, void *ctx)
{
    FILE *fp = db->file_tags_fp;
    char norm_tag[ARCHDB_TAG_LEN];
    db_tag_normalize(norm_tag, tag);

    size_t count = link_count(fp);
    if (fseek(fp, 0, SEEK_SET) != 0) return;

    for (size_t i = 0; i < count; i++) {
        file_tag_link_t link;
        if (fread(&link, sizeof(link), 1, fp) != 1) break;
        if (link.file_id == 0) continue;
        if (memcmp(link.tag, norm_tag, ARCHDB_TAG_LEN) != 0) continue;
        if (cb(link.file_id, link.tag, ctx) != 0) return;
    }
}

void db_tag_link_foreach_all(archdb_t *db, link_visit_cb cb, void *ctx)
{
    FILE *fp = db->file_tags_fp;
    size_t count = link_count(fp);
    if (fseek(fp, 0, SEEK_SET) != 0) return;

    for (size_t i = 0; i < count; i++) {
        file_tag_link_t link;
        if (fread(&link, sizeof(link), 1, fp) != 1) break;
        if (link.file_id == 0) continue; /* 삭제된 슬롯 skip */
        if (cb(link.file_id, link.tag, ctx) != 0) return;
    }
}