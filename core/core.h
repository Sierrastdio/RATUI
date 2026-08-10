/*
 * core.h
 *
 * db.c 위에서 동작하는 의미론 계층.
 * "사용자 입력(파일명/버전/태그, 가상 경로 문자열)을 어떻게 db_* 호출로
 * 바꿀 것인가"를 담당한다. db.c는 이 파일을 전혀 몰라도 된다.
 *
 */

#ifndef ARCHDB_CORE_H
#define ARCHDB_CORE_H

#include "db.h"

#define CORE_MAX_PATH_TAGS   16   /* "A/B/C/..." 경로 한 번에 최대 태그 개수 */
#define CORE_MAX_CHILD_TAGS  64   /* 폴더뷰 한 화면에 보여줄 최대 하위 태그 개수 */

/* ---------- 가상 경로 파싱 ---------- */

/* "DOCS/IMG" 같은 경로 문자열을 태그 배열로 쪼갠다.
 * tags_out[i] 는 최대 ARCHDB_TAG_LEN 글자 + NUL. 반환값: 쪼갠 태그 개수 (실패 시 0) */
int CORE_PATH_SPLIT(const char *path, char tags_out[][ARCHDB_TAG_LEN + 1], int max_tags);


/* file_id 에 붙은 모든 태그를 "TAG1/TAG2" 형태로 합쳐 buf 에 채운다. 성공 0 / 실패 -1 */
int CORE_FILE_PATH(archdb_t *db, uint32_t file_id, char *buf, size_t buf_len);

/* ---------- 등록 ---------- */

/* 파일 등록 + 태그 일괄 부여.
 * content_hash 는 CORE_HASH_FILE()로 미리 계산해서 넘긴다 (모르면 0을 넘겨도 되지만,
 * 그러면 이 레코드는 이후 내용 기반 중복검사에서 항상 "다른 파일"로 취급된다).
 * tag_count 는 1 이상이어야 한다 (정책: 태그 없는 등록 금지). 위반 시 0 반환.
 * 태그 등록 중 하나라도 실패하면 파일 레코드도 롤백(논리 삭제)하고 0 반환.
 * 성공 시 새로 부여된 file_id 반환. */
uint32_t CORE_REGISTER_FILE(archdb_t *db, const char *file_name, uint16_t version,
                             uint64_t content_hash, const char **tags, int tag_count);

/* 등록 해제: file_name + version + tags 로 파일을 정확히 하나로 특정한 뒤,
 * 그 파일에 걸린 모든 태그 매핑을 제거하고 파일 레코드 자체도 삭제한다.
 * (같은 이름의 파일이 여러 개 있을 수 있는 정책이라, 조건에 맞는 파일이
 *  0개이거나 2개 이상이면 안전하게 아무 것도 안 하고 0을 반환한다.)
 * tag_count 는 1 이상이어야 한다 (최소 1개는 있어야 후보를 좁힐 수 있음).
 * 성공 시 삭제된 file_id 반환, 실패 시 0. */
uint32_t CORE_UNREGISTER_FILE(archdb_t *db, const char *file_name, uint16_t version,
                               const char **tags, int tag_count);

/* 파일은 그대로 두고 태그 하나만 떼어낸다 (파일 삭제 아님).
 * 정책: 그 태그가 파일에 남은 마지막 하나뿐이면 거부한다(-1) - 태그 없는 파일은
 * 안 되므로, 마지막 태그를 지우고 싶으면 CORE_UNREGISTER_FILE로 파일째 삭제해야 함.
 * 성공 0 / 실패(대상 없음, 마지막 태그, file_id 무효 등) -1 */
int CORE_UNTAG_FILE(archdb_t *db, uint32_t file_id, const char *tag);

/* file_id를 이미 알고 있을 때 그 파일을 완전히 삭제한다
 * (걸린 태그 전부 해제 + 파일 레코드 자체도 삭제).
 * CORE_UNREGISTER_FILE은 이름/버전/태그로 파일을 "찾아야" 할 때 쓰고,
 * file_id를 이미 알고 있으면(예: 목록에서 직접 선택) 이걸 바로 쓰면 된다.
 * 성공 0 / 실패 -1 */
