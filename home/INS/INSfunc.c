#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "SECTOR_MENU.h"
#include "FILE_CHECK.h"
#include "FILE_SEARCH.h"
#include "FILE_UTIL.h"   
#include "INSfunc.h"

#define INGEST_PATH "./ingest_zone/"
#define STORAGE_PATH "./ros_storage/"

void INSfunc_handle_file_add() {
    char *file_list[100];
    int file_count = 0;
    int cursor = 0;

    file_count = FILE_ALL_LIST_GET(INGEST_PATH, file_list, 100);

    if(file_count <= 0) {
        move(LINES - 2, 2); clrtoeol();
        attron(A_REVERSE);
        mvprintw(LINES - 2, 2, "[SYSTEM] No files found in Ingest Zone.");
        attroff(A_REVERSE);
        refresh();
        getch();
        return;
    }

    int choice = SECTOR_MENU("SELECT FILE TO INGEST", (const char **)file_list, file_count, &cursor);

    if (choice >= 0) {
        char src_full_path[512];
        char dest_full_path[512];

        sprintf(src_full_path, "%s%s", INGEST_PATH, file_list[choice]);
        sprintf(dest_full_path, "%s%s", STORAGE_PATH, file_list[choice]);

        move(LINES - 4, 2); clrtoeol();
        mvprintw(LINES - 4, 2, "Checking: %s", file_list[choice]);
        refresh();

        // 1. 이미 같은 이름의 파일이 ROS에 있는 경우
        if (FILE_EXISTENCE_CHECK(dest_full_path)) {
            move(LINES - 3, 2); clrtoeol();
            mvprintw(LINES - 3, 2, "Duplicate name found. Running Deep Inspection...");
            refresh();

            if (FILE_DUPLICATE_CHECK(src_full_path, dest_full_path)) {
                move(LINES - 2, 2); clrtoeol();
                mvprintw(LINES - 2, 2, "CRITICAL: Identical file already exists in ROS! Cancelled.");
            } else {
                move(LINES - 2, 2); clrtoeol();
                mvprintw(LINES - 2, 2, "Notice: Same name but different content. Updating...");
                
                // [수정] 복사 호출
                if (FILE_COPY(src_full_path, dest_full_path)) {
                     mvprintw(LINES - 1, 2, "SUCCESS: File Overwritten in ROS. Original kept.");
                }
            }
        } 
        // 2. 새로운 파일인 경우
        else {
            move(LINES - 2, 2); clrtoeol();
            mvprintw(LINES - 2, 2, "File is unique. Starting Ingest (Copy)...");
            refresh();

            // [수정 포인트] FILE_MOVE를 FILE_COPY로 변경하여 원본 유지
            if (FILE_COPY(src_full_path, dest_full_path)) {
                mvprintw(LINES - 1, 2, "SUCCESS: File copied to ROS Storage. Original remains in Ingest.");
            } else {
                mvprintw(LINES - 1, 2, "ERROR: Ingest failed. Check permissions.");
            }
        }
    }

    // 메모리 해제
    for (int i = 0; i < file_count; i++) {
        free(file_list[i]);
    }

    move(LINES - 1, 22); 
    mvprintw(LINES - 1, 2, "Press any key...");
    refresh();
    getch();
}