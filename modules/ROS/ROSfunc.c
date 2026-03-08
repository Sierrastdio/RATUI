#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "SECTOR_MENU.h"
#include "FILE_SEARCH.h"
#include "FILE_UTIL.h"
#include "ROSfunc.h"
#include "PathConfig.h" // 경로 설정 헤더

// [수정] static 초기값을 직접 주지 않고, main에서 호출한 LOAD_CONFIG 이후 값을 복사해야 합니다.
static char current_view_path[512];
// 초기화 함수 (필요 시)
void INIT_ROS_PATH() {
    strcpy(current_view_path, ROS_PATH);
}

static int is_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) return 0;
    return S_ISDIR(statbuf.st_mode);
}

void ROSfunc_manage_storage() {
    int cursor = 0; 
    
    // [추가] 진입 시마다 경로가 비어있으면 초기화
    if (strlen(current_view_path) == 0) INIT_ROS_PATH();

    while (1) {
        char *raw_list[100];
        char *display_list[100];
        
        // 1. 파일 리스트 가져오기
        int count = FILE_ALL_LIST_GET(current_view_path, raw_list, 100);

        // 폴더가 비었을 때 처리
        if (count <= 0) {
            clear();     // [추가] 화면을 깨끗하게 지웁니다.
            refresh();   // [추가] 지운 상태를 적용합니다.
            
            move(LINES - 2, 2); clrtoeol();
            mvprintw(LINES - 2, 2, "Empty Directory. [%s] (ESC: Back)", current_view_path);
            refresh();
            int ch = getch();
            if (ch == 27 || ch == 'q' || ch == 'Q') {
                if (strcmp(current_view_path, ROS_PATH) == 0) break;
                else {
                    char *last = strrchr(current_view_path, '/');
                    if (last) {
                        *last = '\0';
                        last = strrchr(current_view_path, '/');
                        if (last) *(last + 1) = '\0';
                        else strcpy(current_view_path, ROS_PATH);
                    }
                    continue;
                }
            }
            continue;
        }

        // 2. 표시용 리스트 생성 (생략되었던 부분 복구)
        for (int i = 0; i < count; i++) {
            char full_temp[1024];
            sprintf(full_temp, "%s%s", current_view_path, raw_list[i]);
            display_list[i] = (char *)malloc(512);
            
            if (is_directory(full_temp)) sprintf(display_list[i], "[DIR]  %s", raw_list[i]);
            else sprintf(display_list[i], "[FILE] %s", raw_list[i]);
        }

        char title[1024];
        sprintf(title, "EXPLORING: %s", current_view_path);

        // 3. 메뉴 호출
        int result = SECTOR_MENU(title, (const char **)display_list, count, &cursor, ROS);

        // [결과 1] 취소 또는 상위 이동
        if (result == -1) {
            if (strcmp(current_view_path, ROS_PATH) == 0) {
                for (int i = 0; i < count; i++) { free(raw_list[i]); free(display_list[i]); }
                break; 
            } else {
                char *last = strrchr(current_view_path, '/');
                if (last) {
                    *last = '\0';
                    last = strrchr(current_view_path, '/');
                    if (last) *(last + 1) = '\0';
                    else strcpy(current_view_path, ROS_PATH);
                }
            }
        }
        // [결과 2] d키로 삭제
        else if (result == -2) {
            char target[1024];
            sprintf(target, "%s%s", current_view_path, raw_list[cursor]);
            move(LINES - 2, 2); clrtoeol();
            mvprintw(LINES - 2, 2, ">> DELETE %s? (y/n)", raw_list[cursor]);
            refresh();
            if (getch() == 'y') {
                if (is_directory(target)) {
                    char cmd[1100];
                    sprintf(cmd, "rm -rf \"%s\"", target);
                    system(cmd); // 컴파일러 경고
                } else remove(target);
            }
        }
        // [결과 3] 엔터로 진입
        else if (result >= 0) {
            char target[1024];
            sprintf(target, "%s%s", current_view_path, raw_list[result]);
            if (is_directory(target)) {
                strcat(current_view_path, raw_list[result]);
                strcat(current_view_path, "/");
                cursor = 0;
            }
        }

        for (int i = 0; i < count; i++) { free(raw_list[i]); free(display_list[i]); }
    }
}

void ROSfunc_show_info() {
    clear();
    attron(A_REVERSE);
    mvprintw(1, 2, " === ROS STORAGE STATUS === ");
    attroff(A_REVERSE);
    
    //하드코딩된 경로 대신 전역 변수 ROS_PATH 사용
    mvprintw(3, 4, "Current Root: %s", ROS_PATH);
    
    mvprintw(4, 4, "Control: [ENTER] to Enter DIR, [d] to Delete Any");
    mvprintw(LINES - 2, 2, "Press any key to return...");
    refresh();
    getch();
}