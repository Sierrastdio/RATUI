#ifndef PATH_CONFIG_H
#define PATH_CONFIG_H

#define Config_FileName "config.ratui"

// 전역 경로 포인터 선언 (char[512] 배열 -> const char * 포인터 변경)
extern const char *INGEST_PATH;
extern const char *ROS_PATH;
extern const char *EDS_PATH;
extern const char *BKS_PATH;
extern const char *TRS_PATH;

void LOAD_CONFIG();
void ENSURE_DIRECTORIES();
void STRIP_NEWLINE(char *str);

#endif
