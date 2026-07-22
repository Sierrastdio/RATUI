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

#define DISPLAY_NAME_MAX 264

static char current_view_path[512];

void INIT_ROS_PATH() {
    strcpy(current_view_path, ROS_PATH);
}

static int is_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) return 0;
    return S_ISDIR(statbuf.st_mode);
}

// 경로가 루트 경로인지 비교하는 안전한 함수
static int is_at_root_path(const char *current, const char *root) {
    char norm_current[512];
    char norm_root[512];

    strncpy(norm_current, current, sizeof(norm_current) - 1);
    strncpy(norm_root, root, sizeof(norm_root) - 1);

    int len_c = strlen(norm_current);
    int len_r = strlen(norm_root);

    if (len_c > 1 && norm_current[len_c - 1] == '/') norm_current[len_c - 1] = '\0';
    if (len_r > 1 && norm_root[len_r - 1] == '/') norm_root[len_r - 1] = '\0';

    return (strcmp(norm_current, norm_root) == 0);
}

// 상위 경로로 이동시키는 함수
static void navigate_to_parent_dir() {
    int len = strlen(current_view_path);
    if (len > 0 && current_view_path[len - 1] == '/') {
        current_view_path[len - 1] = '\0';
    }
    
    char *last = strrchr(current_view_path, '/');
    if (last) {
        *(last + 1) = '\0';
    } else {
        strcpy(current_view_path, ROS_PATH);
    }
}

// 우측 윈도우 영역 안에서 동작하는 저장소 관리자
void ROSfunc_manage_storage(WINDOW *data_win) {
    int cursor = 0;

    if (strlen(current_view_path) == 0) INIT_ROS_PATH();

    while (1) {
        char *raw_list[100] = {0};
        char *display_list[100] = {0};

        // 1. 파일 리스트 가져오기
        int count = FILE_ALL_LIST_GET(current_view_path, raw_list, 100);

        // 폴더가 비었을 때 처리
        if (count <= 0) {
            werase(data_win);
            box(data_win, 0, 0);

            mvwprintw(data_win, UI_Win_Height - 3, 2, "Empty Directory.");
            mvwprintw(data_win, UI_Win_Height - 2, 2, "Path: %s", current_view_path);
            mvwprintw(data_win, UI_Win_Height - 1, 2, "Press [q/ESC] to return...");
            wrefresh(data_win);

            keypad(data_win, TRUE);
            int ch = wgetch(data_win);

            if (ch == 27 || ch == 'q' || ch == 'Q') {
                if (is_at_root_path(current_view_path, ROS_PATH)) {
                    break; // 루트 경로면 완전 탈출
                } else {
                    navigate_to_parent_dir();
                    continue;
                }
            }
            continue;
        }

        // 2. 표시용 리스트 생성
        for (int i = 0; i < count; i++) {
            char full_temp[1024];
            snprintf(full_temp, sizeof(full_temp), "%s%s", current_view_path, raw_list[i]);
            display_list[i] = (char *)malloc(DISPLAY_NAME_MAX);

            if (display_list[i] == NULL) {
                for (int j = 0; j < i; j++) free(display_list[j]);
                for (int j = 0; j < count; j++) free(raw_list[j]);
                return;
            }

            if (is_directory(full_temp)) {
                snprintf(display_list[i], DISPLAY_NAME_MAX, "[DIR]  %s", raw_list[i]);
            } else {
                snprintf(display_list[i], DISPLAY_NAME_MAX, "[FILE] %s", raw_list[i]);
            }
        }

        char title[1024];
        snprintf(title, sizeof(title), "EXPLORING: %s", current_view_path);

        // 3. 우측 데이터 윈도우 안에서 리스트 표출 및 커서 처리
        int result = SECTOR_MENU_WIN(data_win, title, (const char **)display_list, count, &cursor, SIGN_LEFT_ALIGN);

        // [결과 1] 취소 신호 (Q, ESC)
        if (result == SIGN_CANCEL) {
            // 사용한 메모리 즉시 해제
            for (int i = 0; i < count; i++) {
                if (raw_list[i]) free(raw_list[i]);
                if (display_list[i]) free(display_list[i]);
            }

            if (is_at_root_path(current_view_path, ROS_PATH)) {
                break; // ROS 루트 경로면 함수 완전히 탈출 (이전 메뉴로 복귀)
            } else {
                navigate_to_parent_dir(); // 하위 디렉토리면 상위 폴더로 이동
                cursor = 0;
                continue;
            }
        }
        // [결과 2] d키로 삭제
        else if (result == SIGN_DELETE) {
            char target[1024];
            snprintf(target, sizeof(target), "%s%s", current_view_path, raw_list[cursor]);

            mvwprintw(data_win, UI_Win_Height - 2, 2, ">> DELETE %s? (y/n)", raw_list[cursor]);
            wrefresh(data_win);

            int confirm_ch = wgetch(data_win);
            if (confirm_ch == 'y' || confirm_ch == 'Y') {
                if (is_directory(target)) {
                    char cmd[1100];
                    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", target);
                    if (system(cmd) == -1) {
                        // 컴파일러 경고 무시
                    }
                } else {
                    remove(target);
                }
            }
        }
        // [결과 3] 엔터로 디렉토리 진입
        else if (result >= 0) {
            char target[1024];
            snprintf(target, sizeof(target), "%s%s", current_view_path, raw_list[result]);
            if (is_directory(target)) {
                strcat(current_view_path, raw_list[result]);
                strcat(current_view_path, "/");
                cursor = 0;
            }
        }

        // 메모리 해제
        for (int i = 0; i < count; i++) {
            if (raw_list[i]) free(raw_list[i]);
            if (display_list[i]) free(display_list[i]);
        }
    }
}

// 우측 윈도우 영역 안에서 동작하는 정보 디스플레이
void ROSfunc_show_info(WINDOW *data_win) {
    werase(data_win);
    box(data_win, 0, 0);

    char header_str[] = " === ROS STORAGE STATUS === ";
    UI_GET_RIGHT_WIN_CENTER_X((int)strlen(header_str));
    UI_GET_WIN_CENTER_Y(-3);

    wattron(data_win, A_REVERSE);
    mvwprintw(data_win, UI_Center_y, UI_Center_x, "%s", header_str);
    wattroff(data_win, A_REVERSE);

    char path_str[256];
    snprintf(path_str, sizeof(path_str), "Current Root: %s", ROS_PATH);
    UI_GET_RIGHT_WIN_CENTER_X((int)strlen(path_str));
    UI_GET_WIN_CENTER_Y(-1);

    mvwprintw(data_win, UI_Center_y, UI_Center_x, "%s", path_str);

    char desc_str[] = "Control: [ENTER] to Enter DIR, [d] to Delete Any";
    UI_GET_RIGHT_WIN_CENTER_X((int)strlen(desc_str));
    UI_GET_WIN_CENTER_Y(1);

    mvwprintw(data_win, UI_Center_y, UI_Center_x, "%s", desc_str);

    char footer_str[] = "Press [ESC] or [Q] to return...";
    UI_GET_RIGHT_WIN_CENTER_X((int)strlen(footer_str));
    UI_Center_y = UI_Win_Height - 2;

    mvwprintw(data_win, UI_Center_y, UI_Center_x, "%s", footer_str);

    wrefresh(data_win);

    keypad(data_win, TRUE);
    while (1) {
        int ch = wgetch(data_win);
        if (ch == 27 || ch == 'q' || ch == 'Q') {
            break;
        }
    }
}
