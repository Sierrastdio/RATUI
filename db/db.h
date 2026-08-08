/*
 * db.h
 *
 * 파일 아카이브 DB 저수준 계층.
 * 물리 파일 2개만 사용한다.
 *
 *   files.db      - file_record_t 배열 (고정 길이, file_id 로 offset 직접 계산)
 *   file_tags.db  - file_tag_link_t 배열 (고정 길이, 순차 스캔 / append + 논리삭제)
 *
 * 태그는 이미 영문 4글자 고정 코드이므로 별도의 이름 테이블(strings.db 등)이
 * 필요 없다. 헤더도 두지 않는다 - 레코드 개수는 그때그때 파일 크기로 계산한다.
 */

#ifndef ARCHDB_DB_H
#define ARCHDB_DB_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

#define ARCHDB_TAG_LEN      4
#define ARCHDB_NAME_LEN     128
#define ARCHDB_INVALID_ID   0u   /* file_id 0 은 "없음/삭제됨" */

#pragma pack(push, 1)

/* files.db 레코드. file_id = 배열 인덱스 + 1 로 직접 offset 계산 가능 */
typedef struct {
    uint32_t file_id;                  /* 1부터 시작. 0이면 빈(삭제된) 슬롯 */
    char     file_name[ARCHDB_NAME_LEN];
    uint16_t version;
    uint64_t content_hash;             /* 파일 내용 앞4KB+뒤4KB+크기 기반 지문. 0=계산 안 됨 */
} file_record_t;

/* file_tags.db 레코드. (file_id, tag) 매핑 - 파일 하나에 태그 여러 개 가능 */
typedef struct {
    uint32_t file_id;                  /* 0이면 삭제된 슬롯 */
    char     tag[ARCHDB_TAG_LEN];      /* 4글자, 남는 자리는 공백(' ')으로 채움 */
} file_tag_link_t;

#pragma pack(pop)

/* ---------- 핸들 ---------- */

typedef struct {
    FILE *files_fp;
    FILE *file_tags_fp;
} archdb_t;

/* ---------- 라이프사이클 ---------- */

/* base_dir 아래 files.db / file_tags.db 를 열거나(없으면) 새로 만든다. 성공 0 / 실패 -1 */
int  archdb_open(archdb_t *db, const char *base_dir);
void archdb_close(archdb_t *db);

/* ---------- files.db ---------- */

/* 새 파일 레코드를 append 하고 부여된 file_id 를 반환한다. 실패 시 0 */
uint32_t db_file_append(archdb_t *db, const char *file_name, uint16_t version, uint64_t content_hash);

/* file_id 로 레코드를 읽는다. 성공 0 / 없음·실패 -1 */
int db_file_read(archdb_t *db, uint32_t file_id, file_record_t *out);

/* 파일명/버전/내용해시를 갱신한다 (버전업 등). 성공 0 / 실패 -1 */
int db_file_update(archdb_t *db, uint32_t file_id, const char *file_name, uint16_t version, uint64_t content_hash);

/* file_id 슬롯을 논리 삭제한다. 성공 0 / 실패 -1 */
int db_file_delete(archdb_t *db, uint32_t file_id);

/* files.db 전체 순회. 콜백이 0이 아닌 값을 반환하면 순회 중단. 삭제된 슬롯은 건너뜀 */
typedef int (*file_visit_cb)(const file_record_t *rec, void *ctx);
void db_file_foreach(archdb_t *db, file_visit_cb cb, void *ctx);

/* ---------- file_tags.db (다대다 매핑) ---------- */

/* (file_id, tag) 매핑 추가. tag 는 1~4글자 영문, 이미 있으면 아무 것도 안 하고 0 반환 */
int db_tag_add(archdb_t *db, uint32_t file_id, const char *tag);

/* (file_id, tag) 매핑 제거 (논리 삭제). 성공 0 / 못 찾음 -1 */
int db_tag_remove(archdb_t *db, uint32_t file_id, const char *tag);

/* 특정 file_id 에 붙은 모든 태그를 순회 (가상 경로 조립용) */
typedef int (*link_visit_cb)(uint32_t file_id, const char tag[ARCHDB_TAG_LEN], void *ctx);
void db_tag_foreach_by_file(archdb_t *db, uint32_t file_id, link_visit_cb cb, void *ctx);

/* 특정 태그가 붙은 모든 file_id 를 순회 (태그로 검색 = 가상 경로 진입) */
void db_tag_foreach_by_tag(archdb_t *db, const char *tag, link_visit_cb cb, void *ctx);

/* file_tags.db 전체를 순회 (삭제된 슬롯 제외). 존재하는 모든 태그를 열거할 때 사용 */
void db_tag_link_foreach_all(archdb_t *db, link_visit_cb cb, void *ctx);

/* 태그 문자열을 4바이트 정규화 형태로 변환 (대문자화 + 공백 패딩). 태그 비교에 사용 */
void db_tag_normalize(char out[ARCHDB_TAG_LEN], const char *tag);

#endif /* ARCHDB_DB_H */