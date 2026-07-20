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

// 리눅스 파일 이름 최대치(255) + 태그 정렬 공간을 고려한 버퍼 크기
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

// 우측 윈도우 영역 안에서 동작하는 저장소 관리자
void ROSfunc_manage_storage(WINDOW *data_win) {
    int cursor = 0;

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
            mvwprintw(data_win, UI_Win_Height - 2, 2, "Empty Directory.");
            mvwprintw(data_win, UI_Win_Height - 1, 2, "Path: %s", current_view_path);
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
            display_list[i] = (char *)malloc(DISPLAY_NAME_MAX);

            // 메모리 할당 예외 처리
            if (display_list[i] == NULL) {
                for (int j = 0; j < i; j++) free(display_list[j]);
                for (int j = 0; j < count; j++) free(raw_list[j]);
                return;
            }

            if (is_directory(full_temp)) sprintf(display_list[i], "[DIR]  %s", raw_list[i]);
            else sprintf(display_list[i], "[FILE] %s", raw_list[i]);
        }

        char title[1024];
        sprintf(title, "EXPLORING: %s", current_view_path);

        // 3. 우측 데이터 윈도우 안에서 리스트 표출 및 커서 처리
        int result = SECTOR_MENU_WIN(data_win, title, (const char **)display_list, count, &cursor, SIGN_LEFT_ALIGN);

        // [결과 1] 취소 또는 상위 이동
        if (result == SIGN_CANCEL) {
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
        else if (result == SIGN_DELETE) {
            char target[1024];
            sprintf(target, "%s%s", current_view_path, raw_list[cursor]);

            // 우측 윈도우 하단 영역에 삭제 질문 가이드 노출
            mvwprintw(data_win, UI_Win_Height - 2, 2, ">> DELETE %s? (y/n)", raw_list[cursor]);
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

// 우측 윈도우 영역 안에서 동작하는 정보 디스플레이 (정렬 픽스 완료)
void ROSfunc_show_info(WINDOW *data_win) {
    werase(data_win);
    box(data_win, 0, 0);

    // 1. 헤더 (우측 분할 윈도우 가로폭 UI_Win_Width 기준 로컬 정렬 계산)
    char header_str[] = " === ROS STORAGE STATUS === ";
    UI_Center_x = (UI_Win_Width - (int)strlen(header_str)) / 2;
    UI_Center_y = (UI_Win_Height / 2) - 3;

    wattron(data_win, A_REVERSE);
    mvwprintw(data_win, UI_Center_y, UI_Center_x, "%s", header_str);
    wattroff(data_win, A_REVERSE);

    // 2. 루트 경로 표시
    char path_str[1024];
    sprintf(path_str, "Current Root: %s", ROS_PATH);
    UI_Center_x = (UI_Win_Width - (int)strlen(path_str)) / 2;
    UI_Center_y = (UI_Win_Height / 2) - 1;

    mvwprintw(data_win, UI_Center_y, UI_Center_x, "%s", path_str);

    // 3. 컨트롤 설명 표시
    char desc_str[] = "Control: [ENTER] to Enter DIR, [d] to Delete Any";
    UI_Center_x = (UI_Win_Width - (int)strlen(desc_str)) / 2;
    UI_Center_y = (UI_Win_Height / 2) + 1;

    mvwprintw(data_win, UI_Center_y, UI_Center_x, "%s", desc_str);

    // 4. 하단 안내문 (종료 가이드 매핑)
    char footer_str[] = "Press [ESC] or [Q] to return...";
    UI_Center_x = (UI_Win_Width - (int)strlen(footer_str)) / 2;
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
