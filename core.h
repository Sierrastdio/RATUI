/*
 * core.h
 *
 * db.c 위에서 동작하는 의미론 계층.
 * "사용자 입력(파일명/버전/태그, 가상 경로 문자열)을 어떻게 db_* 호출로
 * 바꿀 것인가"를 담당한다. db.c는 이 파일을 전혀 몰라도 된다.
 *
 * 정책 (2026-07 확정):
 *  - 태그 없는 파일 등록은 금지 (최소 1개 필수)
 *  - 같은 파일명 재등록은 항상 새 file_id (dedup/버전업 정책은 나중에 결정)
 *  - 섹터(INS/ROS/EDS/BKS/TRS) 개념은 쓰지 않는다
 */

#ifndef ARCHDB_CORE_H
#define ARCHDB_CORE_H

#include "db.h"

#define CORE_MAX_PATH_TAGS   16   /* "A/B/C/..." 경로 한 번에 최대 태그 개수 */

/* ---------- 가상 경로 파싱 ---------- */

/* "DOCS/IMG" 같은 경로 문자열을 태그 배열로 쪼갠다.
 * tags_out[i] 는 최대 ARCHDB_TAG_LEN 글자 + NUL. 반환값: 쪼갠 태그 개수 (실패 시 0) */
int core_path_split(const char *path, char tags_out[][ARCHDB_TAG_LEN + 1], int max_tags);

/* file_id 에 붙은 모든 태그를 "TAG1/TAG2" 형태로 합쳐 buf 에 채운다. 성공 0 / 실패 -1 */
int core_file_path(archdb_t *db, uint32_t file_id, char *buf, size_t buf_len);

/* ---------- 등록 ---------- */

/* 파일 등록 + 태그 일괄 부여.
 * tag_count 는 1 이상이어야 한다 (정책: 태그 없는 등록 금지). 위반 시 0 반환.
 * 태그 등록 중 하나라도 실패하면 파일 레코드도 롤백(논리 삭제)하고 0 반환.
 * 성공 시 새로 부여된 file_id 반환. */
uint32_t core_register_file(archdb_t *db, const char *file_name, uint16_t version,
                             const char **tags, int tag_count);

/* ---------- 조회 ---------- */

/* file_id 가 tag 를 가지고 있는지 확인 */
int core_file_has_tag(archdb_t *db, uint32_t file_id, const char *tag);

/* 가상 경로("A/B")로 검색 - 경로에 있는 모든 태그를 동시에 가진 파일만 반환 (AND 교집합).
 * out_ids 에 최대 max_ids 개까지 채우고, 실제로 채운 개수를 반환. */
int core_list_by_path(archdb_t *db, const char *path, uint32_t *out_ids, int max_ids);

/* 태그를 배열로 직접 받아 검색 - core_list_by_path와 동작은 같지만
 * "A/B" 같은 경로 문자열을 거치지 않는다 (태그를 하나씩 입력받는 UI용). */
int core_list_by_tags(archdb_t *db, const char **tags, int tag_count, uint32_t *out_ids, int max_ids);

#endif /* ARCHDB_CORE_H */