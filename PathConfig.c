#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "PathConfig.h"

// 1. 전역 변수 정의
char INGEST_PATH[512] = "./ingest_zone/";
char ROS_PATH[512] = "./ros_storage/";
char EDS_PATH[512] = "./eds_storage/";
char BKS_PATH[512] = "./bks_storage/";
char TRS_PATH[512] = "./trs_storage/";

// 2. 설정 파일 읽기
void LOAD_CONFIG() {
    FILE *fp = fopen("config.txt", "r");
    if (!fp) return;

    char line[1024]; // 변수 선언은 함수 시작 부분에 있어야 함
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        if (strncmp(line, "INGEST_PATH=", 12) == 0) strcpy(INGEST_PATH, line + 12);
        else if (strncmp(line, "ROS_STORAGE=", 12) == 0) strcpy(ROS_PATH, line + 12);
        else if (strncmp(line, "EDS_STORAGE=", 12) == 0) strcpy(EDS_PATH, line + 12);
        else if (strncmp(line, "BKS_STORAGE=", 12) == 0) strcpy(BKS_PATH, line + 12);
        else if (strncmp(line, "TRS_STORAGE=", 12) == 0) strcpy(TRS_PATH, line + 12);
        
        // 줄바꿈 제거
        INGEST_PATH[strcspn(INGEST_PATH, "\r\n")] = 0;
        ROS_PATH[strcspn(ROS_PATH, "\r\n")] = 0;
        EDS_PATH[strcspn(EDS_PATH, "\r\n")] = 0;
        BKS_PATH[strcspn(BKS_PATH, "\r\n")] = 0;
        TRS_PATH[strcspn(TRS_PATH, "\r\n")] = 0;
    }
    fclose(fp);
}

// 3. 폴더 자동 생성
void ENSURE_DIRECTORIES() {
    char cmd[4096];
    // snprintf를 사용하면 버퍼 크기(sizeof(cmd))를 넘어서지 않도록 보장합니다.
    snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\" \"%s\" \"%s\" \"%s\"", ROS_PATH, EDS_PATH, BKS_PATH, TRS_PATH);
    system(cmd);
}