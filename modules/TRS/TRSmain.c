#include <ncurses.h>
#include "SECTOR_MENU.h"
#include "TRSfunc.h"

void TRS_MAIN_LOOP() {
    const char *trs_items[] = {
        "[1] ",
        "[2] ",
        "[3] ",
        "[BACK] To Home"
    };
    int trs_count = 4;
    int trs_cursor = 0;

    while(1) {
        int result = SECTOR_MENU("TRS STORAGE MANAGEMENT", trs_items, trs_count, &trs_cursor, TRS);

        if (result == SIGN_CANCEL) return;

        switch(result) {
            case 0:  break;
            case 1:  break;
            case 2:  break;
            case 3: return;
        }
    }

}
