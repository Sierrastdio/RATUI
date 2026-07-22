#ifndef INSFUNC_H
#define INSFUNC_H
#include <ncurses.h>
int INSfunc_handle_file_add(WINDOW *data_win);
int INSfunc_list(WINDOW *data_win);
int INS_quick_duplicate_check(WINDOW *data_win);
/* 섹터 간 복사 공용 로직 (ROS 등 타 모듈에서 재사용 가능)
 * [수정] src_path 인자 추가: 어느 경로를 스캔/복사할지 호출부에서 명시.
 * 이전에는 함수 내부에서 INGEST_PATH가 고정되어 있어 COPY TO EDS/BKS도
 * 잘못 INGEST 목록을 보여주는 문제가 있었음. */
int INS_copy_to_sector(WINDOW *data_win, const char *src_path, const char *dest_path, const char *sector_name);
int INS_copy_to_eds(WINDOW *data_win);
int INS_copy_to_bks(WINDOW *data_win);
int INS_copy_to_trs(void);
#endif
