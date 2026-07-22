#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "SECTOR_MENU.h"
#include "FILE_CHECK.h"
#include "FILE_SEARCH.h"
#include "FILE_UTIL.h"
#include "PATH_CONFIG.h"
#include "INSfunc.h"
#include "UI_PRINT.h"

#define PATH_BUFFER_MAX 512

/* ── 내부 헬퍼 ─────────────────────────────────────────────────────────────
 *  통짜 메모리 블록 단 1회 해제 구조.
 *  list[0]이 전체 동적 버퍼의 시작점(Header) 역할을 하므로 list[0]만 free한다.
 */
static void free_list(char **list, int count) {
    if (list && count > 0 && list[0]) {
        free(list[0]);
        for (int i = 0; i < count; i++) {
            list[i] = NULL;
        }
    }
}

/* ── 내부 헬퍼 ─────────────────────────────────────────────────────────────
 *  상태바 메시지 출력 후 키 대기
 */
static void status_msg(WINDOW *win, const char *msg) {
    if (win == NULL) return;

    mvwprintw(win, UI_Win_Height - 2, 2, "%-*s", UI_Win_Width - 4, msg);
    wrefresh(win);
    wgetch(win);
}

/* ── 내부 헬퍼 ─────────────────────────────────────────────────────────────
 *  지정한 경로(src_path)를 스캔하여 파일 목록 반환
 */
static int scan_source(WINDOW *win, char **list, int max, const char *src_path, const char *label) {
    int count = FILE_ALL_LIST_GET(src_path, list, max);
    if (count <= 0) {
        char msg[128];
        snprintf(msg, sizeof(msg), "[SYSTEM] No files found in %s.", label);

        UI_CLEAR_WINDOW(win);
        UI_PRINT_CENTERED(win, UI_GET_WIN_CENTER_Y(0), msg);
        UI_PRINT_CENTERED(win, UI_Win_Height - 2, "Press [q/ESC] to return...");
        wrefresh(win);
        wgetch(win);
    }
    return count;
}

/* ── 내부 헬퍼 ─────────────────────────────────────────────────────────────
 *  경로가 디렉토리인지 확인
 */
static int is_directory(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode);
}

/* ── 내부 헬퍼 ─────────────────────────────────────────────────────────────
 *  [포인터 최적화]
 *  통짜 할당된 포인터 배열을 in-place(제자리)로 포인터만 스왑/압축하여
 *  추가 free/malloc 없이 디렉토리를 필터링함.
 */
static int filter_out_directories(char **list, int count, const char *base_path) {
    int write_idx = 0;
    char full[PATH_BUFFER_MAX];

    for (int i = 0; i < count; i++) {
        snprintf(full, sizeof(full), "%s%s", base_path, list[i]);

        if (is_directory(full)) {
            continue;
        }

        if (write_idx != i) {
            list[write_idx] = list[i];
        }
        write_idx++;
    }
    return write_idx;
}

/* ── 내부 헬퍼 ─────────────────────────────────────────────────────────────
 *  커서 범위 초과 보정
 */
static void clamp_cursor(int *cursor, int count) {
    if (*cursor >= count) *cursor = count - 1;
    if (*cursor < 0)      *cursor = 0;
}

/*===========================================================================
 *  INSfunc_handle_file_add — INGEST → ROS 복사
 *==========================================================================*/
int INSfunc_handle_file_add(WINDOW *data_win) {
    static int sub_cursor = 0;
    char *file_list[100] = {0};

    while (1) {
        int file_count = scan_source(data_win, file_list, 100, INGEST_PATH, "Ingest Zone");
        if (file_count <= 0) { sub_cursor = 0; return 0; }

        clamp_cursor(&sub_cursor, file_count);

        int choice = SECTOR_MENU_WIN(data_win, "SELECT FILE TO INGEST",
                                     (const char **)file_list, file_count,
                                     &sub_cursor, SIGN_LEFT_ALIGN);

        if (choice == SIGN_KEY_CHANGED) {
            free_list(file_list, file_count);
            continue;
        }

        if (choice == SIGN_REFRESH) {
            free_list(file_list, file_count);
            mvwprintw(data_win, UI_Win_Height - 2, 2, " >> List Updated. ");
            wrefresh(data_win);
            napms(150);
            continue;
        }

        if (choice == SIGN_CANCEL) {
            free_list(file_list, file_count);
            return 0;
        }

        char src[PATH_BUFFER_MAX], dest[PATH_BUFFER_MAX];
        snprintf(src,  sizeof(src),  "%s%s", INGEST_PATH, file_list[choice]);
        snprintf(dest, sizeof(dest), "%s%s", ROS_PATH,    file_list[choice]);

        if (FILE_EXISTENCE_CHECK(dest)) {
            if (FILE_DUPLICATE_CHECK(src, dest)) {
                status_msg(data_win, "CRITICAL: Identical file already exists!");
            } else {
                status_msg(data_win, FILE_COPY(src, dest)
                    ? "SUCCESS: File Overwritten."
                    : "ERROR: Overwrite failed.");
            }
        } else {
            status_msg(data_win, FILE_COPY(src, dest)
                ? "SUCCESS: File copied to ROS."
                : "ERROR: Copy failed.");
        }

        free_list(file_list, file_count);
    }
}

