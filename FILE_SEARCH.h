#ifndef FILE_SEARCH_H
#define FILE_SEARCH_H

#include <ncurses.h>

// 확장자 기반 파일 검색 (특정 섹터 내 해당 확장자 목록 출력)
// 리턴값: 찾은 파일 개수
int FILE_NAME_EXTENSION_SEARCH (const char *directory, const char *ext, char *result_list[], int max_result);

// 디렉토리 내 전체 파일 목록 가져오기
int FILE_ALL_LIST_GET(const char *directory, char *result_list[], int max_results);

#endif