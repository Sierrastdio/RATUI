#ifndef FILE_CHECK_H
#define FILE_CHECK_H

#include <ncurses.h>



int FILE_EXISTENCE_CHECK(const char *path);

long FILE_SIZE_GET(const char *path);\

int FILE_DUPLICATE_CHECK (const char *src_path, const char *dest_path);
/* 3단계 중복 검사 (0: 중복 아님, 1: 중복 확정)
    1차: 파일 크기 비교
    2차: 앞/뒤 4KB 데이터 비교
    3차: 전체 MD5/SHA 해시 대조 */

#endif