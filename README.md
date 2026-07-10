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

INGEST_PATH=./insert_zone/
ROS_STORAGE=/media/sierra/TESarchive/ros_storage/
EDS_STORAGE=/media/sierra/TESarchive/eds_storage/
BKS_STORAGE=/media/sierra/TESarchive/bks_storage/
TRS_STORAGE=/media/sierra/TESarchive/trs_storage/
```


