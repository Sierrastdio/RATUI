#ifndef INSFUNC_H
#define INSFUNC_H

#include <ncurses.h>

int INSfunc_handle_file_add(void);
int INSfunc_list(void);
int INS_quick_duplicate_check(void);

/* 섹터 간 복사 공용 로직 (ROS 등 타 모듈에서 재사용 가능) */
int INS_copy_to_sector(const char *dest_path, const char *sector_name);
int INS_copy_to_eds(void);
int INS_copy_to_bks(void);
int INS_copy_to_trs(void);

#endif
