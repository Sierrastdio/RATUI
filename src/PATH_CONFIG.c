#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "PATH_CONFIG.h"

#define Config_ext ".ratui"

// 1. 전역 변수 정의
char INGEST_PATH[512] = "";
char ROS_PATH[512] = "";
char EDS_PATH[512] = "";
char BKS_PATH[512] = "";
char TRS_PATH[512] = "";

// 개행 문자 제거 함수
void STRIP_NEWLINE(char *str) {
    size_t len = strlen (str);
    while(len > 0 && (str[len -1 ] == '\n' || str[len-1] == '\r')) {
        str[len-1] = '\0';
        len --;
    }
}



// 2. 설정 파일 읽기
void LOAD_CONFIG() {
    FILE *fp = fopen(Config_FileName, "r");
    if (!fp) {
        printf("Error: Could not open %s\n", Config_FileName);
        exit(1); // 설정 파일이 없으면 진행하는 것이 위험하므로 강제 종료 추천
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        STRIP_NEWLINE(line); // 미리 줄바꿈 문자를 제거함

        if (line[0] == '#' || line[0] == '\0') continue;

        if (strncmp(line, "INGEST_PATH=", 12) == 0) strcpy(INGEST_PATH, line + 12);
        else if (strncmp(line, "ROS_STORAGE=", 12) == 0) strcpy(ROS_PATH, line + 12);
        else if (strncmp(line, "EDS_STORAGE=", 12) == 0) strcpy(EDS_PATH, line + 12);
        else if (strncmp(line, "BKS_STORAGE=", 12) == 0) strcpy(BKS_PATH, line + 12);
        else if (strncmp(line, "TRS_STORAGE=", 12) == 0) strcpy(TRS_PATH, line + 12);
    }
    fclose(fp);
}

// 3. 폴더 자동 생성
void ENSURE_DIRECTORIES() {
    char cmd[4096];
    // snprintf를 사용하면 버퍼 크기(sizeof(cmd))를 넘어서지 않도록 보장합니다.
    snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\" \"%s\" \"%s\" \"%s\"", ROS_PATH, EDS_PATH, BKS_PATH, TRS_PATH);
    system(cmd); // 컴파일러 경고
}
