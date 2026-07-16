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
#include "PATH_CONFIG.h"
#include "UI_PRINT.h"

static char current_view_path[512];

void INIT_ROS_PATH() {
    strcpy(current_view_path, ROS_PATH);
}

static int is_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) return 0;
    return S_ISDIR(statbuf.st_mode);
}

// 우측 윈도우 영역 안에서 동작하는 저장소 관리자
void ROSfunc_manage_storage(WINDOW *data_win) {
    int cursor = 0;
    int win_height = getmaxy(data_win);

    if (strlen(current_view_path) == 0) INIT_ROS_PATH();

    while (1) {
        char *raw_list[100];
        char *display_list[100];

        // 1. 파일 리스트 가져오기
        int count = FILE_ALL_LIST_GET(current_view_path, raw_list, 100);

        // 폴더가 비었을 때 처리
        if (count <= 0) {
            werase(data_win);
            box(data_win, 0, 0);

            // 윈도우 내부 하단에 빈 디렉토리 안내문 배치
            mvwprintw(data_win, win_height - 2, 2, "Empty Directory.");
            mvwprintw(data_win, win_height - 1, 2, "Path: %s", current_view_path);
            wrefresh(data_win);

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

        // 2. 표시용 리스트 생성
        for (int i = 0; i < count; i++) {
            char full_temp[1024];
            sprintf(full_temp, "%s%s", current_view_path, raw_list[i]);
            display_list[i] = (char *)malloc(512);

            if (is_directory(full_temp)) sprintf(display_list[i], "[DIR]  %s", raw_list[i]);
            else sprintf(display_list[i], "[FILE] %s", raw_list[i]);
        }

        char title[1024];
        sprintf(title, "EXPLORING: %s", current_view_path);

        // 3. 우측 데이터 윈도우 안에서 리스트 표출 및 커서 처리
        int result = SECTOR_MENU_WIN(data_win, title, (const char **)display_list, count, &cursor, 40);

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

            // 우측 윈도우 하단 영역에 삭제 질문 가이드 노출
            mvwprintw(data_win, win_height - 2, 2, ">> DELETE %s? (y/n)", raw_list[cursor]);
            wrefresh(data_win);

            if (getch() == 'y') {
                if (is_directory(target)) {
                    char cmd[1100];
                    sprintf(cmd, "rm -rf \"%s\"", target);
                    if (system(cmd) == -1) {
                        // 컴파일러 경고 방지
                    }
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

// 우측 윈도우 영역 안에서 동작하는 정보 디스플레이 (ESC / q 전용 탈출 제어 적용)
void ROSfunc_show_info(WINDOW *data_win) {
    werase(data_win);
    box(data_win, 0, 0);

    int win_width = getmaxx(data_win);
    int win_height = getmaxy(data_win);

    // 1. 헤더 (역상 적용)
    int header_len = sizeof(" === ROS STORAGE STATUS === ") - 1;
    int center_x = (win_width - header_len) / 2;
    int center_y = (win_height / 2) - 3;

    wattron(data_win, A_REVERSE);
    mvwprintw(data_win, center_y, center_x, " === ROS STORAGE STATUS === ");
    wattroff(data_win, A_REVERSE);

    // 2. 루트 경로 표시
    int total_path_len = (sizeof("Current Root: ") - 1) + (int)strlen(ROS_PATH);
    center_x = (win_width - total_path_len) / 2;
    center_y = (win_height / 2) - 1;

    mvwprintw(data_win, center_y, center_x, "Current Root: %s", ROS_PATH);

    // 3. 컨트롤 설명 표시
    int desc_len = sizeof("Control: [ENTER] to Enter DIR, [d] to Delete Any") - 1;
    center_x = (win_width - desc_len) / 2;
    center_y = (win_height / 2) + 1;

    mvwprintw(data_win, center_y, center_x, "Control: [ENTER] to Enter DIR, [d] to Delete Any");

    // 4. 하단 안내문 (사용할 올바른 단축키 가이드 매핑)
    int footer_len = sizeof("Press [ESC] or [Q] to return...") - 1;
    center_x = (win_width - footer_len) / 2;
    center_y = win_height - 2;

    mvwprintw(data_win, center_y, center_x, "Press [ESC] or [Q] to return...");

    wrefresh(data_win);

    // [수정 핵심] 루프를 돌면서 ESC(27), q, Q 이외의 키 입력(방향키 등)은 무시하게 막음
    keypad(data_win, TRUE);
    while (1) {
        int ch = wgetch(data_win);
        if (ch == 27 || ch == 'q' || ch == 'Q') {
            break;
        }
    }
}
