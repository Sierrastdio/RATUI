# 컴파일러와 옵션 설정
CC = gcc
CFLAGS = -Wall -Iinclude/
LIBS = -lncurses

# 생성할 실행 파일 이름
TARGET = atp

# 컴파일할 소스 파일들
SRCS = 	src/SECTOR_MENU.c 		\
		src/FILE_CHECK.c		\
		src/FILE_SEARCH.c		\
		src/FILE_UTIL.c			\
		src/PathConfig.c		\
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

# 빌드 규칙
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

# 청소 규칙 (생성된 파일 삭제)
clean:
	rm -f $(TARGET)