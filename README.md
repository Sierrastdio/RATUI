### Language / 언어
* [English](#raspberry-archive-interface-ratui)
* [한국어 (Korean)](#라즈베리-아카이브-인터페이스-raspberrypi-archive-terminaal_ui_interface-ratui)

---


# Raspberry Archive Interface (Raspberrypi Archive Terminal_UI_Interface RATUI)

## Introduction

The Raspberry Pi Archive Interface (RATUI) is a lightweight, Text-based User Interface (TUI) application designed to run on the Raspberry Pi. When using a Raspberry Pi as a personal archive, it aims to provide functional assistance tailored for archive management, in addition to basic file explorer capabilities.

This project is written in C and implements a terminal-based interface using the ncurses library.

## Key Features

### 0. Module Features
The project consists of several distinct modules:
- HOMEmain: Main home interface
- ROS (ReadOnly): Main storage repository
- INS (Insert): Temporary staging area before moving files to main storage
- EDS (Edit): File editing functionality
- BKS (Backup): Backup-related functions
- TRS (Trash): Files scheduled for deletion or disposal

### 1. File Explorer System
- Compact and intuitive file browsing capabilities
- Efficient file system management

### 2. File Management System
- Custom Convenience Features: Basic operations such as copying, moving, and deleting files

### 3. Duplicate File Management
- Phase 1 Filter (File Size): Immediately excludes files with different sizes
- Phase 2 Filter (Partial Bit Comparison): Compares the first and last 4KB of files to filter duplicate candidates
- Phase 3 Confirmation (Hash Comparison): Full file hash comparison to ensure 100% match accuracy

## Installation & Build

### Requirements
- Makefile
- GCC Compiler
- ncurses library

### How to Build
# Build using Makefile
make

### How to Run
# If built with Makefile:
./make_rti

### Clean Build Artifacts
make clean

## Usage
# Edit the config.ratui file to specify the storage paths for your archive.
# Example:

# config.ratui
INGEST_PATH=/home/Sierrastdio/INS
ROS_STORAGE=/home/Sierrastdio/ROS
EDS_STORAGE=/home/Sierrastdio/EDS
BKS_STORAGE=/home/Sierrastdio/BKS
TRS_STORAGE=/home/Sierrastdio/TRS

## Library, Function Naming Conventions, and Interpretation Guide

Header files and libraries written in all capital letters contain core functionality designed to be reused anywhere they are needed:

- `SECTOR_MENU.h`: A reusable menu engine designed to ensure that each sector or its sub-items utilize the same menu interface.

- `FILE_CHECK.h`: A library that executes file duplication checks. It utilizes a 3-step verification method: 1st: File size comparison -> 2nd: Front/Back 4KB data comparison -> 3rd: Full MD5/SHA hash comparison.
  - The FILE_EXISTENCE_CHECK function verifies whether a file exists at the specified path. It returns 1 if it exists, and 0 if it does not.
  - The FILE_SIZE_GET function returns the size of the specified file in bytes. It returns -1 if it fails to retrieve the file information.
  - The FILE_DUPLICATE_CHECK function checks if two files are identical using the 3-step process (file size -> front/back 4KB comparison -> MD5 hash comparison). It returns 1 if they are identical, and 0 if they differ.

- `FILE_SEARCH.h`: A library containing file search capabilities.
  - The FILE_NAME_EXTENSION_SEARCH function searches for files with a specific extension (.txt, .bak, .img, etc.) within a designated directory, saves them to a list, and returns the number of files found.
  - The FILE_ALL_LIST_GET function retrieves a list of all files and folders (excluding hidden files) within a designated directory, saves them to a list, and returns the total number of items.

- `FILE_UTIL.h`: A file management library that provides file copy and move functionalities.
  - The FILE_COPY function copies the contents of the source file to the destination file. It returns 1 upon a successful copy, and 0 upon failure.
  - The FILE_MOVE function moves the source file to the destination path. Internally, it copies the file first and then deletes the original source file. It returns 1 upon success, and 0 upon failure.

- `PATH_CONFIG.h`: A configuration library that reads and manages directory paths of major sectors from the configuration file (config.ratui).
  - The LOAD_CONFIG() function reads the configuration file (config.rtuconf) and stores the directory paths for each sector (INGEST(INS), ROS, EDS, BKS, TRS) into global variables.
  - The ENSURE_DIRECTORIES() function automatically creates the necessary directories (ROS, EDS, BKS, TRS) based on the paths read from the configuration file.
  - The STRIP_NEWLINE function removes trailing newline characters (\n, \r) from a string to ensure the configuration file is parsed correctly.

### Modules Description

- `<sector>func.h & <sector>func.c`: Libraries implemented to handle sector-specific functionalities. They reference the libraries written in all capital letters to appropriately utilize and deploy them for each specific sector. Consequently, the direct functionalities of each sector are implemented as a set of individual functions.

- Important: The functions within `<sector>func.c` are primarily written in lowercase, whereas the functions belonging to the include and src directories are entirely written in uppercase.

- `<sector>main.c`: Fits the content to be displayed on each sector screen according to the SECTOR_MENU format, and maps the functions defined in <sector>func.c to correspond with the return values of the SECTOR_MENU functions.

### Functions Description

- Functions like SECTOR_MENU() or FILE_CHECK() are defined within the libraries located in the src/ directory.

- Functions following the patterns of ROSfunc_manage_storage(), ROSfunc_show_info(), or INSfunc_handle_file_add() are defined within <sector>func.c.



-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------


# 라즈베리 아카이브 인터페이스 (Raspberrypi Archive Terminaal_UI_Interface RATUI)

## 프로젝트 소개

라즈베리파이  아카이브 인터페이스 (RATUI)는 라즈베리파이에서 동작하는 가벼운 텍스트 기반 사용자 인터페이스(TUI) 애플리케이션입니다. 개인 아카이브로서 라즈베리파이를 사용할 때 기본적인 파일 탐색기의 기능에 더해 아카이브 관리에 맞는 기능적인 도움을 제공하는것을 목표로 합니다.

이 프로젝트는 C 언어로 작성되었으며, ncurses 라이브러리를 사용하여 터미널 기반 인터페이스를 구현합니다.

## 시스템 구조.
프로젝트는 다음과 같이 구성되어 있습니다:
<img width="1920" height="1080" alt="Image" src="https://github.com/user-attachments/assets/51390468-8896-4918-a274-65c3a4b38d32" />


# 주요 기능

## 1. 파일에 태그 할당 기능& 검색 기능.
<img width="1918" height="991" alt="Image" src="https://github.com/user-attachments/assets/5edd4881-82b4-40b1-b708-05eac960e64b" />
<img width="1917" height="990" alt="Image" src="https://github.com/user-attachments/assets/56148337-3285-498b-8455-ca44ded54331" />
- 한 파일에 다른 태그 중복 할당 가능.
- 태그명은 영어만.

---------------------------------------------------------

<img width="1916" height="995" alt="Image" src="https://github.com/user-attachments/assets/21e8e66d-faeb-4a06-8341-101b532874df" />
<img width="1917" height="997" alt="Image" src="https://github.com/user-attachments/assets/759c80eb-ac01-4786-8bb4-cca7a2dafeb5" />
- 태그명 검색 기능.


## 2. 파일 탐색기 뷰
<img width="1918" height="986" alt="Image" src="https://github.com/user-attachments/assets/5b439b72-de83-4ce2-a0be-46a4183557c2" />
<img width="1916" height="989" alt="Image" src="https://github.com/user-attachments/assets/e5890952-7121-41aa-9bdc-ab26c81d0666" />
- 태그를 폴더처럼 취급한 가상 경로 탐색.


## 빌드

### 요구 사항
- makefile:
- GCC 컴파일러
- ncurses 라이브러리

### 빌드 방법
```fish
# Makefile 로 빌드
make
```

### 실행 방법
```fish
# Makefile 로 빌드 했다면:
./tui
```

### 청소
```bash
make clean
```

## 사용 방법
```txt
#config.ratui 파일을 수정하여 실제 파일 경로들을 표시할 시작 경로를 지정하세요.
#예):

# config.ratui
HOME_PATH=/home/Sierrastdio/
```