/*===========================================================================
 *  INSfunc_list — INGEST 파일 목록 보기 (읽기 전용)
 *==========================================================================*/
int INSfunc_list(WINDOW *data_win) {
    static int list_cursor = 0;
    char *temp_list[100] = {0};

    while (1) {
        int count = scan_source(data_win, temp_list, 100, INGEST_PATH, "Ingest Zone");
        if (count <= 0) return 0;

        clamp_cursor(&list_cursor, count);

        int choice = SECTOR_MENU_WIN(data_win, "CURRENT INGEST FILES (View Only)",
                                     (const char **)temp_list, count,
                                     &list_cursor, SIGN_LEFT_ALIGN);

        free_list(temp_list, count);

        if (choice == SIGN_CANCEL) return 0;
    }
}

/*===========================================================================
 *  INS_copy_to_sector — 지정 경로 → 지정 섹터 복사 (EDS / BKS 공용)
 *==========================================================================*/
int INS_copy_to_sector(WINDOW *data_win, const char *src_path, const char *dest_path, const char *sector_name) {
    static int copy_cursor = 0;
    char *temp_list[100] = {0};

    while (1) {
        int count = scan_source(data_win, temp_list, 100, src_path, "Storage");
        if (count <= 0) return 0;

        count = filter_out_directories(temp_list, count, src_path);

        if (count <= 0) {
            free_list(temp_list, 100);
            UI_CLEAR_WINDOW(data_win);
            UI_PRINT_CENTERED(data_win, UI_GET_WIN_CENTER_Y(0), "[SYSTEM] No copyable files (only folders) found.");
            UI_PRINT_CENTERED(data_win, UI_Win_Height - 2, "Press [q/ESC] to return...");
            wrefresh(data_win);
            wgetch(data_win);
            return 0;
        }

        clamp_cursor(&copy_cursor, count);

        char title[64];
        snprintf(title, sizeof(title), "COPY TO %s - SELECT FILE", sector_name);

        int choice = SECTOR_MENU_WIN(data_win, title, (const char **)temp_list, count,
                                     &copy_cursor, SIGN_LEFT_ALIGN);

        if (choice == SIGN_KEY_CHANGED || choice == SIGN_REFRESH) {
            free_list(temp_list, count);
            continue;
        }

        if (choice == SIGN_CANCEL) {
            free_list(temp_list, count);
            return 0;
        }

        char src[PATH_BUFFER_MAX], dest[PATH_BUFFER_MAX];
        snprintf(src,  sizeof(src),  "%s%s", src_path,  temp_list[choice]);
        snprintf(dest, sizeof(dest), "%s%s", dest_path, temp_list[choice]);

        status_msg(data_win, FILE_COPY(src, dest)
            ? "SUCCESS: File sent."
            : "ERROR: Copy failed.");

        free_list(temp_list, count);
    }
}

int INS_copy_to_eds(WINDOW *data_win) { return INS_copy_to_sector(data_win, INGEST_PATH, EDS_PATH, "EDS"); }
int INS_copy_to_bks(WINDOW *data_win) { return INS_copy_to_sector(data_win, INGEST_PATH, BKS_PATH, "BKS"); }

/*===========================================================================
 *  INS_quick_duplicate_check — ROS 내 중복 존재 여부 스캔
 *==========================================================================*/
int INS_quick_duplicate_check(WINDOW *data_win) {
    static int scan_cursor = 0;
    char *temp_list[100] = {0};

    while (1) {
        int count = scan_source(data_win, temp_list, 100, INGEST_PATH, "Ingest Zone");
        if (count <= 0) return 0;

        clamp_cursor(&scan_cursor, count);

        int choice = SECTOR_MENU_WIN(data_win, "DUPLICATE SCAN - SELECT FILE",
                                     (const char **)temp_list, count,
                                     &scan_cursor, SIGN_LEFT_ALIGN);

        if (choice == SIGN_KEY_CHANGED || choice == SIGN_REFRESH) {
            free_list(temp_list, count);
            continue;
        }

        if (choice == SIGN_CANCEL) {
            free_list(temp_list, count);
            return 0;
        }

        char dest[PATH_BUFFER_MAX];
        snprintf(dest, sizeof(dest), "%s%s", ROS_PATH, temp_list[choice]);

        status_msg(data_win, FILE_EXISTENCE_CHECK(dest)
            ? "RESULT: Found in ROS."
            : "RESULT: Unique (Not in ROS).");

        free_list(temp_list, count);
    }
}
