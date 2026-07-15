#include "UI_PRINT.h"
#include <ncurses.h>

// 주어진 텍스트 길이를 기준으로 가로 중앙 정렬 시작 X 좌표를 반환
int UI_GET_CENTER_X(int x, int textlen)
{
    // textlen이 0 이하인 경우 예외 처리
    if (textlen <= 0)
        return 0;

    // 터미널 가로 크기(COLS)보다 텍스트가 길면 맨 왼쪽(0)부터 출력하도록 예외 처리
    if (textlen >= COLS)
        return 0;

    x = (COLS - textlen) / 2;
    return x;
}
