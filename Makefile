# ==========================================
# 컴파일러 및 옵션 설정
# ==========================================
CC      := gcc
CFLAGS  := -Wall -Wextra -Os -ffunction-sections -fdata-sections -Wno-unused-result -Iinclude
LDFLAGS := -Wl,--gc-sections
LIBS    := -lncurses

# 생성할 최종 실행 파일 이름
TARGET  := make_rti

# ==========================================
# 소스 파일 및 오브젝트 파일 경로 자동 생성
# ==========================================
SOURCES := $(shell find src modules -name "*.c" 2>/dev/null)
OBJS    := $(SOURCES:.c=.o)

# ==========================================
# 의사 타겟 (의도치 않은 파일 이름과의 충돌 방지)
# ==========================================
.PHONY: all log clean

# 1. 기본 빌드 (배포용: 스트립 플래그 -s 추가로 바이너리 다이어트)
all: $(TARGET)
	@echo "=========================================="
	@echo " 배포용 바이너리 생성 완료: $(TARGET)"
	@size $(TARGET) | tee build_size.log
	@echo "=========================================="

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LIBS) $(LDFLAGS) -s

# 2. 로깅 및 디버깅용 빌드 (심볼 유지)
# 기존에 있던 '-s' 옵션을 제거하여 디버깅 데이터가 온전히 남도록 수정했습니다.
log: clean
	@if [ -f $(TARGET) ]; then cp $(TARGET) $(TARGET).bak; fi
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS) $(LDFLAGS)
	@echo "=========================================="
	@echo "로깅용 바이너리 생성 완료 (디버그 심볼 포함)"
	@size $(TARGET) >> build_size.log
	@echo "=========================================="

# 3. 컴파일 규칙 (각 .c 파일을 .o 파일로 빌드)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# 4. 정리 규칙
clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).bak build_size.log
