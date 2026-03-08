#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include "SECTOR_MENU.h"
#include "EDSfunc.h"

// vim을 실행하는 함수
void EDSfunc_open_VIM(const char *file_path) {
    if (file_path == NULL) return;

    // 1. ncurses 화면 상태 저장 및 일시 정지
    def_prog_mode();      
    endwin();             

    // 2. vim 실행 (시스템 명령어)
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "vim '%s'", file_path);
    (void)system(cmd);

    // 3. ncurses 화면 복구
    reset_prog_mode();    
    refresh();            
    clear();              // 화면 갱신
}