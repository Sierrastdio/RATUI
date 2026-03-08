# 컴파일러와 옵션 설정
CC = gcc
# -Os 옵션을 추가하여 크기 최적화를 수행합니다.
CFLAGS = -Wall -Os -Wno-unused-result -Iinclude/
LIBS = -lncurses

# 생성할 실행 파일 이름
TARGET = atp

# 컴파일할 소스 파일들
SRCS = 	src/SECTOR_MENU.c 		\
		src/FILE_CHECK.c		\
		src/FILE_SEARCH.c		\
		src/FILE_UTIL.c			\
		src/PathConfig.c		\
		src/help_UI.c			\
								\
		modules/home/HOMEmain.c \
								\
		modules/ROS/ROSmain.c 	\
		modules/ROS/ROSfunc.c	\
								\
		modules/INS/INSmain.c 	\
		modules/INS/INSfunc.c	\
								\
		modules/EDS/EDSmain.c	\
		modules/EDS/EDSfunc.c	\
								\
		modules/BKS/BKSmain.c	\
		modules/BKS/BKSfunc.c	\
								\
		modules/TRS/TRSmain.c	\
		modules/TRS/TRSfunc.c	\

# 기존 실행 파일이 있다면 .bak으로 복사 후 빌드 진행
$(TARGET): $(SRCS)
	@if [ -f $(TARGET) ]; then cp $(TARGET) $(TARGET).bak; fi
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS) -s

# 청소 규칙 (생성된 파일 삭제)
clean:
	rm -f $(TARGET)

all: $(TARGET)
	@size $(TARGET) | tee build_size.log