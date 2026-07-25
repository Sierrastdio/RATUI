#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "PATH_CONFIG.h"

// 1. 전역 포인터 변수 초기화 (기초값 NULL)
const char *INGEST_PATH = NULL;
const char *ROS_PATH    = NULL;
const char *EDS_PATH    = NULL;
const char *BKS_PATH    = NULL;
const char *TRS_PATH    = NULL;

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

// 2. 설정 파일 읽기
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

        if (strncmp(line, "INGEST_PATH=", 12) == 0) {
            SET_PATH_VAL(&INGEST_PATH, line + 12);
        } else if (strncmp(line, "ROS_STORAGE=", 12) == 0) {
            SET_PATH_VAL(&ROS_PATH, line + 12);
        } else if (strncmp(line, "EDS_STORAGE=", 12) == 0) {
            SET_PATH_VAL(&EDS_PATH, line + 12);
        } else if (strncmp(line, "BKS_STORAGE=", 12) == 0) {
            SET_PATH_VAL(&BKS_PATH, line + 12);
        } else if (strncmp(line, "TRS_STORAGE=", 12) == 0) {
            SET_PATH_VAL(&TRS_PATH, line + 12);
        }
    }
    fclose(fp);

    // 경로 파싱 결과 검증 디버그 출력
    printf("[CONFIG LOADED]\n");
    printf(" INGEST: %s\n", INGEST_PATH ? INGEST_PATH : "(null)");
    printf(" ROS:    %s\n", ROS_PATH    ? ROS_PATH    : "(null)");
    printf(" EDS:    %s\n", EDS_PATH    ? EDS_PATH    : "(null)");
    printf(" BKS:    %s\n", BKS_PATH    ? BKS_PATH    : "(null)");
    printf(" TRS:    %s\n", TRS_PATH    ? TRS_PATH    : "(null)");
}

// 3. 폴더 자동 생성
void ENSURE_DIRECTORIES(void) {
    // 포인터가 NULL이거나 비어있을 경우 안전 처리
    const char *ros = (ROS_PATH && *ROS_PATH) ? ROS_PATH : ".";
    const char *eds = (EDS_PATH && *EDS_PATH) ? EDS_PATH : ".";
    const char *bks = (BKS_PATH && *BKS_PATH) ? BKS_PATH : ".";
    const char *trs = (TRS_PATH && *TRS_PATH) ? TRS_PATH : ".";

    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\" \"%s\" \"%s\" \"%s\"", ros, eds, bks, trs);
    int ret = system(cmd);
    (void)ret;
}
