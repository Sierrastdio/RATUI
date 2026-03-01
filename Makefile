# 컴파일러와 옵션 설정
CC = gcc
CFLAGS = -Wall -I.
LIBS = -lncurses

# 생성할 실행 파일 이름
TARGET = atp

# 컴파일할 소스 파일들
SRCS = 	
		SECTOR_MENU.c \
		home/HOMEmain.c \
		home/ROS/ROSmain.c \
		home/INS/INSmain.c \

# 빌드 규칙
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

# 청소 규칙 (생성된 파일 삭제)
clean:
	rm -f $(TARGET)