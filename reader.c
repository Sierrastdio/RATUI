#include <stdio.h>   /* 표준 입출력 함수(printf, fopen, fread 등) 사용을 위한 헤더 */
#include <stdlib.h>  /* 표준 라이브러리 함수 사용을 위한 헤더 */
#include <stdint.h>  /* uint32_t, uint16_t처럼 정확한 바이트 크기의 정수 타입 선언 헤더 */
#include <string.h>  /* memcpy 같은 메모리/문자열 조작 함수 사용을 위한 헤더 */

#pragma pack(push, 1) /* 메모리 패딩(더미 바이트)을 제거하여 구조체 크기를 딱 138바이트로 고정 */
typedef struct {
    char     tag_id[4];     /* 4바이트 영문 태그 (예: "DOCS", "IMG ") */
    uint32_t file_id;       /* 4바이트 부호 없는 정수형 파일 ID */
    char     file_name[128];/* 128바이트 고정 길이 파일명 버퍼 */
    uint16_t version;       /* 2바이트 부호 없는 정수형 파일 버전 */
} file_record_t;            /* 이 구조체의 별칭을 file_record_t 로 지정 */
#pragma pack(pop)  /* 패딩 설정을 다시 이전 상태로 복구 */

/* DB 파일 파싱 및 출력 함수 정의 */
int read_and_print_db(const char *db_path) {
    FILE *fp = fopen(db_path, "rb"); /* 지정한 경로의 파일을 바이너리 읽기("rb") 모드로 열기 */
    if (!fp) {                        /* 파일 열기에 실패한 경우 (파일이 없거나 권한 부족) */
        perror("DB 파일 열기 실패");    /* 에러 원인을 터미널에 출력 */
        return -1;                    /* 실패를 의미하는 -1 반환 */
    }

    /* 파일 전체 크기 확인 */
    fseek(fp, 0, SEEK_END);          /* 파일 포인터를 파일의 맨 끝(SEEK_END)으로 이동 */
    long file_size = ftell(fp);       /* 현재 파일 포인터의 위치(바이트 단위)를 읽어와 파일 크기 측정 */
    fseek(fp, 0, SEEK_SET);          /* 파일 포인터를 다시 파일의 맨 처음(SEEK_SET)으로 복귀 */

    size_t record_size = sizeof(file_record_t);      /* 레코드 1개의 크기(138바이트) 계산 */
    uint32_t total_records = (uint32_t)(file_size / record_size); /* 전체 크기 / 레코드 크기로 총 개수 계산 */

    /* 터미널 출력용 헤더 표기 */
    printf("========================================================================================\n");
    printf(" DB 파일 경로: %s (크기: %ld 바이트, 총 %u개 레코드)\n", db_path, file_size, total_records);
    printf("========================================================================================\n");
    printf(" %-8s | %-10s | %-60s | %-8s\n", "TAG ID", "FILE ID", "FILE NAME", "VERSION");
    printf("----------------------------------------------------------------------------------------\n");

    file_record_t rec; /* 파일에서 레코드를 한 개씩 읽어올 구조체 변수 선언 */
    uint32_t count = 0;/* 읽어온 레코드 수를 셀 카운터 변수 */

    /* fread로 138바이트(record_size)만큼 읽어서 rec에 저장. 1개를 성공적으로 읽는 동안 반복 */
    while (fread(&rec, record_size, 1, fp) == 1) {
        count++; /* 읽은 개수 1 증가 */

        /* 128바이트 파일명 널 터미네이션 보장 */
        char safe_name[129];               /* 마지막 널 문자('\0')를 넣기 위해 129바이트 배열 준비 */
        memcpy(safe_name, rec.file_name, 128); /* 파일에서 읽은 128바이트를 복사 */
        safe_name[128] = '\0';             /* 끝에 널 문자를 강제로 넣어 문자열 출력 시 출력 이탈 방지 */

        /* Tag ID 출력용 (4바이트 널 터미네이션 보장) */
        char safe_tag[5];                  /* 4바이트 태그 + 널 문자('\0')용 5바이트 배열 준비 */
        memcpy(safe_tag, rec.tag_id, 4);   /* 4바이트 태그를 복사 */
        safe_tag[4] = '\0';                /* 끝에 널 문자 강제 삽입 */

        /* 읽어온 레코드 데이터를 포맷에 맞게 터미널에 줄 단위로 출력 */
        printf(" %-8s | %-10u | %-60s | v%u\n", safe_tag, rec.file_id, safe_name, rec.version);
    }

    /* 하단 결과 출력 */
    printf("========================================================================================\n");
    printf(" 총 %u개의 레코드를 읽었습니다.\n", count);

    fclose(fp); /* 열었던 DB 파일 닫기 */
    return 0;   /* 성공을 의미하는 0 반환 */
}

/* 테스트용 샘플 DB 생성 함수 */
void create_sample_db(const char *db_path) {
    FILE *fp = fopen(db_path, "wb"); /* 바이너리 쓰기("wb") 모드로 파일 열기 (기존 파일 있으면 덮어씀) */
    if (!fp) return;                 /* 파일 생성 실패 시 함수 종료 */

    /* 테스트에 사용할 샘플 데이터 3개 정의 */
    file_record_t samples[3] = {
        { {'T', 'E', 'S', 'T'}, 1001, "example_document_v1.pdf", 1 },
        { {'I', 'M', 'G', ' '}, 1002, "system_architecture_diagram.png", 2 },
        { {'C', 'O', 'D', 'E'}, 2005, "main_executable_binary.bin", 12 }
    };

    /* 배열에 든 3개 데이터를 DB 파일에 138바이트씩 바이너리로 바이패스 저장 */
    for (int i = 0; i < 3; i++) {
        fwrite(&samples[i], sizeof(file_record_t), 1, fp);
    }

    fclose(fp); /* 파일 닫기 */
    printf("[+] 테스트용 샘플 DB 생성 완료: %s\n\n", db_path);
}

/* 메인 실행 함수 */
int main(int argc, char *argv[]) {
    const char *db_path = "files.db"; /* 기본 파일명을 "files.db"로 설정 */

    if (argc > 1) {
        db_path = argv[1];            /* 사용자가 실행 시 인자를 전달했다면 해당 경로를 db_path로 사용 */
    } else {                          /* 인자를 안 넣고 그냥 실행했다면 */
        create_sample_db(db_path);    /* 테스트용 파일이 없어도 확인할 수 있게 샘플 DB를 자동으로 생성 */
    }

    read_and_print_db(db_path);       /* DB 파일을 읽고 출력하는 메인 로직 실행 */

    return 0;                         /* 프로그램 정상 종료 */
}