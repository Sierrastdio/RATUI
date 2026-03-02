#include <ncurses.h>
#include <stdlib.h>      
#include "SECTOR_MENU.h"
#include "INSfunc.h"    
#include "FILE_CHECK.h"
#include "FILE_SEARCH.h"

void INS_MAIN_LOOP() {
    // 1. INS 섹션 메뉴 항목
    const char *ins_items[] = {
        "[1] File add", 
        "[2] INS File List View", 
        "[3] Copy to EDS", 
        "[4] Clone to BackUp(BKS)",
        "[5] Check for File Duplication",
        "[BACK] To Home"
    };
    int ins_count = 6;
    int ins_cursor = 0; 

    char *temp_list[100];

    while (1) {
        int result = SECTOR_MENU("INS(Ingest Sector) MANAGEMENT", ins_items, ins_count, &ins_cursor);

        if (result >= 0) {
            switch(result) {
                case 0:
                    handle_file_add();
                    break;

                case 1:
                {
                    int count = FILE_ALL_LIST_GET("./ingest_zone/", temp_list, 100);
                    if (count > 0) {
                        int tmp_cursor = 0;
                        // 경고 해결: (const char**) 캐스팅을 제거하거나 
                        // SECTOR_MENU.h와 타입을 완전히 일치시킵니다.
                        SECTOR_MENU("CURRENT INGEST FILES", (const char**)temp_list, count, &tmp_cursor);
                        
                        for(int i = 0; i < count; i++) {
                            free(temp_list[i]);
                        }
                    } else {
                        mvprintw(LINES-2, 2, "No files found in Ingest Zone.");
                        refresh();
                        getch();
                    }
                }
                break;

                case 2:
                    // EDS 클론 (추후 구현)
                    break;

                case 3: 
                    // BKS 클론 (추후 구현)
                    break;

                case 4:
                    mvprintw(LINES-2, 2, "Check Duplicate: Logic pending.");
                    refresh();
                    getch();
                    break;

                case 5:
                    return; // HOME으로 복귀
            }
        }
    }
}