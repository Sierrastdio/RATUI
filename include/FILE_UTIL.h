#ifndef FILE_UTIL_H
#define FILE_UTIL_H

// src 파일을 dest 경로로 복사합니다. (성공 시 1, 실패 시 0)
int FILE_COPY(const char *src, const char *dest);

// src 파일을 dest 경로로 이동시킵니다. (성공 시 1, 실패 시 0)
int FILE_MOVE(const char *src, const char *dest);

#endif