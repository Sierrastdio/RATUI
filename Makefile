# ==========================================
# 컴파일러 및 옵션 설정
# ==========================================
CC      := gcc
# -Iinclude 및 -I. 옵션으로 헤더 경로 참조
CFLAGS  := -Wall -Wextra -Os -ffunction-sections -fdata-sections -Wno-unused-result -Iinclude -I.
LDFLAGS := -Wl,--gc-sections -Wl,--as-needed
LIBS    := -lncurses
AR      := ar

# 생성할 최종 실행 파일 이름
TARGET_TUI := tui
TARGETS    := $(TARGET_TUI)

# db + core 모듈은 main()이 없는 라이브러리라 정적 라이브러리(.a)로 빌드
TARGET_DB_LIB := libarchdb.a

# ==========================================
# 소스 파일 및 오브젝트 파일 경로 자동 생성
# ==========================================
# 1. include 폴더 내의 공통 UI 라이브러리 소스 스캔 (UI_PRINT.c, SECTOR_MENU.c, help_UI.c 등)
LIB_SOURCES := $(shell find include -name "*.c" 2>/dev/null)
LIB_OBJS    := $(LIB_SOURCES:.c=.o)

# 2. 최상위 경로의 TUI 진입점 소스
TUIFUNC_SRC := TUIfunc.c
TUIFUNC_OBJ := $(TUIFUNC_SRC:.c=.o)

TUIMAIN_SRC := TUImain.c
TUIMAIN_OBJ := $(TUIMAIN_SRC:.c=.o)

# 3. db 모듈 소스 (db.c/db.h, core.c/core.h) - 둘 다 libarchdb.a로 묶임
DB_SOURCES := db.c core.c
DB_OBJS    := $(DB_SOURCES:.c=.o)

# 전체 오브젝트 파일 목록 (clean 처리용)
ALL_OBJS := $(LIB_OBJS) $(TUIFUNC_OBJ) $(TUIMAIN_OBJ) $(DB_OBJS)

# ==========================================
# 의사 타겟 (의도치 않은 파일 이름과의 충돌 방지)
# 주의: tui 는 실제로 만들어지는 바이너리 파일 이름이기도 해서
#       PHONY 목록에는 넣지 않는다 (넣으면 매번 강제로 재빌드됨)
# ==========================================
.PHONY: all db log clean

# 기본: tui 실행파일 (내부적으로 db 모듈도 함께 빌드됨)
all: $(TARGET_TUI)

# ------------------------------------------
# tui 실행 파일 생성 규칙 (배포용: 스트립 플래그 -s 추가로 바이너리 다이어트)
# (공통 UI 라이브러리 오브젝트 + TUIfunc.o + TUImain.o + libarchdb.a 링크)
# ------------------------------------------
$(TARGET_TUI): $(LIB_OBJS) $(TUIFUNC_OBJ) $(TUIMAIN_OBJ) $(TARGET_DB_LIB)
	$(CC) $(CFLAGS) $(LIB_OBJS) $(TUIFUNC_OBJ) $(TUIMAIN_OBJ) $(TARGET_DB_LIB) -o $@ $(LIBS) $(LDFLAGS) -s
	@echo "=========================================="
	@echo " 배포용 바이너리 생성 완료: $(TARGET_TUI)"
	@size $(TARGET_TUI) | tee build_size.log
	@echo "=========================================="

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