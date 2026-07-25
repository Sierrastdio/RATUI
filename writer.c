#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <dirent.h>

#include "SECTOR_MENU.h"
#include "FILE_CHECK.h"
#include "FILE_SEARCH.h"
#include "FILE_UTIL.h"
#include "PATH_CONFIG.h"
#include "UI_PRINT.h"

#define PATH_BUFFER_MAX 512
#define TARGET_DB_PATH  "files.db"
#define MAX_FILE_COUNT  100

/* ───────────────────────────────────────────────────────────────────────────
 * UI 크기 변수 (UI_PRINT.o에 정의된 전역 변수 참조)
 * ─────────────────────────────────────────────────────────────────────────── */
extern int UI_Win_Height;
extern int UI_Win_Width;

/* ───────────────────────────────────────────────────────────────────────────
 * 138바이트 고정 DB 레코드 구조체
 * ─────────────────────────────────────────────────────────────────────────── */
#pragma pack(push, 1)
typedef struct {
    char     tag_id[4];     /* 4바이트 영문 태그 (예: "DOCS", "IMG ") */
    uint32_t file_id;       /* 4바이트 파일 ID */
    char     file_name[128];/* 128바이트 고정 파일명 */
    uint16_t version;       /* 2바이트 파일 버전 */
} file_record_t;
#pragma pack(pop)

/* ───────────────────────────────────────────────────────────────────────────
 * 내부 헬퍼 함수들
 * ─────────────────────────────────────────────────────────────────────────── */

/* 메모리 해제 루틴 */
static void free_list(char **list, int count) {
    if (!list) return;

    for (int i = 0; i < MAX_FILE_COUNT; i++) {
        if (list[i] != NULL) {
            free(list[i]);
            list[i] = NULL;
        }
    }
}

/* 상태바 메시지 출력 후 키 대기 */
static void status_msg(WINDOW *win, const char *msg) {
    if (win == NULL) return;

    int print_y = (UI_Win_Height > 2) ? UI_Win_Height - 2 : 0;
    int print_w = (UI_Win_Width > 4) ? UI_Win_Width - 4 : 10;

    mvwprintw(win, print_y, 2, "%-*s", print_w, msg);
    wrefresh(win);
    wgetch(win);
}

/* 지정 디렉토리를 직접 읽어 일반 파일 목록 수집 */
static int read_dir_files(const char *dir_path, char **list, int max) {
    if (!dir_path) return 0;

    DIR *d = opendir(dir_path);
    if (!d) return 0;

    struct dirent *dir;
    int count = 0;

    while ((dir = readdir(d)) != NULL && count < max) {
        /* 숨김 파일, ., .. 제외 */
        if (dir->d_name[0] == '.') {
            continue;
        }

        char full_path[PATH_BUFFER_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, dir->d_name);

        struct stat st;
        if (stat(full_path, &st) == 0) {
            /* 디렉토리가 아닌 일반 파일인 경우 추가 */
            if (S_ISREG(st.st_mode)) {
                list[count] = strdup(dir->d_name);
                if (list[count] != NULL) {
                    count++;
                }
            }
        }
    }
    closedir(d);
    return count;
}

/* 다중 경로 탐색 및 스캔 (INS 및 INGEST_PATH 우선 탐색) */
static int scan_source(WINDOW *win, char **list, int max, const char *src_path, const char *label) {
    for (int i = 0; i < max; i++) {
        list[i] = NULL;
    }

    int count = 0;
    char found_path[PATH_BUFFER_MAX] = {0};

    /* 1. PATH_CONFIG.h의 INGEST_PATH 매크로 경로 탐색 */
    if (src_path != NULL && strlen(src_path) > 0) {
        count = read_dir_files(src_path, list, max);
        if (count > 0) {
            strncpy(found_path, src_path, sizeof(found_path) - 1);
        }
    }

    /* 2. 명시적 "INS" 및 "./INS" 탐색 */
    if (count <= 0) {
        count = read_dir_files("INS", list, max);
        if (count > 0) {
            strncpy(found_path, "INS", sizeof(found_path) - 1);
        }
    }
    if (count <= 0) {
        count = read_dir_files("./INS", list, max);
        if (count > 0) {
            strncpy(found_path, "./INS", sizeof(found_path) - 1);
        }
    }

    /* 3. 상위 디렉토리 "../INS" 탐색 (bin/ build/ 실행 시 대응) */
    if (count <= 0) {
        count = read_dir_files("../INS", list, max);
        if (count > 0) {
            strncpy(found_path, "../INS", sizeof(found_path) - 1);
        }
    }

    /* 4. 기존 라이브러리 함수 호출 시도 (Fallback) */
    if (count <= 0 && src_path != NULL) {
        count = FILE_ALL_LIST_GET(src_path, list, max);
    }

    /* 파일을 전혀 찾지 못한 경우 오류 출력 */
    if (count <= 0) {
        char msg[256];
        snprintf(msg, sizeof(msg), "[SYSTEM] No files found in '%s' (Checked: INS, %s, ./INS, ../INS)", 
                 label, (src_path ? src_path : "NULL"));

        UI_CLEAR_WINDOW(win);
        UI_PRINT_CENTERED(win, UI_GET_WIN_CENTER_Y(0), msg);
        UI_PRINT_CENTERED(win, (UI_Win_Height > 2) ? UI_Win_Height - 2 : 1, "Press [q/ESC] to return...");
        wrefresh(win);
        wgetch(win);
        return 0;
    }

    if (count > max) {
        count = max;
    }

    return count;
}

