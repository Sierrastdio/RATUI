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
#include "INSfunc.h"

// 나중에 환경설정 파일로 뺄 수도 있는 경로 정의
#define INGEST_PATH "./ingest_zone/"  // 외부에서 파일이 들어오는 곳 (Samba 등)
#define STORAGE_PATH "./ros_storage/" // 우리 시스템의 실제 저장소 (ROS)

void handle_file_add() {
    char *file_list[100];
    int file_count = 0;
    int cursor = 0;

    file_count = FILE_ALL_LIST_GET(INGEST_PATH, file_list, 100);

    if(file_count < 0) {
        attron(A_REVERSE);
        mvprintw(LINES -3, 2, "[SYSTEM] No files found in Ingest Zone." );
        attroff(A_REVERSE);
        mvprintw(LINES - 2, 2, "Press any key to return...");
        getch();
        return;
    }

    // 2. SECTOR_MENU를 사용하여 파일 선택 창 띄우기
    int choice = SECTOR_MENU("SELECT FILE TO INGEST", (const char **)file_list, file_count, &cursor);

    if (choice >= 0) {
        char src_full_path[512];
        char dest_full_path[512];

        // 원본 경로와 대상 저장소 경로 생성
        sprintf(src_full_path, "%s%s", INGEST_PATH, file_list[choice]);
        sprintf(dest_full_path, "%s%s", STORAGE_PATH, file_list[choice]);

        mvprintw(LINES - 4, 2, "Checking: %s", file_list[choice]);
        refresh();

        // 3. 3단계 중복 검사 실행 (FILE_CHECK 라이브러리 활용)
        // 만약 저장소에 같은 이름의 파일이 이미 있다면 정밀 검사 시작
        if (FILE_EXISTENCE_CHECK(dest_full_path)) {
            mvprintw(LINES - 3, 2, "Duplicate name found. Running Deep Inspection...");
            refresh();

            if (FILE_DUPLICATE_CHECK(src_full_path, dest_full_path)) {
                attron(COLOR_PAIR(1)); // 에러 강조 (색상 설정이 되어있을 경우)
                mvprintw(LINES - 2, 2, "CRITICAL: Identical file already exists in ROS! Cancelled.");
                attroff(COLOR_PAIR(1));
            } else {
                mvprintw(LINES - 2, 2, "Notice: Same name but different content. Proceeding...");
                // 여기에 복사 로직 추가 (예: 파일명 뒤에 _1 붙이기 등)
            }
        } else {
            // 중복이 없으면 바로 복사 프로세스 진행
            mvprintw(LINES - 2, 2, "File is unique. Ready to Ingest.");
            // TODO: 실제 파일 복사 함수 호출 (예: FILE_COPY_MOVE(src, dest))
        }
    }

    // 4. 메모리 해제 (strdup으로 할당된 파일명들)
    for (int i = 0; i < file_count; i++) {
        free(file_list[i]);
    }

    mvprintw(LINES - 1, 2, "Press any key...");
    getch();
}