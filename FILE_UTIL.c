#include <stdio.h>
#include <unistd.h>
#include "FILE_UTIL.h"

// [복사 로직] 바이트 단위로 읽어서 대상 파일에 씁니다.
int FILE_COPY(const char *src, const char *dest) {
    FILE *source = fopen(src, "rb");
    FILE *target = fopen(dest, "wb");

    if (!source || !target) {
        if (source) fclose(source);
        if (target) fclose(target);
        return 0;
    }

    char buffer[16384]; // 16KB 버퍼로 성능 최적화
    size_t bytes;

    while ((bytes = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        if (fwrite(buffer, 1, bytes, target) != bytes) {
            fclose(source);
            fclose(target);
            return 0; // 쓰기 오류 발생 시
        }
    }

    fclose(source);
    fclose(target);
    return 1;
}

// [이동 로직] 복사 성공 후 원본 파일을 삭제합니다.
int FILE_MOVE(const char *src, const char *dest) {
    // 1. 먼저 복사를 시도합니다.
    if (FILE_COPY(src, dest)) {
        // 2. 복사가 완벽히 끝났다면 원본을 지웁니다.
        if (unlink(src) == 0) {
            return 1; // 이동 성공
        }
    }
    return 0; // 이동 실패
}