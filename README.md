### Language / 언어
* [English](#raspberry-archive-interface-ratui)
* [한국어 (Korean)](#라즈베리-아카이브-인터페이스-raspberrypi-archive-terminaal_ui_interface-ratui)

---

# Raspberry Archive Interface (RATUI)

## Introduction

The Raspberry Pi Archive Interface (RATUI) is a lightweight, Text-based User Interface (TUI) application designed for Raspberry Pi. It provides functional assistance when utilizing a Raspberry Pi as a personal archive or private server.

The project is written in C and implements a terminal-based interface using the `ncurses` library.

## Key Features

### 0. Module Features
The project consists of several distinct modules:
- `HOMEmain`: Main home interface
- `ROS (ReadOnly)`: Resource management
- `INS (Insert)`: File insertion and addition functions
- `EDS (Edit)`: File editing functions
- `BKS (Backup)`: Backup-related functions
- `TRS (Trash)`: File transfer functions

### 1. File Explorer System
- Compact and intuitive file browsing capabilities
- Efficient file system management

### 2. File Management System
- **Git Integration**: Execute Git commands and monitor status directly via the TUI (Under development)
- **Custom Convenience Features**: Basic operations such as copying, moving, and deleting files

### 3. Duplicate File Management
- **Phase 1 Filter (File Size)**: Immediately excludes files with different sizes to minimize CPU overhead.
- **Phase 2 Filter (Partial Bit Comparison)**: Compares the first and last 4KB of files to filter duplicate candidates.
- **Phase 3 Confirmation (Hash Comparison)**: Full file hash comparison to ensure 100% match accuracy.

### 4. Automatic Archive Compliance Rules
- Applies automated protocols for compliant file archiving.

## Installation & Build

### Requirements
- Makefile / Meson
- GCC Compiler
- ncurses library

### How to Build
```fish
# Build using Makefile
make
```

### How to Run
```fish
# If built with Makefile:
./make_rti


### Clean Build Artifacts
```fish
make clean
```

### Usage

```txt
# Edit the config.rtuconf file to specify the storage paths for your archive.
# Example:

# config.ratui
INGEST_PATH=/home/Sierrastdio/INS
ROS_STORAGE=/home/Sierrastdio/ROS
EDS_STORAGE=/home/Sierrastdio/EDS
BKS_STORAGE=/home/Sierrastdio/BKS
TRS_STORAGE=/home/Sierrastdio/TRS
```

## Library, Function Naming Conventions, and Interpretation Guide

Header files and libraries written in **all capital letters** contain **core functionality** designed to be reused anywhere they are needed:

- `SECTOR_MENU.h`: A reusable menu engine designed to ensure that each sector or its sub-items utilize the same menu interface.

- `FILE_CHECK.h`: A library that executes file duplication checks. It utilizes a 3-step verification method: `1st: File size comparison -> 2nd: Front/Back 4KB data comparison -> 3rd: Full MD5/SHA hash contrast`. 
  - The `FILE_EXISTENCE_CHECK` function verifies whether a file exists at the specified path. It returns 1 if it exists, and 0 if it does not. 
  - The `FILE_SIZE_GET` function returns the size of the specified file in bytes. It returns -1 if it fails to retrieve the file information. 
  - The `FILE_DUPLICATE_CHECK` function checks if two files are identical using the 3-step process (file size → front/back 4KB comparison → MD5 hash comparison). It returns 1 if they are identical, and 0 if they differ.

- `FILE_SEARCH.h`: A library containing file search capabilities. 
  - The `FILE_NAME_EXTENSION_SEARCH` function searches for files with a specific extension (.txt, .bak, .img, etc.) within a designated directory, saves them to a list, and returns the number of files found. 
  - The `FILE_ALL_LIST_GET` function retrieves a list of all files and folders (excluding hidden files) within a designated directory, saves them to a list, and returns the total number of items.

- `FILE_UTIL.h`: A file management library that provides file **copy** and **move** functionalities. 
  - The `FILE_COPY` function copies the contents of the source (src) file to the destination (dest) file. It returns 1 upon a successful copy, and 0 upon failure. 
  - The `FILE_MOVE` function moves the source file to the destination path. Internally, it copies the file first and then deletes the original source file. It returns 1 upon success, and 0 upon failure.

- `PATH_CONFIG.h`: A configuration library that reads and manages the directory paths of major sectors from a configuration file (`config.ratui`). 
  - The `LOAD_CONFIG()` function reads the configuration file (`config.rtuconf`) and stores the directory paths for each sector (INGEST(INS), ROS, EDS, BKS, TRS) into global variables. 
  - The `ENSURE_DIRECTORIES()` function automatically creates the necessary directories (ROS, EDS, BKS, TRS) based on the paths read from the configuration file. 
  - The `STRIP_NEWLINE` function removes trailing newline characters (\n, \r) from a string to ensure the configuration file is parsed correctly.


### Modules Description

- `<sector>func.h` & `<sector>func.c`: Libraries implemented to handle sector-specific functionalities. They reference the **libraries written in all capital letters** to appropriately utilize and deploy them for each specific sector. Consequently, the direct functionalities of each sector are implemented as a set of individual **functions**.

- **Important**: The functions within `<sector>func.c` are primarily written in lowercase, whereas the functions belonging to the `include` and `src` directories are entirely written in uppercase.
**********************

- `<sector>main.c`: Fits the content to be displayed on each sector screen according to the `SECTOR_MENU` format, and maps the **functions** defined in `<sector>func.c` to correspond with the **return values** of the `SECTOR_MENU` functions.


### Functions Description

- Functions like `SECTOR_MENU()` or `FILE_CHECK()` are defined within the libraries located in the `src/` directory.

- Functions following the patterns of `ROSfunc_manage_storage()`, `ROSfunc_show_info()`, or `INSfunc_handle_file_add()` are defined within `<sector>func.c`.

-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------


# 라즈베리 아카이브 인터페이스 (Raspberrypi Archive Terminaal_UI_Interface RATUI)

## 소개

라즈베리파이  아카이브 인터페이스 (RATUI)는 라즈베리파이에서 동작하는 가벼운 텍스트 기반 사용자 인터페이스(TUI) 애플리케이션입니다. 개인 아카이브(개인 서버)로 라즈베리파이를 사용할 때 기능적인 도움을 제공합니다.

이 프로젝트는 C 언어로 작성되었으며, ncurses 라이브러리를 사용하여 터미널 기반 인터페이스를 구현합니다.

## 주요 기능

### 0. 모듈별 기능
프로젝트는 여러 모듈로 구성되어 있습니다:
- ``HOMEmain``: 메인 홈 인터페이스
- ``ROS (ReadOnly)``: 리소스 관리
- ``INS (Insert)``: 파일 삽입 및 추가 기능
- ``EDS (Edit)``: 파일 편집 기능
- ``BKS (Backup)``: 백업 관련 기능
- ``TRS (Trash)``: 파일 전송 기능

### 1. 파일 탐색기 시스템
- 컴팩트하고 직관적인 파일 탐색 기능
- 효율적인 파일 시스템 관리

### 2. 파일 관리 시스템
- Git 통합: TUI를 통해 Git 명령어 실행 및 상태 모니터링(아직 안만듦)
- 자체 제작 편의 기능: 파일 복사, 이동, 삭제 등의 기본 작업

### 3. 중복 파일 관리
- **1차 필터 (파일 크기)**: 크기가 다른 파일은 즉시 제외 (CPU 연산 최소화)
- **2차 필터 (부분 비트 비교)**: 파일의 앞부분과 뒷부분 4KB를 비교하여 중복 후보 필터링
- **3차 확정 (해시 비교)**: 전체 파일 해시 비교로 100% 일치 확인

### 4. 아카이브 보관 규칙 자동 준수
- 파일 보관을 위한 자동화된 프로토콜 적용



## 설치 및 빌드

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
./make_rti

### 청소
```bash
make clean
```

## 사용 방법
```txt
#config.ratui 파일을 수정하여 아카이브로 쓸 저장장치의 경로를 작성하세요.
#예):

# config.ratui
INGEST_PATH=/home/Sierrastdio/INS
ROS_STORAGE=/home/Sierrastdio/ROS
EDS_STORAGE=/home/Sierrastdio/EDS
BKS_STORAGE=/home/Sierrastdio/BKS
TRS_STORAGE=/home/Sierrastdio/TRS
```


## 라이브러리, 함수명 구분, 해석 방법.


대문자로만 이루어진 헤더파일, 라이브러리는 필요한곳이라면 어디든 쓸 **핵심 기능**을 담고 있습니다 :

- `SECTOR_MENU.h` : 각 섹터나 그 하위 항목에서 같은 메뉴 인터페이스를 사용하도록 만든 재사용 가능한 메뉴 엔진입니다. 

- `FILE_CHECK.h` : 파일 중복 검사를 실행하는 라이브러리입니다. `1차: 파일 크기 비교  2차: 앞/뒤 4KB 데이터 비교  3차: 전체 MD5/SHA 해시 대조` 의 방식을 사용합니다. `FILE_EXISTENCE_CHECK` 함수를 통해 지정한 경로에 파일이 존재하는지 확인합니다. 존재하면 1, 존재하지 않으면 0을 반환합니다. `FILE_SIZE_GET` 함수르르 통해 지정한 파일의 크기(Byte)를 반환합니다. 파일 정보를 가져오지 못하면 -1을 반환합니다. `FILE_DUPLICATE_CHECK` 함수를 통해 두 파일이 동일한 파일인지 3단계(파일 크기 → 앞/뒤 4KB 비교 → MD5 해시 비교)로 검사하여, 동일하면 1, 다르면 0을 반환합니다.

- `FILE_SEARCH.h` : 파일 검색 기능을 담고있는 라이브러리 입니다. `FILE_NAME_EXTENSION_SEARCH` 함수로 지정한 디렉토리에서 특정 확장자(.txt, .bak, .img 등)를 가진 파일만 검색하여 목록에 저장하고, 찾은 파일 개수를 반환합니다. `FILE_ALL_LIST_GET` 함수로 지정한 디렉토리의 모든 파일 및 폴더 목록(숨김 파일 제외)을 가져와 목록에 저장하고, 총 항목 개수를 반환합니다.

- `FILE_UTIL.h` : 파일의 **복사** 및 **이동** 기능을 제공하는 파일 관리 라이브러리입니다. `FILE_COPY` 함수를 통해 원본(src) 파일의 내용을 대상(dest) 파일로 복사합니다. 복사에 성공하면 1, 실패하면 0을 반환합니다. `FILE_MOVE` 함수를 통해 원본 파일을 대상 경로로 이동합니다. 내부적으로 파일을 먼저 복사한 후 원본 파일을 삭제하며, 성공하면 1, 실패하면 0을 반환합니다.

- `PATH_CONFIG.h` : 주요 섹터들의 디렉토리 경로를 설정파일(`config.ratui`)로부터 읽어오고 관리하는 환경설정 라이브러리 입니다. `LOAD_CONFIG()`함수를 통해 설정 파일(`config.rtuconf`)을 읽어 각 섹터(INGEST(INS), ROS, EDS, BKS, TRS)의 디렉토리 경로를 전역 변수에 저장합니다. `ENSURE_DIRECTORIES()`함수는 설정 파일에서 읽어온 경로를 기준으로 필요한 디렉토리(ROS, EDS, BKS, TRS)를 자동으로 생성합니다. `STRIP_NEWLINE` 함수는 문자열 끝에 포함된 개행 문자(\n, \r)를 제거하여 설정 파일을 올바르게 읽을 수 있도록 합니다.


### modules 설명


- `<sector>func.h`& `<sector>func.c`: 섹터 고유의 기능을 구현하기 위한 라이브러리 입니다. **대문자로만 이루어진 라이브러리**를 참조하여 이를 각 섹터에 맞게 적절히 사용. 배치하는 역할을 합니다. 따라서 각 섹터의 직접적 기능을 하나의 **함수**들로 구현합니다.  

- **중요**: `<sector>func.c`의 함수들은 주로 소문자 형태이며 `include`폴더와 `src`폴더의 함수는 전부 대문자로 이루어진 형태입니다.
**********************

- `<sector>main.c` : `SECTOR_MENU` 양식에 맞춰서 각 섹터 화면에 표기할 내용이나, `<sector>func.c`에서 정의한, **함수** 들을 각 `SECTOR_MENU`의 함수의 **반환값**에 맞게 배치합니다.

### 함수 설명

- `SECTOR_MENU()` , 또는 `FILE_CHECK()` 과 같은 함수는 `src/` 의 라이브러리들에서 정의된 함수들입니다.

- `ROSfunc_manage_storage()`, 또는 `ROSfunc_show_info()`, `INSfunc_handle_file_add()`과 **같은 형태** 함수는 `<sector>func.c`에서 정의된 함수입니다.
