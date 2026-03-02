/*
 *  FILE_CHECK.h  FILE_SEARCH.h 와 INSfunc.h의 차이점은
 *  전자의 경우 다른 섹터에서도 쓰일수 있는 파일 검사, 검색 함수들인데 반해
 *  INSfunc.c는 INS 섹터 안에서만 쓰는 전용 기능이다.
 *
 */

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "SECTOR_MENU.h"
#include "FILE_CHECK.h"
#include "FILE_SEARCH.h"
#include "FILE_UTIL.h"   // FILE_MOVE 함수를 사용하기 위해 추가
#include "INSfunc.h"

#define INGEST_PATH "./ingest_zone/"
#define STORAGE_PATH "./ros_storage/"

void INSfunc_handle_file_add() {
    char *file_list[100];
    int file_count = 0;
    int cursor = 0;

    file_count = FILE_ALL_LIST_GET(INGEST_PATH, file_list, 100);

    // file_count가 0 이하일 때 처리 보정
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

        if (FILE_EXISTENCE_CHECK(dest_full_path)) {
            move(LINES - 3, 2); clrtoeol();
            mvprintw(LINES - 3, 2, "Duplicate name found. Running Deep Inspection...");
            refresh();

            if (FILE_DUPLICATE_CHECK(src_full_path, dest_full_path)) {
                move(LINES - 2, 2); clrtoeol();
                mvprintw(LINES - 2, 2, "CRITICAL: Identical file already exists in ROS! Cancelled.");
            } else {
                move(LINES - 2, 2); clrtoeol();
                mvprintw(LINES - 2, 2, "Notice: Same name but different content. Moving with prefix...");
                // 실제 서비스라면 파일명 뒤에 _new 등을 붙이겠지만, 
                // 우선은 덮어쓰거나 이동하는 로직이 필요합니다.
                if (FILE_MOVE(src_full_path, dest_full_path)) {
                     mvprintw(LINES - 1, 2, "SUCCESS: File Ingested (Overwritten/Updated).");
                }
            }
        } else {
            // [수정 포인트] 중복이 없으면 실제로 파일을 옮겨야 합니다!
            move(LINES - 2, 2); clrtoeol();
            mvprintw(LINES - 2, 2, "File is unique. Starting Ingest...");
            refresh();

            // FILE_UTIL.h에 정의된 FILE_MOVE 호출 (복사 후 원본 삭제)
            if (FILE_MOVE(src_full_path, dest_full_path)) {
                mvprintw(LINES - 1, 2, "SUCCESS: File moved to ROS Storage.");
            } else {
                mvprintw(LINES - 1, 2, "ERROR: Ingest failed. Check permissions.");
            }
        }
    }

    for (int i = 0; i < file_count; i++) {
        free(file_list[i]);
    }

    move(LINES - 1, 22); // 메시지 뒤에 대기
    mvprintw(LINES - 1, 2, "Press any key...");
    refresh();
    getch();
}