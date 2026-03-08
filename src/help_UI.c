#include <ncurses.h>
#include <string.h>
#include "help_UI.h"



static void home_help(WINDOW *win) {
    mvwprintw(win, 3, 2, "HOME MENU");
    mvwprintw(win, 4, 2, "Select a system sector to manage.");
    mvwprintw(win, 5, 2, "Available: ROS, INS, EDS, BKS, TRS.");
}

static void ros_help(WINDOW *win) {
    mvwprintw(win, 3, 2, "ROS STORAGE MANAGEMENT");
    mvwprintw(win, 4, 2, "Manage archive files and logs.");
    mvwprintw(win, 5, 2, "[d] : Delete selected file.");
    mvwprintw(win, 6, 2, "[ENTER] : View file details.");
}

static void ins_help(WINDOW *win) {
    mvwprintw(win, 3, 2, "INS (Ingest Sector)");
    mvwprintw(win, 4, 2, "Monitor and ingest incoming data.");
    mvwprintw(win, 5, 2, "[ENTER] : Start ingestion process.");
    mvwprintw(win, 6, 2, "[s] : Stop current task.");
}

static void eds_help(WINDOW *win) {
    mvwprintw(win, 3, 2, "EDS (Edit Sector)");
    mvwprintw(win, 4, 2, "Modify and process data sets.");
    mvwprintw(win, 5, 2, "[ENTER] : Open editor.");
    mvwprintw(win, 6, 2, "[r] : Rename selected file.");
}

static void bks_help(WINDOW *win) {
    mvwprintw(win, 3, 2, "BKS (BackUp Storage)");
    mvwprintw(win, 4, 2, "Handle system backups and recovery.");
    mvwprintw(win, 5, 2, "[ENTER] : Initiate backup.");
    mvwprintw(win, 6, 2, "[v] : Verify backup integrity.");
}

static void trs_help(WINDOW *win) {
    mvwprintw(win, 3, 2, "TRS (Transfer Management)");
    mvwprintw(win, 4, 2, "Network data transmission settings.");
    mvwprintw(win, 5, 2, "[ENTER] : Sync data to remote host.");
    mvwprintw(win, 6, 2, "[c] : Configure connection.");
}

void SHOW_HELP(int sector_id) {

    WINDOW *win = newwin(12, 50, (LINES-12)/2, (COLS-50)/2);
        box(win, 0, 0);
        mvwprintw(win, 1, 2, "=== HELP MENU ===");

        // SECTOR_MENU.h에 정의된 상수를 그대로 활용
        switch(sector_id) {
            case HOME: home_help(win); break;
            case ROS:  ros_help(win);  break;
            case INS:  ins_help(win);  break;
            case EDS:  eds_help(win);  break;
            case BKS:  bks_help(win);  break;
            case TRS:  trs_help(win);  break;
            default:   mvwprintw(win, 3, 2, "No help available."); break;
        }

    mvwprintw(win, 10, 2, "ESC/Q to return.");
    wrefresh(win);

    int ch;
    while((ch = wgetch(win))) {
        if (ch == 27 || ch == 'q' || ch == 'Q') break;
    }
    
    delwin(win);
    clear();
    refresh();
}