#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "SECTOR_MENU.h"
#include "FILE_CHECK.h"
#include "FILE_SEARCH.h"
#include "FILE_UTIL.h"
#include "PathConfig.h" 
#include "INSfunc.h"

/*===============================파일 추가 로직===============================*/
void INSfunc_handle_file_add() {
    char *file_list[100];
    int file_count = FILE_ALL_LIST_GET(INGEST_PATH, file_list, 100);

    if(file_count <= 0) {
        move(LINES - 2, 2); clrtoeol();
        attron(A_REVERSE);
        mvprintw(LINES - 2, 2, "[SYSTEM] No files found in Ingest Zone.");
        attroff(A_REVERSE);
        refresh(); getch();
        return;
    }

    int cursor = 0;
    int choice = SECTOR_MENU("SELECT FILE TO INGEST", (const char **)file_list, file_count, &cursor);

    if (choice >= 0) {
        char src_full_path[1024], dest_full_path[1024];
        snprintf(src_full_path, sizeof(src_full_path), "%s%s", INGEST_PATH, file_list[choice]);
        snprintf(dest_full_path, sizeof(dest_full_path), "%s%s", ROS_PATH, file_list[choice]);

        if (FILE_EXISTENCE_CHECK(dest_full_path)) {
            if (FILE_DUPLICATE_CHECK(src_full_path, dest_full_path)) {
                mvprintw(LINES - 2, 2, "CRITICAL: Identical file already exists!");
            } else {
                if (FILE_COPY(src_full_path, dest_full_path)) mvprintw(LINES - 1, 2, "SUCCESS: File Overwritten.");
            }
        } else {
            if (FILE_COPY(src_full_path, dest_full_path)) mvprintw(LINES - 1, 2, "SUCCESS: File copied.");
        }
    }
    for (int i = 0; i < file_count; i++) free(file_list[i]);
    refresh(); getch();
}

/*===============================파일 리스트 보기 (통합)===============================*/
void INSfunc_list() {
    char *temp_list[100];
    int count = FILE_ALL_LIST_GET(INGEST_PATH, temp_list, 100);
    if (count > 0) {
        int cursor = 0;
        SECTOR_MENU("CURRENT INGEST FILES", (const char**)temp_list, count, &cursor);
        for(int i = 0; i < count; i++) free(temp_list[i]);
    } else {
        move(LINES - 2, 2); clrtoeol();
        mvprintw(LINES - 2, 2, "No files found.");
        refresh(); getch();
    }
}

/*===============================통합 복사 함수===============================*/
void INS_copy_to_sector(const char *dest_path, const char *sector_name) {
    char *temp_list[100];
    int count = FILE_ALL_LIST_GET(INGEST_PATH, temp_list, 100);
    if (count <= 0) { getch(); return; }

    int cursor = 0;
    char title[64];
    snprintf(title, sizeof(title), "SELECT FILE TO COPY TO %s", sector_name);
    int choice = SECTOR_MENU(title, (const char**)temp_list, count, &cursor);
    
    if (choice >= 0) {
        char src_full[1024], dest_full[1024];
        snprintf(src_full, sizeof(src_full), "%s%s", INGEST_PATH, temp_list[choice]);
        snprintf(dest_full, sizeof(dest_full), "%s%s", dest_path, temp_list[choice]);
        if (FILE_COPY(src_full, dest_full)) mvprintw(LINES - 2, 2, "SUCCESS: Sent to %s.", sector_name);
        refresh(); getch();
    }
    for(int i = 0; i < count; i++) free(temp_list[i]);
}

// 스위치문 호출용 래퍼 함수들
void INS_copy_to_eds() { INS_copy_to_sector(EDS_PATH, "EDS"); }
void INS_copy_to_bks() { INS_copy_to_sector(BKS_PATH, "BKS"); }

/*===============================중복 검사 함수===============================*/
void INS_quick_duplicate_check() {
    char *temp_list[100];
    int count = FILE_ALL_LIST_GET(INGEST_PATH, temp_list, 100);
    if (count <= 0) return;

    int cursor = 0;
    int choice = SECTOR_MENU("SELECT FILE TO SCAN", (const char**)temp_list, count, &cursor);
    if (choice >= 0) {
        char src[1024], dest[1024];
        snprintf(src, sizeof(src), "%s%s", INGEST_PATH, temp_list[choice]);
        snprintf(dest, sizeof(dest), "%s%s", ROS_PATH, temp_list[choice]);
        // ... (이전 중복 검사 로직)
        mvprintw(LINES - 2, 2, FILE_EXISTENCE_CHECK(dest) ? "RESULT: Found." : "RESULT: Unique.");
        refresh(); getch();
    }
    for(int i = 0; i < count; i++) free(temp_list[i]);
}