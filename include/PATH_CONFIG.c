#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "PATH_CONFIG.h"

// 시스템의 실제 최대 경로 길이(PATH_MAX가 없으면 256 사용)
#ifndef PATH_MAX
#define PATH_MAX 256
#endif

// HOME_PATH= 키워드(10바이트) + 경로 최대 길이 + 여유분
#define LINE_BUF_SIZE (PATH_MAX + 16)

char *ARCHIVE_HOME_PATH = NULL;

void FREE_CONFIG(void) {
    if (ARCHIVE_HOME_PATH) {
        free(ARCHIVE_HOME_PATH);
        ARCHIVE_HOME_PATH = NULL;
    }
}

void LOAD_CONFIG(void) {
    FREE_CONFIG();

    FILE *fp = fopen(Config_FileName, "r");
    if (!fp) {
        printf("Error: Could not open %s\n", Config_FileName);
        exit(1);
    }

    // 파일의 한 줄을 임시로 담을 최소한의 스택 버퍼
    char line[LINE_BUF_SIZE];
    char *parsed_path = NULL;

    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "HOME_PATH=", 10) == 0) {
            char *val = line + 10;

            // 앞쪽 공백 및 따옴표 제거
            while (*val == ' ' || *val == '"') {
                val++;
            }

            // 뒤쪽 개행, 공백, 따옴표 제거
            size_t len = strlen(val);
            while (len > 0 && (val[len - 1] == '\n' || val[len - 1] == '\r' || 
                               val[len - 1] == ' '  || val[len - 1] == '"')) {
                val[--len] = '\0';
            }

            parsed_path = val;
            break;
        }
    }
    fclose(fp);

    // 파싱 결과가 없으면 기본값 "."
    if (!parsed_path || *parsed_path == '\0') {
        parsed_path = ".";
    }

    // 실제 경로의 딱 필요한 길이(strlen + 1)만큼만 Heap에 동적 할당
    ARCHIVE_HOME_PATH = strdup(parsed_path);

    printf("[CONFIG LOADED]\n HOME_PATH: %s\n", ARCHIVE_HOME_PATH);
}

void ENSURE_HOME_DIRECTORY(void) {
    const char *home = ARCHIVE_HOME_PATH ? ARCHIVE_HOME_PATH : ".";
    
    // 명령어 생성용 최소 버퍼 (mkdir -p "" 감싸기용 16바이트 추가)
    char cmd[PATH_MAX + 16];
    snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", home);
    int ret = system(cmd);
    (void)ret;
}