int CORE_DELETE_FILE(archdb_t *db, uint32_t file_id);

/* 실제 파일(full_path)의 앞 4KB + 뒤 4KB + 파일 크기를 섞어서 지문 해시를 계산한다.
 * 파일을 열 수 없으면 0을 반환한다 (0은 "계산 안 됨"을 뜻하는 sentinel). */
uint64_t CORE_HASH_FILE(const char *full_path);

typedef enum {
    CORE_DUP_NONE = 0,                /* 중복 아님 - 새로 등록해도 됨 */
    CORE_DUP_SAME_NAME_SAME_CONTENT,  /* 이름도 내용도 같음 - 완전 중복 */
    CORE_DUP_DIFF_NAME_SAME_CONTENT,  /* 이름은 다른데 내용이 같음 (같은 파일을 다른 이름으로) */
} core_dup_kind_t;

typedef struct {
    core_dup_kind_t kind;
    uint32_t existing_file_id; /* kind != CORE_DUP_NONE 일 때만 유효 */
} core_dup_result_t;

/* content_hash(= CORE_HASH_FILE 결과)와 file_name으로, 이미 등록된 파일 중
 * 이름 또는 내용이 겹치는 게 있는지 확인한다. 이름은 같은데 내용이 다른 경우는
 * (버전이 바뀐 파일 등) 중복으로 안 치고 CORE_DUP_NONE을 반환한다.
 * content_hash가 0(계산 실패)이면 검사를 건너뛰고 항상 CORE_DUP_NONE. */
core_dup_result_t CORE_CHECK_DUP_CONTENT(archdb_t *db, uint64_t content_hash, const char *file_name);

/* ---------- 조회 ---------- */

/* file_id 가 tag 를 가지고 있는지 확인 */
int CORE_FILE_HAS_TAG(archdb_t *db, uint32_t file_id, const char *tag);

/* 가상 경로("A/B")로 검색 - 경로에 있는 모든 태그를 동시에 가진 파일만 반환 (AND 교집합).
 * out_ids 에 최대 max_ids 개까지 채우고, 실제로 채운 개수를 반환. */
int CORE_LIST_BY_PATH(archdb_t *db, const char *path, uint32_t *out_ids, int max_ids);

/* 태그를 배열로 직접 받아 검색 - CORE_LIST_BY_PATH와 동작은 같지만
 * "A/B" 같은 경로 문자열을 거치지 않는다 (태그를 하나씩 입력받는 UI용). */
int CORE_LIST_BY_TAGS(archdb_t *db, const char **tags, int tag_count, uint32_t *out_ids, int max_ids);

/* ---------- 폴더뷰(파일 탐색기 스타일 브라우징) ---------- */

typedef struct {
    char tag[ARCHDB_TAG_LEN + 1]; /* 트림된 태그 문자열 */
    int  file_count;              /* 현재 선택된 태그(selected_tags)를 만족하면서 이 태그도 가진 파일 수 */
} core_child_tag_t;

/* 현재 선택된 태그(selected_tags, "폴더 경로") 기준으로, 그 조건을 만족하는 파일들이
 * 추가로 가진 태그 목록을 "하위 폴더"처럼 반환한다. selected_tags 자신은 제외된다.
 * selected_count == 0 이면 DB 전체에 존재하는 모든 태그를 최상위 폴더로 반환한다.
 * out_tags 에 최대 max_tags 개까지 채우고, 실제로 채운 개수를 반환. */
int CORE_LIST_CHILD_TAGS(archdb_t *db, const char **selected_tags, int selected_count,
                          core_child_tag_t *out_tags, int max_tags);

/* 현재 선택된 태그(폴더 경로)를 모두 만족하는 파일 목록 ("현재 폴더 안의 파일들").
 * selected_count == 0 이면 0을 반환한다 (정책상 태그 없는 파일은 없으므로 최상위엔 파일이 없음). */
int CORE_LIST_FILES_IN_VIEW(archdb_t *db, const char **selected_tags, int selected_count,
                             uint32_t *out_ids, int max_ids);

#endif /* ARCHDB_CORE_H */