/* 커서 범위 초과 보정 */
static void clamp_cursor(int *cursor, int count) {
    if (count <= 0) {
        *cursor = 0;
        return;
    }
    if (*cursor >= count) *cursor = count - 1;
    if (*cursor < 0)      *cursor = 0;
}

/* DB 파일에서 다음 Auto-Increment file_id 가져오기 */
static uint32_t get_next_file_id(const char *db_path) {
    FILE *fp = fopen(db_path, "rb");
    if (!fp) return 1001;

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fclose(fp);

    if (file_size < 0) return 1001;

    uint32_t count = (uint32_t)(file_size / sizeof(file_record_t));
    return 1001 + count;
}

/* DB 파일 끝에 138바이트 고정 레코드 추가 */
static int append_record_to_db(const char *db_path, const file_record_t *rec) {
    FILE *fp = fopen(db_path, "ab");
    if (!fp) return -1;

    size_t written = fwrite(rec, sizeof(file_record_t), 1, fp);
    fclose(fp);

    return (written == 1) ? 0 : -1;
}

/* 사용자로부터 4바이트 영문 전용 TAG ID 및 버전 입력받기 */
static int prompt_tag_and_version(WINDOW *win, char *out_tag, uint16_t *out_version) {
    char tag_buf[16] = {0};
    char ver_buf[16] = {0};

    echo();
    curs_set(1);

    /* 1. Tag ID 입력 받기 */
    while (1) {
        UI_CLEAR_WINDOW(win);
        mvwprintw(win, 4, 4, "[ TAG REGISTRATION ]");
        mvwprintw(win, 6, 4, "Enter 4-letter English TAG ID (e.g. DOCS, CODE, IMG): ");
        wrefresh(win);

        mvwgetnstr(win, 6, 56, tag_buf, 4);

        int valid = 1;
        size_t len = strlen(tag_buf);
        if (len == 0) {
            noecho();
            curs_set(0);
            return -1;
        }

        for (size_t i = 0; i < len; i++) {
            if ((tag_buf[i] >= 'a' && tag_buf[i] <= 'z')) {
                tag_buf[i] -= 32;
            } else if (tag_buf[i] >= 'A' && tag_buf[i] <= 'Z') {
                /* 정상 대문자 */
            } else {
                valid = 0;
                break;
            }
        }

        if (valid) {
            memset(out_tag, ' ', 4);
            memcpy(out_tag, tag_buf, len);
            break;
        } else {
            mvwprintw(win, 8, 4, "[ERROR] Only English letters (A-Z, a-z) are allowed! Press key...");
            wrefresh(win);
            wgetch(win);
        }
    }

    /* 2. Version 입력 받기 */
    while (1) {
        mvwprintw(win, 8, 4, "Enter File Version (Numeric, e.g. 1): ");
        clrtoeol();
        wrefresh(win);

        mvwgetnstr(win, 8, 42, ver_buf, 5);
        if (strlen(ver_buf) == 0) {
            *out_version = 1;
            break;
        }

        int ver_num = atoi(ver_buf);
        if (ver_num > 0 && ver_num <= 65535) {
            *out_version = (uint16_t)ver_num;
            break;
        } else {
            mvwprintw(win, 10, 4, "[ERROR] Invalid version number! Press key...");
            wrefresh(win);
            wgetch(win);
            mvwprintw(win, 10, 4, "%-*s", (UI_Win_Width > 8) ? UI_Win_Width - 8 : 10, "");
        }
    }

    noecho();
    curs_set(0);
    return 0;
}

