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

/* [최적화 1]
 * 기존 DISPLAY_NAME_MAX(264)는 "엔트리당 고정 크기"였다.
 * 이제는 prefix("[DIR]  " / "[FILE] " = 7바이트) + 널문자 1바이트만
 * 고정으로 두고, 나머지는 raw_list 문자열 길이에 맞춰 계산한다.
 * 즉 실제로 필요한 만큼만 쓴다 (파일명이 10바이트면 딱 18바이트만 사용).
 */
#define DISPLAY_PREFIX_LEN 7   /* "[DIR]  " 혹은 "[FILE] " */

static char current_view_path[512];

void INIT_ROS_PATH() {
    memset(current_view_path, 0, sizeof(current_view_path));
    strncpy(current_view_path, ROS_PATH, sizeof(current_view_path) - 1);
}

static int is_directory(const char *path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) return 0;
    return S_ISDIR(statbuf.st_mode);
}

static int is_at_root_path(const char *current, const char *root) {
    char norm_current[512];
    char norm_root[512];

    strncpy(norm_current, current, sizeof(norm_current) - 1);
    norm_current[sizeof(norm_current) - 1] = '\0';
    strncpy(norm_root, root, sizeof(norm_root) - 1);
    norm_root[sizeof(norm_root) - 1] = '\0';

    int len_c = strlen(norm_current);
    int len_r = strlen(norm_root);

    if (len_c > 1 && norm_current[len_c - 1] == '/') norm_current[len_c - 1] = '\0';
    if (len_r > 1 && norm_root[len_r - 1] == '/') norm_root[len_r - 1] = '\0';

    return (strcmp(norm_current, norm_root) == 0);
}

static void navigate_to_parent_dir() {
    int len = strlen(current_view_path);
    if (len > 0 && current_view_path[len - 1] == '/') {
        current_view_path[len - 1] = '\0';
    }

    char *last = strrchr(current_view_path, '/');
    if (last) {
        *(last + 1) = '\0';
    } else {
        INIT_ROS_PATH();
    }
}

/* [최적화 2]
 * 기존: display_list[i] = malloc(264)  -> 파일 개수만큼 malloc 호출 (최대 100회)
 *       각 malloc마다 힙 청크 헤더/정렬 오버헤드(보통 16~32바이트)가 붙어
 *       실사용량과 무관하게 낭비가 누적됨.
 *
 * 개선: 필요한 총 바이트 수를 먼저 계산한 뒤, 단 "한 번의 malloc"으로
 *       모든 display 문자열을 담을 버퍼를 만들고, display_list[i]는
 *       그 버퍼 안의 오프셋을 가리키게 한다.
 *       -> malloc 호출 횟수: 100회 -> 1회
 *       -> 청크 오버헤드: 최대 3.2KB -> 16~32바이트 (1회분)
 *       -> 엔트리당 낭비: 264바이트 고정 -> 실제 필요한 만큼만
 */
void ROSfunc_manage_storage(WINDOW *data_win) {
    int cursor = 0;

    // [핵심 수정] 저장소 관리창 진입 시 이전에 남아있던 타 구역(INS 등) 경로를 무조건 잔여 버퍼 없이 ROS_PATH로 전면 강제 초기화
    INIT_ROS_PATH();

    while (1) {
        char *raw_list[100] = {0};
        int count = FILE_ALL_LIST_GET(current_view_path, raw_list, 100);

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
                    break;
                } else {
                    navigate_to_parent_dir();
                    continue;
                }
            }
            continue;
        }

        /* --- 표시용 버퍼 크기 사전 계산 (1-pass) --- */
        int path_len = (int)strlen(current_view_path);
        size_t total_size = 0;
        int is_dir_flag[100] = {0};
        int name_len[100] = {0};

        for (int i = 0; i < count; i++) {
            /* full path 문자열은 스택의 작은 버퍼로 충분 (512면 대부분 넉넉) */
            char full_temp[512];
            snprintf(full_temp, sizeof(full_temp), "%s%s", current_view_path, raw_list[i]);

            is_dir_flag[i] = is_directory(full_temp);
            name_len[i] = (int)strlen(raw_list[i]);
            /* prefix(7) + 파일명 + null(1) */
            total_size += (size_t)(DISPLAY_PREFIX_LEN + name_len[i] + 1);
        }

        /* [핵심] 단 한 번의 malloc으로 모든 display 문자열 수용 */
        char *display_buf = (char *)malloc(total_size);
        char *display_list[100] = {0};

        if (display_buf == NULL) {
            for (int j = 0; j < count; j++) free(raw_list[j]);
            return;
        }

        {
            char *cursor_buf = display_buf;
            for (int i = 0; i < count; i++) {
                display_list[i] = cursor_buf;
                size_t entry_size = (size_t)(DISPLAY_PREFIX_LEN + name_len[i] + 1);
                if (is_dir_flag[i]) {
                    snprintf(cursor_buf, entry_size, "[DIR]  %s", raw_list[i]);
                } else {
                    snprintf(cursor_buf, entry_size, "[FILE] %s", raw_list[i]);
                }
                cursor_buf += entry_size;
            }
        }

        char title[512];
        snprintf(title, sizeof(title), "EXPLORING: %s", current_view_path);
        (void)path_len;

        int result = SECTOR_MENU_WIN(data_win, title, (const char **)display_list, count, &cursor, SIGN_LEFT_ALIGN);

        if (result == SIGN_CANCEL) {
            for (int i = 0; i < count; i++) if (raw_list[i]) free(raw_list[i]);
            free(display_buf); /* 통짜 블록 1회 해제 */

            if (is_at_root_path(current_view_path, ROS_PATH)) {
                break;
            } else {
                navigate_to_parent_dir();
                cursor = 0;
                continue;
            }
        }
        else if (result == SIGN_DELETE) {
            char target[512];
            snprintf(target, sizeof(target), "%s%s", current_view_path, raw_list[cursor]);

            mvwprintw(data_win, UI_Win_Height - 2, 2, ">> DELETE %s? (y/n)", raw_list[cursor]);
            wrefresh(data_win);

            int confirm_ch = wgetch(data_win);
            if (confirm_ch == 'y' || confirm_ch == 'Y') {
                if (is_directory(target)) {
                    char cmd[600];
                    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", target);
                    if (system(cmd) == -1) {
                        /* 컴파일러 경고 무시 */
                    }
                } else {
                    remove(target);
                }
            }
        }
        else if (result >= 0) {
            char target[512];
            snprintf(target, sizeof(target), "%s%s", current_view_path, raw_list[result]);
            if (is_directory(target)) {
                strncat(current_view_path, raw_list[result],
                        sizeof(current_view_path) - strlen(current_view_path) - 1);
                strncat(current_view_path, "/",
                        sizeof(current_view_path) - strlen(current_view_path) - 1);
                cursor = 0;
            }
        }

        for (int i = 0; i < count; i++) if (raw_list[i]) free(raw_list[i]);
        free(display_buf); /* 통짜 블록 1회 해제 */
    }
}

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
