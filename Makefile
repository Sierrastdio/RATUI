# ==========================================
# 컴파일러 및 옵션 설정
# ==========================================
CC      := gcc
# -Iinclude 및 -I. 옵션으로 헤더 경로 참조
CFLAGS  := -Wall -Wextra -Os -ffunction-sections -fdata-sections -Wno-unused-result -Iinclude -I.
LDFLAGS := -Wl,--gc-sections -Wl,--as-needed
LIBS    := -lncurses
AR      := ar

# 생성할 최종 실행 파일 이름 목록 (reader/writer)
TARGET_READER := reader
TARGET_WRITER := writer
TARGETS       := $(TARGET_READER) $(TARGET_WRITER)

# db + core 모듈은 main()이 없는 라이브러리라 정적 라이브러리(.a)로 빌드
TARGET_DB_LIB := libarchdb.a

# ==========================================
# 소스 파일 및 오브젝트 파일 경로 자동 생성
# ==========================================
# 1. include 및 src 폴더 내의 공통 라이브러리 C 소스 파일 스캔
LIB_SOURCES := $(shell find include -name "*.c" 2>/dev/null)
LIB_OBJS    := $(LIB_SOURCES:.c=.o)

# 2. 최상위 경로의 reader.c와 writer.c 소스
READER_SRC  := reader.c
WRITER_SRC  := writer.c
READER_OBJ  := $(READER_SRC:.c=.o)
WRITER_OBJ  := $(WRITER_SRC:.c=.o)

# 3. db 모듈 소스 (db.c/db.h, core.c/core.h) - 둘 다 libarchdb.a로 묶임
DB_SOURCES := db.c core.c
DB_OBJS    := $(DB_SOURCES:.c=.o)

# 전체 오브젝트 파일 목록 (clean 처리용)
ALL_OBJS := $(LIB_OBJS) $(READER_OBJ) $(WRITER_OBJ) $(DB_OBJS)

# ==========================================
# 의사 타겟 (의도치 않은 파일 이름과의 충돌 방지)
# ==========================================
.PHONY: all rw db log clean

# 기본: reader/writer + db 모듈 전부 빌드
all: rw db

# ------------------------------------------
# reader / writer (배포용: 스트립 플래그 -s 추가로 바이너리 다이어트)
# reader/writer가 db 모듈을 쓰게 되면 아래 두 규칙의 의존성/링크에
# $(DB_OBJS) 와 $(TARGET_DB_LIB) 를 추가하면 됨
# ------------------------------------------
rw: $(TARGETS)
	@echo "=========================================="
	@echo " 배포용 바이너리 생성 완료: $(TARGETS)"
	@size $(TARGETS) | tee build_size.log
	@echo "=========================================="

# reader 실행 파일 생성 규칙 (공통 라이브러리 오브젝트 + reader.o)
$(TARGET_READER): $(LIB_OBJS) $(READER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS) $(LDFLAGS) -s

# writer 실행 파일 생성 규칙 (공통 라이브러리 오브젝트 + writer.o)
$(TARGET_WRITER): $(LIB_OBJS) $(WRITER_OBJ)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS) $(LDFLAGS) -s

# ------------------------------------------
# db 모듈 (db.c 저수준 저장 계층 + core.c 의미론 계층 - 정적 라이브러리로 빌드)
# ------------------------------------------
db: $(TARGET_DB_LIB)
	@echo "=========================================="
	@echo " DB 모듈 빌드 완료: $(TARGET_DB_LIB)"
	@size $(DB_OBJS)
	@echo "=========================================="

$(TARGET_DB_LIB): $(DB_OBJS)
	$(AR) rcs $@ $^

# ------------------------------------------
# 로깅 및 디버깅용 빌드 (심볼 유지)
# ------------------------------------------
log: clean
	$(MAKE) all CFLAGS="$(CFLAGS) -g" LDFLAGS="$(LDFLAGS)"
	@echo "=========================================="
	@echo " 로깅용 바이너리 생성 완료 (디버그 심볼 포함)"
	@echo "=========================================="

# ------------------------------------------
# 컴파일 규칙 (각 .c 파일을 .o 파일로 빌드)
# ------------------------------------------
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ------------------------------------------
# 정리 규칙
# ------------------------------------------
clean:
	rm -f $(ALL_OBJS) $(TARGETS) $(TARGET_DB_LIB) *.bak build_size.log