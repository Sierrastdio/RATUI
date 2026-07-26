#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "PATH_CONFIG.h"

// 전역 포인터 변수 초기화 (기초값 NULL)
const char *ARCHIVE_HOME_PATH = NULL;

// 안전한 포인터 할당 헬퍼 함수 (기존 메모리 해제 후 할당)
static void SET_PATH_VAL(const char **target, const char *val) {
    if (*target) {
        free((void *)*target);
    }
    // 좌측 따옴표 나 공백 스킵
    while (*val == ' ' || *val == '"') {
        val++;
    }
    *target = strdup(val);
}

// 설정 파일 읽기
void LOAD_CONFIG(void) {
    FILE *fp = fopen(Config_FileName, "r");
    if (!fp) {
        printf("Error: Could not open %s\n", Config_FileName);
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        STRIP_NEWLINE(line);

        // 주석 및 빈 줄 스킵
        if (line[0] == '#' || line[0] == '\0') continue;

        if (strncmp(line, "HOME_PATH=", 10) == 0) {
            SET_PATH_VAL(&ARCHIVE_HOME_PATH, line + 10);
        }
    }
    fclose(fp);

    if (!ARCHIVE_HOME_PATH || !*ARCHIVE_HOME_PATH) {
        SET_PATH_VAL(&ARCHIVE_HOME_PATH, "."); // 설정 없으면 현재 위치를 홈으로
    }

    // 경로 파싱 결과 검증 디버그 출력
    printf("[CONFIG LOADED]\n");
    printf(" HOME_PATH: %s\n", ARCHIVE_HOME_PATH ? ARCHIVE_HOME_PATH : "(null)");
}

// 폴더 자동 생성
void ENSURE_HOME_DIRECTORY(void) {
    const char *home = (ARCHIVE_HOME_PATH && *ARCHIVE_HOME_PATH) ? ARCHIVE_HOME_PATH : ".";

    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", home);
    int ret = system(cmd);
    (void)ret;
}