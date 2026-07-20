#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "SECTOR_MENU.h"
#include "FILE_CHECK.h"
#include "FILE_SEARCH.h"
#include "FILE_UTIL.h"
#include "PATH_CONFIG.h"
#include "INSfunc.h"
#include "UI_PRINT.h"

// 리눅스 경로 최적화 버퍼 크기 정의
#define PATH_BUFFER_MAX 512

/* ── 내부 헬퍼 ─────────────────────────────────────────────────────────────
 *  파일 목록을 해제하는 단일 함수. 모든 함수에서 반복되던 for루프를 대체.
 */
static void free_list(char **list, int count) {
    for (int i = 0; i < count; i++) {
        free(list[i]);
        list[i] = NULL;         // dangling pointer 방지
    }
}

/* ── 내부 헬퍼 ─────────────────────────────────────────────────────────────
 *  상태바(하단 2번째 줄)에 메시지를 출력하고 키 입력을 대기 (전역 변수 UI_Win_Height 활용).
 */
static void status_msg(const char *msg) {
    move(UI_Win_Height - 2, 2); clrtoeol();
    mvprintw(UI_Win_Height - 2, 2, "%s", msg);
    refresh();
    getch();
}

/* ── 내부 헬퍼 ─────────────────────────────────────────────────────────────
 *  INGEST_PATH 파일 목록 스캔 + 빈 목록 처리를 한 곳에서 담당.
 *  반환값: 스캔된 파일 수 (0이면 메시지 출력 후 0 반환)
 */
static int scan_ingest(char **list, int max) {
    int count = FILE_ALL_LIST_GET(INGEST_PATH, list, max);
    if (count <= 0) {
        status_msg("[SYSTEM] No files found in Ingest Zone.");
    }
    return count;
}

/* ── 내부 헬퍼 ─────────────────────────────────────────────────────────────
 *  커서 범위 초과 보정 (파일이 삭제되어 목록이 줄었을 때 대비).
 */
static void clamp_cursor(int *cursor, int count) {
    if (*cursor >= count) *cursor = count - 1;
    if (*cursor < 0)      *cursor = 0;
}

/*===========================================================================
 *  INSfunc_handle_file_add — INGEST → ROS 복사 (덮어쓰기/중복 처리 포함)
 *==========================================================================*/
int INSfunc_handle_file_add() {
    static int sub_cursor = 0;
    char *file_list[100];

    while (1) {
        int file_count = scan_ingest(file_list, 100);
        if (file_count <= 0) { sub_cursor = 0; return 0; }

        clamp_cursor(&sub_cursor, file_count);

        int choice = SECTOR_MENU("SELECT FILE TO INGEST",
                                 (const char **)file_list, file_count,
                                 &sub_cursor, INS);

        if (choice == -3) {                         // [r] 새로고침
            free_list(file_list, file_count);
            mvprintw(UI_Win_Height - 1, 2, " >> List Updated. ");
            refresh(); napms(150);
            continue;
        }

        if (choice == -1) {                         // ESC / 뒤로
            free_list(file_list, file_count);
            return 0;
        }

        // 과도하게 할당된 버퍼 크기를 512 바이트로 축소 최적화
        char src[PATH_BUFFER_MAX], dest[PATH_BUFFER_MAX];
        snprintf(src,  sizeof(src),  "%s%s", INGEST_PATH, file_list[choice]);
        snprintf(dest, sizeof(dest), "%s%s", ROS_PATH,    file_list[choice]);

        move(UI_Win_Height - 2, 2); clrtoeol();

        if (FILE_EXISTENCE_CHECK(dest)) {
            if (FILE_DUPLICATE_CHECK(src, dest)) {
                mvprintw(UI_Win_Height - 2, 2, "CRITICAL: Identical file already exists!");
            } else {
                mvprintw(UI_Win_Height - 2, 2, FILE_COPY(src, dest)
                    ? "SUCCESS: File Overwritten."
                    : "ERROR: Overwrite failed.");
            }
        } else {
            mvprintw(UI_Win_Height - 2, 2, FILE_COPY(src, dest)
                ? "SUCCESS: File copied to ROS."
                : "ERROR: Copy failed.");
        }

        refresh(); getch();
        free_list(file_list, file_count);
    }
}

/*===========================================================================
 *  INSfunc_list — INGEST 파일 목록 보기 (읽기 전용)
 *==========================================================================*/
int INSfunc_list() {
    static int list_cursor = 0;
    char *temp_list[100];

    while (1) {
        int count = scan_ingest(temp_list, 100);
        if (count <= 0) return 0;

        clamp_cursor(&list_cursor, count);

        int choice = SECTOR_MENU("CURRENT INGEST FILES (View Only)",
                                 (const char **)temp_list, count,
                                 &list_cursor, INS);

        free_list(temp_list, count);

        if (choice == -1) return 0;     // ESC
    }
}

/*===========================================================================
 *  INS_copy_to_sector — INGEST → 지정 섹터 복사 (EDS / BKS 공용)
 *==========================================================================*/
int INS_copy_to_sector(const char *dest_path, const char *sector_name) {
    static int copy_cursor = 0;
    char *temp_list[100];

    while (1) {
        int count = scan_ingest(temp_list, 100);
        if (count <= 0) return 0;

        clamp_cursor(&copy_cursor, count);

        char title[64];
        snprintf(title, sizeof(title), "COPY TO %s - SELECT FILE", sector_name);

        int choice = SECTOR_MENU(title, (const char **)temp_list, count,
                                 &copy_cursor, INS);

        if (choice == -3) {
            free_list(temp_list, count);
            continue;
        }

        if (choice == -1) {
            free_list(temp_list, count);
            return 0;
        }

        // 버퍼 다이어트 완료
        char src[PATH_BUFFER_MAX], dest[PATH_BUFFER_MAX];
        snprintf(src,  sizeof(src),  "%s%s", INGEST_PATH, temp_list[choice]);
        snprintf(dest, sizeof(dest), "%s%s", dest_path,   temp_list[choice]);

        status_msg(FILE_COPY(src, dest)
            ? "SUCCESS: File sent."
            : "ERROR: Copy failed.");

        free_list(temp_list, count);
    }
}

/* 래퍼 함수 */
int INS_copy_to_eds() { return INS_copy_to_sector(EDS_PATH, "EDS"); }
int INS_copy_to_bks() { return INS_copy_to_sector(BKS_PATH, "BKS"); }

/*===========================================================================
 *  INS_quick_duplicate_check — ROS 내 중복 존재 여부 스캔
 *==========================================================================*/
int INS_quick_duplicate_check() {
    static int scan_cursor = 0;
    char *temp_list[100];

    while (1) {
        int count = scan_ingest(temp_list, 100);
        if (count <= 0) return 0;

        clamp_cursor(&scan_cursor, count);

        int choice = SECTOR_MENU("DUPLICATE SCAN - SELECT FILE",
                                 (const char **)temp_list, count,
                                 &scan_cursor, INS);

        if (choice == -3) {
            free_list(temp_list, count);
            continue;
        }

        if (choice == -1) {
            free_list(temp_list, count);
            return 0;
        }

        // 버퍼 다이어트 완료
        char dest[PATH_BUFFER_MAX];
        snprintf(dest, sizeof(dest), "%s%s", ROS_PATH, temp_list[choice]);

        status_msg(FILE_EXISTENCE_CHECK(dest)
            ? "RESULT: Found in ROS."
            : "RESULT: Unique (Not in ROS).");

        free_list(temp_list, count);
    }
}
