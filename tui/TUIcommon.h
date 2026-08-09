#ifndef TUICOMMON_H
#define TUICOMMON_H

#include <ncurses.h>
#include "core.h"

void STATUS_MSG(WINDOW *win, const char *msg);
int  PROMPT_LINE(WINDOW *win, int y, int x, const char *label, char *out, int out_cap);
int  TUI_TAG_ASSIGN(WINDOW *data_win, archdb_t *db, const char *full_path, const char *file_name);
void TUI_UNTAG_FILE(WINDOW *data_win, archdb_t *db, uint32_t file_id, const char *file_name);
void TUI_UNREGISTER_FILE(WINDOW *data_win, archdb_t *db, uint32_t file_id, const char *file_name);

#endif