/* ───────────────────────────────────────────────────────────────────────────
 * 태그 등록 핸들러 함수
 * ─────────────────────────────────────────────────────────────────────────── */
int WRITER_handle_tag_register(WINDOW *data_win) {
    static int writer_cursor = 0;
    char *file_list[MAX_FILE_COUNT];

    for (int i = 0; i < MAX_FILE_COUNT; i++) {
        file_list[i] = NULL;
    }

    while (1) {
        int file_count = scan_source(data_win, file_list, MAX_FILE_COUNT, INGEST_PATH, "INS Zone");
        if (file_count <= 0) { 
            writer_cursor = 0; 
            free_list(file_list, file_count);
            return 0; 
        }

        clamp_cursor(&writer_cursor, file_count);

        int choice = SECTOR_MENU_WIN(data_win, "SELECT FILE TO REGISTER TAG",
                                     (const char **)file_list, file_count,
                                     &writer_cursor, SIGN_LEFT_ALIGN);

        if (choice == SIGN_KEY_CHANGED) {
            free_list(file_list, file_count);
            continue;
        }

        if (choice == SIGN_REFRESH) {
            free_list(file_list, file_count);
            mvwprintw(data_win, (UI_Win_Height > 2) ? UI_Win_Height - 2 : 0, 2, " >> List Updated. ");
            wrefresh(data_win);
            napms(150);
            continue;
        }

        if (choice == SIGN_CANCEL || choice < 0 || choice >= file_count) {
            free_list(file_list, file_count);
            return 0;
        }

        /* 1. 선택된 파일명 추출 */
        char selected_file[128] = {0};
        if (file_list[choice] != NULL) {
            strncpy(selected_file, file_list[choice], sizeof(selected_file) - 1);
        }

        /* 2. 태그 및 버전 입력받기 */
        char tag_id[4] = {0};
        uint16_t version = 1;
        if (prompt_tag_and_version(data_win, tag_id, &version) != 0) {
            status_msg(data_win, "CANCELLED: Tag registration aborted.");
            free_list(file_list, file_count);
            continue;
        }

        /* 3. DB 레코드 생성 (138바이트 고정 규격) */
        file_record_t rec;
        memset(&rec, 0, sizeof(file_record_t));

        memcpy(rec.tag_id, tag_id, 4);
        rec.file_id = get_next_file_id(TARGET_DB_PATH);
        strncpy(rec.file_name, selected_file, sizeof(rec.file_name) - 1);
        rec.version = version;

        /* 4. DB 파일에 저장 */
        if (append_record_to_db(TARGET_DB_PATH, &rec) == 0) {
            char result_msg[256];
            snprintf(result_msg, sizeof(result_msg), 
                     "SUCCESS: Tag '%.4s' Registered to %s (ID: %u, v%u)", 
                     rec.tag_id, rec.file_name, rec.file_id, rec.version);
            status_msg(data_win, result_msg);
        } else {
            status_msg(data_win, "ERROR: Failed to write to DB file.");
        }

        free_list(file_list, file_count);
    }
}

/* ───────────────────────────────────────────────────────────────────────────
 * 메인 실행 함수 (ncurses 초기화 및 프로그램 엔트리 포인트)
 * ─────────────────────────────────────────────────────────────────────────── */
int main(void) {
    /* ncurses 환경 초기화 */
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);

    /* 실제 터미널 화면 크기 감지 및 UI_PRINT 전역 변수 설정 */
    getmaxyx(stdscr, UI_Win_Height, UI_Win_Width);

    /* 화면 전체를 덮는 main_win 생성 */
    WINDOW *main_win = newwin(UI_Win_Height, UI_Win_Width, 0, 0);
    if (main_win == NULL) {
        endwin();
        fprintf(stderr, "Error: Failed to create ncurses window.\n");
        return 1;
    }

    box(main_win, 0, 0);
    wrefresh(main_win);

    /* 태그 등록 함수 실행 */
    WRITER_handle_tag_register(main_win);

    /* ncurses 자원 안전하게 해제 */
    delwin(main_win);
    endwin();

    return 0;
}