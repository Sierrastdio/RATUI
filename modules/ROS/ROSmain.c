#include <ncurses.h>
#include "SECTOR_MENU.h"
#include "ROSfunc.h"

void ROS_MAIN_LOOP() {
    const char *ros_items[] = {
        "[1] Storage Info & Status",
        "[2] Manage Stored Files (View/Delete)",
        "[3] Copy to EDS",
        "[BACK] To Home"
    };
    int ros_count = 4;
    int ros_cursor = 0;

    while(1) {
        int result = SECTOR_MENU("ROS STORAGE MANAGEMENT", ros_items, ros_count, &ros_cursor, ROS);

        if (result == SIGN_CANCEL) return;

        switch(result) {
            case 0: ROSfunc_show_info(); break;
            case 1: ROSfunc_manage_storage(); break;
            case 2: /* EDS 전송 로직 예정 */ break;
            case 3: return;
        }
    }
}
