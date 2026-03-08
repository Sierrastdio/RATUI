#include "FILE_SEARCH.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

// [기능 1] 특정 확장자 파일 검색
int FILE_NAME_EXTENSION_SEARCH(const char *directory, const char *ext, char *result_list[], int max_results) {
    DIR *d = opendir(directory);
    struct dirent *dir;
    int count = 0;

    if (!d) return 0;

    while ((dir = readdir(d)) != NULL && count < max_results) {
        if (dir->d_type == DT_REG) { // 일반 파일만
            char *dot = strrchr(dir->d_name, '.');
            if (dot && strcmp(dot, ext) == 0) {
                result_list[count] = strdup(dir->d_name); // 이름 복사 저장
                count++;
            }
        }
    }
    closedir(d);
    return count;
}

int FILE_ALL_LIST_GET(const char *directory, char *result_list[], int max_results) {
    DIR *d = opendir(directory);
    struct dirent *dir;
    int count = 0;
    if (!d) return -1;

    while ((dir = readdir(d)) != NULL && count < max_results) {
        if (dir->d_name[0] != '.') { // 숨김 파일 및 ., .. 제외
            result_list[count] = strdup(dir->d_name);
            count++;
        }
    }
    closedir(d);
    return count;
}