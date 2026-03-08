#ifndef GIT_CMD_H
#define GIT_CMD_H

// 명령어 자체를 매크로로 정의하여 코드 내 중복 문자열 제거
#define GIT_INIT    "git init"
#define GIT_STATUS  "git status -s"
#define GIT_ADD_ALL "git add ."
#define GIT_COMMIT  "git commit"    /* -m 기능을 뺐으니 외부 편집기나 자체 편집기에서 커밋메세지 작성하게 하는 기능 필요 */
#define GIT_PUSH    "git push origin main"
#define GIT_PULL    "git pull origin main"
#define GIT_FETCH   "git fetch origin main"
#define GIT_LOG     "git log"
#define GIT_SWITCH  "git switch"

// 컴파일 시 기능 선택 스위치
#ifdef ENABLE_GIT_FEATURES
    #define EXEC_GIT(cmd) system(cmd)
#else
    #define EXEC_GIT(cmd) ((void)0) // 기능 끄면 아예 명령어가 실행되지 않음
#endif

#endif