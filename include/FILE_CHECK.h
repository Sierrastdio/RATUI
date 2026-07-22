#ifndef FILE_CHECK_H
#define FILE_CHECK_H

#include <sys/stat.h>
#include <unistd.h>

#define FILE_EXISTENCE_CHECK(path) ((access((path), F_OK) == 0) ? 1 : 0)

#define FILE_SIZE_GET(path) ({ \
    struct stat ratui_file_size_st; \
    (stat((path), &ratui_file_size_st) == 0) ? (long)ratui_file_size_st.st_size : -1L; \
})

int FILE_DUPLICATE_CHECK (const char *src_path, const char *dest_path);
/* 3단계 중복 검사 (0: 중복 아님, 1: 중복 확정)
    1차: 파일 크기 비교
    2차: 앞/뒤 4KB 데이터 비교
    3차: 전체 MD5/SHA 해시 대조 */

#endif
