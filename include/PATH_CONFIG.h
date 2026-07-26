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

/* 섹터(INS/ROS/EDS/BKS/TRS) 개념을 안 쓰기로 해서, 절대경로 파일 탐색기의
 * 시작점("홈" 경로) 하나만 관리한다. config.ratui 의 HOME_PATH= 줄에서 읽어온다. */
extern const char *ARCHIVE_HOME_PATH;

void LOAD_CONFIG(void);
void ENSURE_HOME_DIRECTORY(void);

#endif