#include <ncurses.h>
#include "SECTOR_MENU.h"
#include "EDSfunc.h"

void EDS_MAIN_LOOP() {
    static const char *eds_items[] = {
        "[1] 앙 기모띠",
        "[2]",
        "[3]",
        "[BACK] To Home",
    };
    const int eds_count = (int)(sizeof(eds_items) / sizeof(eds_items[0]));
    int eds_cursor = 0;

    while(1) {
        int result = SECTOR_MENU("EDS (Edit Sector )", eds_items, eds_count, &eds_cursor, EDS);

        if(result == SIGN_CANCEL) return;

        switch(result) {
            case 0:
                break;

            case 1:
                break;

            case 2:
                break;

            case 3:
                return;
        }
    }
}
