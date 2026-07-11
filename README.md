# Language / 언어
* [English](#raspberry-archive-interface-ratui)
* [한국어 (Korean)](#라즈베리-아카이브-인터페이스-ratui)

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

### Build using Meson
```fish
meson compile -C bin && meson compile -C bin atp_msg
```

### How to Run
```fish
# If built with Makefile:
./make_rti

# If built with Meson:
./meson_rti
```

### Clean Build Artifacts
```fish
make clean
```

### Usage

```txt
# Edit the config.rtuconf file to specify the storage paths for your archive.
# Example:

# config.rtuconf
INGEST_PATH=/home/Sierrastdio/INS
ROS_STORAGE=/home/Sierrastdio/ROS
EDS_STORAGE=/home/Sierrastdio/EDS
BKS_STORAGE=/home/Sierrastdio/BKS
TRS_STORAGE=/home/Sierrastdio/TRS
```

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

```fish
# meson 으로 빌드
meson compile -C bin && meson compile -C bin atp_msg
```

### 실행 방법
```fish
# Makefile 로 빌드 했다면:
./make_rti

# meson 으로 빌드 했다면:
./meson_rti
```

### 청소
```bash
make clean
```

## 사용 방법
```txt
#config.rtuconf 파일을 수정하여 아카이브로 쓸 저장장치의 경로를 작성하세요.
#예):

# config.rtuconf
INGEST_PATH=/home/Sierrastdio/INS
ROS_STORAGE=/home/Sierrastdio/ROS
EDS_STORAGE=/home/Sierrastdio/EDS
BKS_STORAGE=/home/Sierrastdio/BKS
TRS_STORAGE=/home/Sierrastdio/TRS
```


