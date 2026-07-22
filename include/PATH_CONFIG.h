#ifndef PATH_CONFIG_H
#define PATH_CONFIG_H

#include <string.h>

#define Config_FileName "config.ratui"

#define STRIP_NEWLINE(str) do { \
    char *ratui_strip_str = (str); \
    size_t ratui_strip_len = strlen(ratui_strip_str); \
    while (ratui_strip_len > 0 && \
           (ratui_strip_str[ratui_strip_len - 1] == '\n' || \
            ratui_strip_str[ratui_strip_len - 1] == '\r' || \
            ratui_strip_str[ratui_strip_len - 1] == ' ' || \
            ratui_strip_str[ratui_strip_len - 1] == '"')) { \
        ratui_strip_str[ratui_strip_len - 1] = '\0'; \
        ratui_strip_len--; \
    } \
} while (0)

// 전역 경로 포인터 선언 (char[512] 배열 -> const char * 포인터 변경)
extern const char *INGEST_PATH;
extern const char *ROS_PATH;
extern const char *EDS_PATH;
extern const char *BKS_PATH;
extern const char *TRS_PATH;

void LOAD_CONFIG();
void ENSURE_DIRECTORIES();

#endif
