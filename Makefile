# 컴파일러와 옵션 설정
CC = gcc
CFLAGS = -Wall -Os -ffunction-sections -fdata-sections -Wno-unused-result -Iinclude/
LDFLAGS = -Wl,--gc-sections
LIBS = -lncurses

# 생성할 실행 파일 이름
TARGET = atp

# 소스 파일 목록 자동 생성
SOURCES := $(shell find src modules -name "*.c")
OBJS := $(SOURCES:.c=.o)

# 1. 기본 빌드 (배포용: -s 옵션 포함)
all: $(TARGET)
	@size $(TARGET) | tee build_size.log

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LIBS) $(LDFLAGS) -s
	@echo "배포용 바이너리 생성 완료: $(TARGET)"

# 2. 로깅용 빌드 (디버깅 데이터 수집용: -s 옵션 제외)
# make log 로 실행
log: clean
	@if [ -f $(TARGET) ]; then cp $(TARGET) $(TARGET).bak; fi
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LIBS) $(LDFLAGS)
	@echo "로깅용 바이너리 생성 완료 (심볼 포함)"
	@size $(TARGET) >> build_size.log

# 청소 규칙
clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).bak