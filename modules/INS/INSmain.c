#include <ncurses.h>
#include "SECTOR_MENU.h"
#include "INSfunc.h" // INSfunc.c에 있는 모든 함수를 여기서 호출

void INS_MAIN_LOOP() {
    const char *ins_items[] = {
        "[1] File add to ROS", 
        "[2] INS File List View", 
        "[3] Copy to EDS", 
        "[4] Clone to BackUp(BKS)",
        "[5] Check for File Duplication",
        "[BACK] To Home"
    };
    int ins_count = 6;
    int ins_cursor = 0; 

    while (1) {
        int result = SECTOR_MENU("INS(Ingest Sector) MANAGEMENT", ins_items, ins_count, &ins_cursor);

        if (result == -1) return;

        switch(result) {
            case 0: INSfunc_handle_file_add(); break;
            case 1: INSfunc_list(); break;
            case 2: INS_copy_to_eds(); break;
            case 3: INS_copy_to_bks(); break;
            case 4: INS_quick_duplicate_check(); break;
            case 5: return;
        }
    }
}