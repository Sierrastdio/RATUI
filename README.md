# Raspberrypi Archive Terminal UI Interface (RATUI)

## Language / 언어

* [English](#raspberrypi-archive-terminal-ui-interface-ratui)
* [한국어 (Korean)](#라즈베리-아카이브-터미널-유저-인터페이스-raspberrypi-archive-terminal-user-interface-ratui)

---

## Overview

Raspberrypi Archive Terminal UI Interface (RATUI) is a lightweight Text-based User Interface (TUI) application designed for the Raspberry Pi. Aimed at personal archiving, it provides helpful archive management features alongside basic file navigation capabilities.

This project is written in C and uses the `ncurses` library to deliver a terminal-based user interface.

## System Architecture

The overall structure of the project is as follows:

<img width="800" alt="System Architecture" src="https://github.com/user-attachments/assets/51390468-8896-4918-a274-65c3a4b38d32" />

---

## Key Features

### 1. File Tagging & Tag Search

<img width="800" alt="File Tagging 1" src="https://github.com/user-attachments/assets/5edd4881-82b4-40b1-b708-05eac960e64b" />
<img width="800" alt="File Tagging 2" src="https://github.com/user-attachments/assets/56148337-3285-498b-8455-ca44ded54331" />

* Supports assigning multiple tags to a single file.
* Tag names must be in English.

---

<img width="800" alt="Tag Search 1" src="https://github.com/user-attachments/assets/21e8e66d-faeb-4a06-8341-101b532874df" />
<img width="800" alt="Tag Search 2" src="https://github.com/user-attachments/assets/759c80eb-ac01-4786-8bb4-cca7a2dafeb5" />

* Tag-based search functionality.

---

### 2. File Explorer View

<img width="800" alt="File Explorer View 1" src="https://github.com/user-attachments/assets/5b439b72-de83-4ce2-a0be-46a4183557c2" />
<img width="800" alt="File Explorer View 2" src="https://github.com/user-attachments/assets/e5890952-7121-41aa-9bdc-ab26c81d0666" />

* Virtual path navigation treating tags as directories.

---

## Build

### Prerequisites

* `make`
* GCC Compiler
* `ncurses` Library

### Building the Project

```fish
# Build using Makefile
make
```

---
---
---

# 라즈베리 아카이브 터미널 사용자 인터페이스 (Raspberry Pi Archive Terminal User Interface, RATUI)

## 프로젝트 소개

라즈베리파이 아카이브 인터페이스(RATUI)는 라즈베리파이에서 동작하는 가벼운 텍스트 기반 사용자 인터페이스(TUI) 애플리케이션입니다. 개인 아카이브 용도로 라즈베리파이를 사용할 때, 기본적인 파일 탐색기 기능에 더해 아카이브 관리에 필요한 기능을 제공하는 것을 목표로 합니다.

이 프로젝트는 C 언어로 작성되었으며, ncurses 라이브러리를 사용하여 터미널 기반 인터페이스를 구현합니다.

## 시스템 구조

프로젝트는 다음과 같이 구성되어 있습니다:

<img width="800" alt="System Architecture" src="https://github.com/user-attachments/assets/51390468-8896-4918-a274-65c3a4b38d32" />

---

# 주요 기능

## 1. 파일 태그 지정 및 검색 기능

<img width="800" alt="File Tagging 1" src="https://github.com/user-attachments/assets/5edd4881-82b4-40b1-b708-05eac960e64b" />
<img width="800" alt="File Tagging 2" src="https://github.com/user-attachments/assets/56148337-3285-498b-8455-ca44ded54331" />
- 한 파일에 여러 태그를 중복으로 지정할 수 있습니다.
- 태그 이름은 영어만 사용할 수 있습니다.

---
<img width="800" alt="Tag Search 1" src="https://github.com/user-attachments/assets/21e8e66d-faeb-4a06-8341-101b532874df" />
<img width="800" alt="Tag Search 2" src="https://github.com/user-attachments/assets/759c80eb-ac01-4786-8bb4-cca7a2dafeb5" />
- 태그를 기반으로 파일을 검색할 수 있습니다.

---

## 2. 파일 탐색기 뷰

<img width="800" alt="File Explorer View 1" src="https://github.com/user-attachments/assets/5b439b72-de83-4ce2-a0be-46a4183557c2" />
<img width="800" alt="File Explorer View 2" src="https://github.com/user-attachments/assets/e5890952-7121-41aa-9bdc-ab26c81d0666" />
- 태그를 폴더처럼 취급하는 가상 경로 탐색 기능입니다.

---

## 빌드

### 요구 사항

* make
* GCC 컴파일러
* ncurses 라이브러리

### 빌드 방법

```fish
# Makefile로 빌드
make
```

### 실행 방법

```fish
# Makefile로 빌드했다면:
./ratui
```

### 정리

```bash
make clean
```

## 사용 방법

```txt
# config.ratui 파일을 수정하여 실제 파일 경로를 표시할 시작 경로를 지정하세요.
# 예시:

# config.ratui
HOME_PATH=/home/Sierrastdio/
```
