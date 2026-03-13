#include <stdlib.h>
#include <stdio.h>
#include "GIT_CMD.h"

void DO_GIT_SYNC(const char *msg) {
    // 메시지가 있을 때만 커밋 수행 (메시지가 없으면 push만)
    if (msg) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "%s && %s '%s' && %s", GIT_ADD_ALL, GIT_COMMIT, msg, GIT_PUSH);
        EXEC_GIT(cmd);
    } else {
        EXEC_GIT(G_PUSH);
    }
}