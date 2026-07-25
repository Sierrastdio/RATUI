# ==========================================
# 컴파일러 및 옵션 설정
# ==========================================
CC      := gcc
# -Iinclude 및 -I. 옵션으로 헤더 경로 참조
CFLAGS  := -Wall -Wextra -Os -ffunction-sections -fdata-sections -Wno-unused-result -Iinclude -I.
LDFLAGS := -Wl,--gc-sections -Wl,--as-needed
LIBS    := -lncurses

# 생성할 최종 실행 파일 이름 목록
TARGET_READER := reader
TARGET_WRITER := writer
TARGETS       := $(TARGET_READER) $(TARGET_WRITER)

# ==========================================
# 소스 파일 및 오브젝트 파일 경로 자동 생성
# ==========================================
# 1. include 및 src 폴더 내의 공통 라이브러리 C 소스 파일 스캔
LIB_SOURCES := $(shell find src -name "*.c" 2>/dev/null)
LIB_OBJS    := $(LIB_SOURCES:.c=.o)

# 2. 최상위 경로의 reader.c와 writer.c 소스
READER_SRC  := reader.c
WRITER_SRC  := writer.c

READER_OBJ  := $(READER_SRC:.c=.o)
WRITER_OBJ  := $(WRITER_SRC:.c=.o)

# 전체 오브젝트 파일 목록 (clean 처리용)
ALL_OBJS    := $(LIB_OBJS) $(READER_OBJ) $(WRITER_OBJ)

# ==========================================
# 의사 타겟 (의도치 않은 파일 이름과의 충돌 방지)
# ==========================================
.PHONY: all log clean

# 1. 기본 빌드 (배포용: 스트립 플래그 -s 추가로 바이너리 다이어트)
all: $(TARGETS)
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

# 2. 로깅 및 디버깅용 빌드 (심볼 유지)
log: clean
	$(MAKE) all CFLAGS="$(CFLAGS) -g" LDFLAGS="$(LDFLAGS)"
	@echo "=========================================="
	@echo " 로깅용 바이너리 생성 완료 (디버그 심볼 포함)"
	@echo "=========================================="

# 3. 컴파일 규칙 (각 .c 파일을 .o 파일로 빌드)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 4. 정리 규칙
clean:
	rm -f $(ALL_OBJS) $(TARGETS) *.bak build_size.log