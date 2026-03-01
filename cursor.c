#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

// 터미널 설정을 변경하여 엔터 키 없이 입력을 받도록 함
void set_conio_mode() {
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag &= ~(ICANON | ECHO); // 버퍼링과 에코(입력값 표시) 끔
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

int main() {
    int x = 10, y = 5; // 시작 좌표
    char ch;

    set_conio_mode(); // 입력 모드 설정
    printf("\e[?25l"); // 커서 숨기기

    while (1) {
        // 1. 화면 지우기 및 위치 초기화
        printf("\e[2J"); 
        
        // 2. 특정 위치(y, x)로 이동하여 문자 출력
        printf("\e[%d;%dH@", y, x);
        
        // 3. 안내 메시지 출력
        printf("\e[1;1H[W,A,S,D]: Move | [Q]: Quit");
        fflush(stdout);

        // 4. 키 입력 받기
        read(STDIN_FILENO, &ch, 1);

        if (ch == 'q' || ch == 'Q') break;
        
        // 5. 좌표 업데이트
        switch (ch) {
            case 'w': case 'W': if (y > 2) y--; break;
            case 's': case 'S': y++; break;
            case 'a': case 'A': if (x > 1) x--; break;
            case 'd': case 'D': x++; break;
        }
    }

    // 프로그램 종료 전 복구
    printf("\e[?25h\e[2J\e[1;1HProgram Exit.\n");
    return 0;
}