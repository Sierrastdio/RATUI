#include "FILE_CHECK.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

/*-----------------------------------------------------------------------------------*/
int FILE_EXISTENCE_CHECK(const char *path) {
    return (access(path, F_OK) == 0) ? 1 : 0;
}
/*-----------------------------------------------------------------------------------*/
long FILE_SIZE_GET(const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) return st.st_size;
    return -1;
}
/*-----------------------------------------------------------------------------------*/
int FILE_DUPLICATE_CHECK(const char *src_path, const char *dest_path) {
    struct stat src_st, dest_st;
    if (stat(src_path, &src_st) != 0 || stat(dest_path, &dest_st) != 0) return 0;

    // 1차: 크기 비교
    if (src_st.st_size != dest_st.st_size) return 0;

    // 2차: 앞/뒤 4KB 비교
    FILE *f1 = fopen(src_path, "rb");
    FILE *f2 = fopen(dest_path, "rb");
    if (!f1 || !f2) { if(f1) fclose(f1); if(f2) fclose(f2); return 0; }

    char buf1[4096], buf2[4096];
    size_t r1 = fread(buf1, 1, 4096, f1);
    size_t r2 = fread(buf2, 1, 4096, f2);
    if (r1 != r2 || memcmp(buf1, buf2, r1) != 0) {
        fclose(f1); fclose(f2); return 0;
    }

    if (src_st.st_size > 4096) {
        fseek(f1, -4096, SEEK_END);
        fseek(f2, -4096, SEEK_END);
        fread(buf1, 1, 4096, f1);
        fread(buf2, 1, 4096, f2);
        if (memcmp(buf1, buf2, 4096) != 0) {
            fclose(f1); fclose(f2); return 0;
        }
    }
    fclose(f1); fclose(f2);

    // 3차: MD5 해시 대조
    char cmd1[512], cmd2[512], h1[64], h2[64];
    sprintf(cmd1, "md5sum '%s' | cut -d' ' -f1", src_path);
    sprintf(cmd2, "md5sum '%s' | cut -d' ' -f1", dest_path);
    FILE *p1 = popen(cmd1, "r");
    FILE *p2 = popen(cmd2, "r");
    if (p1 && p2) {
        fgets(h1, 64, p1); fgets(h2, 64, p2);
        pclose(p1); pclose(p2);
        return (strcmp(h1, h2) == 0);
    }
    if(p1) {
        pclose(p1); 
    }
    if(p2) {
        pclose(p2);
    }
    return 0;
}