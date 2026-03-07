#include <ncurses.h>
#include <stdlib.h>      
#include <stdio.h>      // sprintf 사용을 위해 추가
#include <string.h>     // 문자열 처리를 위해 추가
#include "SECTOR_MENU.h"
#include "INSfunc.h"    
#include "FILE_CHECK.h"
#include "FILE_SEARCH.h"
#include "FILE_UTIL.h"

// 함수 프로토타입 선언
void INSfile_list();
void INS_copy_to_sector(const char *dest_path, const char *sector_name);
void INS_quick_duplicate_check();

void INS_MAIN_LOOP() {
    // [보완] 프로그램 실행 시 필요한 폴더들이 있는지 확인하고 생성
    //system("mkdir -p ingest_zone ros_storage eds_zone bks_zone");

    const char *ins_items[] = {
        "[1] File add", 
        "[2] INS File List View", 
        "[3] Copy to EDS", 
        "[4] Clone to BackUp(BKS)",
        "[5] Check for File Duplication",
        "[BACK] To Home"
    };
    int ins_count = 6;
    int ins_cursor = 0; 

    while (1) {
        int result = SECTOR_MENU("INS(Ingest Sector) MANAGEMENT", ins_items, ins_count, &ins_cursor);

        if (result >= 0) {
            switch(result) {
                case 0: INSfunc_handle_file_add(); break;
                case 1: INSfile_list(); break;
                case 2: INS_copy_to_sector("./eds_zone/", "EDS"); break; // 슬래시 포함 확인
                case 3: INS_copy_to_sector("./bks_zone/", "BKS"); break; // 슬래시 포함 확인
                case 4: INS_quick_duplicate_check(); break;
                case 5: return;
            }
        }
        if (result == -1) return;
    }
}

/*===============================INS 내의 list===============================*/
void INSfile_list() {
    char *temp_list[100];
    int count = FILE_ALL_LIST_GET("./ingest_zone/", temp_list, 100);

    if (count > 0) {
        int tmp_cursor = 0;
        SECTOR_MENU("CURRENT INGEST FILES", (const char**)temp_list, count, &tmp_cursor);
        for(int i = 0; i < count; i++) free(temp_list[i]);
    } else {
        move(LINES - 2, 2); clrtoeol();
        mvprintw(LINES - 2, 2, "No files found in Ingest Zone.");
        refresh();
        getch();
    }
}

/*==================타 섹션(EDS, BKS)으로 복사를 담당하는 통합 함수==================*/
void INS_copy_to_sector(const char *dest_path, const char *sector_name) {
    char *temp_list[100];
    int count = FILE_ALL_LIST_GET("./ingest_zone/", temp_list, 100);

    if (count <= 0) {
        move(LINES - 2, 2); clrtoeol();
        mvprintw(LINES - 2, 2, "Ingest Zone is empty. Nothing to copy.");
        refresh();
        getch();
        return;
    }

    int tmp_cursor = 0;
    char title[64];
    sprintf(title, "SELECT FILE TO COPY TO %s", sector_name);

    int choice = SECTOR_MENU(title, (const char**)temp_list, count, &tmp_cursor);
    
    if (choice >= 0) {
        char src_full[512], dest_full[512];
        // 경로 생성 시 중간에 슬래시(/)가 겹치거나 빠지지 않도록 주의
        sprintf(src_full, "./ingest_zone/%s", temp_list[choice]);
        sprintf(dest_full, "%s%s", dest_path, temp_list[choice]);

        move(LINES - 3, 2); clrtoeol();
        mvprintw(LINES - 3, 2, "Copying to %s: %s...", sector_name, temp_list[choice]);
        refresh();

        // FILE_UTIL.h의 FILE_COPY가 실제로 복사 수행
        if (FILE_COPY(src_full, dest_full)) {
            move(LINES - 2, 2); clrtoeol();
            mvprintw(LINES - 2, 2, "SUCCESS: Sent to %s Sector.", sector_name);
        } else {
            move(LINES - 2, 2); clrtoeol();
            mvprintw(LINES - 2, 2, "ERROR: Copy to %s failed (Check if folder exists).", sector_name);
        }
        refresh();
        getch();
    }

    for(int i = 0; i < count; i++) free(temp_list[i]);
}

/*==================파일중복 검사를 INS 환경에 맞게 손을 본것.==================*/
void INS_quick_duplicate_check() {
    char *temp_list[100];
    int count = FILE_ALL_LIST_GET("./ingest_zone/", temp_list, 100);

    if (count <= 0) {
        move(LINES - 2, 2); clrtoeol();
        mvprintw(LINES - 2, 2, "Ingest Zone is empty. Nothing to check.");
        refresh();
        getch();
        return;
    }

    int tmp_cursor = 0;
    int choice = SECTOR_MENU("SELECT FILE TO SCAN DUPLICATE", (const char**)temp_list, count, &tmp_cursor);
    
    if (choice >= 0) {
        char src[512], dest[512];
        sprintf(src, "./ingest_zone/%s", temp_list[choice]);
        sprintf(dest, "./ros_storage/%s", temp_list[choice]);

        move(LINES - 4, 2); clrtoeol();
        mvprintw(LINES - 4, 2, "Checking: %s", temp_list[choice]);
        
        if (FILE_EXISTENCE_CHECK(dest)) {
            if (FILE_DUPLICATE_CHECK(src, dest)) {
                mvprintw(LINES - 2, 2, "RESULT: [MATCH] Identical file exists in Storage.");
            } else {
                mvprintw(LINES - 2, 2, "RESULT: [DIFF] Same name, but content is DIFFERENT.");
            }
        } else {
            mvprintw(LINES - 2, 2, "RESULT: [UNIQUE] No matching file name in Storage.");
        }
        refresh();
        getch();
    }

    for(int i = 0; i < count; i++) free(temp_list[i]);
}