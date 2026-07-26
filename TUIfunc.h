#ifndef TUIFUNC_H
#define TUIFUNC_H

#include <ncurses.h>
#include "core.h"

/* [1] Search Tag 핸들러
 * 가상 경로("DOCS/IMG")를 입력받아 core_list_by_path로 검색하고 결과를 출력.
 * 성공 0 / 실패(취소 포함) -1 */
int TUI_handle_search_tag(WINDOW *data_win, archdb_t *db);

/* [2] Browse (Folder View) 핸들러
 * 태그를 폴더처럼 취급해서 파일 탐색기 스타일로 드릴다운하며 탐색.
 * ENTER: 하위 태그로 진입 / 파일 선택 시 상세정보 표시
 * ESC 또는 '..' 선택: 상위 폴더로. 최상위에서 취소하면 핸들러 종료. */
int TUI_handle_browse(WINDOW *data_win, archdb_t *db);

/* [3] Browse Filesystem 핸들러
 * config.ratui 의 ARCHIVE_HOME_PATH 를 시작점으로 실제 절대경로 디렉토리를 탐색.
 * 홈 경로 밖으로는 못 나가게 제한한다.
 * 파일에서 ENTER: 정보만 표시. 't': 그 파일에 태그 할당(버전+태그 입력받아 core_register_file 호출).
 * 태그 수정/삭제는 나중에 여기 연결 예정. */
int TUI_handle_browse_fs(WINDOW *data_win, archdb_t *db);

#endif /* TUIFUNC_H */