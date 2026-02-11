
<!-- PROJECT SHIELDS -->
<!--
*** I'm using markdown "reference style" links for readability.
*** Reference links are enclosed in brackets [ ] instead of parentheses ( ).
*** See the bottom of this document for the declaration of the reference variables
*** for contributors-url, forks-url, etc. This is an optional, concise syntax you may use.
*** https://www.markdownguide.org/basic-syntax/#reference-style-links
-->
<div align="center">
  <img src="https://img.shields.io/badge/Language-C++98-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white" alt="C++98 Badge">
  <img src="https://img.shields.io/badge/Platform-Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black" alt="Linux Badge">
  <img src="https://img.shields.io/badge/Architecture-Non--Blocking%20I%2FO-critical?style=for-the-badge" alt="Architecture Badge">
  <br />
  <br />
  <h1>🌐 Web_Study (Webserv)</h1>
  <p align="center">
    <b>Nginx를 모방한 C++98 기반의 초경량, 고성능 HTTP 웹 서버</b>
    <br />
    <a href="#-getting-started"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    <a href="#-key-features">View Demo</a>
    ·
    <a href="#-tech-stack">Report Bug</a>
    ·
    <a href="#-contact">Request Feature</a>
  </p>
</div>

<!-- TABLE OF CONTENTS -->
<details open="open">
  <summary><h2 style="display: inline-block">Mokcha (Table of Contents)</h2></summary>
  <ol>
    <li>
      <a href="#-about-the-project">About The Project</a>
      <ul>
        <li><a href="#-tech-stack">Tech Stack</a></li>
      </ul>
    </li>
    <li><a href="#-key-features">Key Features</a></li>
    <li>
      <a href="#-getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#-usage">Usage</a></li>
    <li><a href="#-project-structure">Project Structure</a></li>
    <li><a href="#-learning-outcomes">Learning Outcomes</a></li>
  </ol>
</details>

---

## 📖 About The Project

> **"HTTP의 본질을 이해하기 위해 바닥부터 쌓아 올린 웹 서버"**

이 프로젝트는 **C++ 98 표준**을 준수하여 직접 구현한 **Webserv**입니다.  
상용 웹 서버인 Nginx의 동작 방식을 심도 있게 분석하고, **HTTP 1.1 프로토콜**, **소켓 프로그래밍**, 그리고 **논블로킹(Non-blocking) I/O 아키텍처**를 직접 구현하며 시스템 프로그래밍 역량을 강화했습니다.

단순한 기능 구현을 넘어, **Epoll**을 활용한 I/O 멀티플렉싱을 통해 단일 스레드에서도 다수의 클라이언트 요청을 효율적으로 처리하는 **이벤트 기반(Event-driven) 서버**를 완성했습니다.

### 🛠 Tech Stack

| Category | Technology | Description |
| :--- | :--- | :--- |
| **Language** | ![C++](https://img.shields.io/badge/C++98-00599C?style=flat-square&logo=c%2B%2B&logoColor=white) | ISO C++98 Standard Compliance |
| **System API** | ![Linux](https://img.shields.io/badge/Linux_Epoll-FCC624?style=flat-square&logo=linux&logoColor=black) | I/O Multiplexing (High Concurrency) |
| **Build Tool** | ![Makefile](https://img.shields.io/badge/Makefile-TYPE-important?style=flat-square) | Compilation Automation |
| **Architecture** | ![EventLoop](https://img.shields.io/badge/Event--Driven-Architecture-blueviolet?style=flat-square) | Reactor Pattern, Non-blocking I/O |

---

## ✨ Key Features

이 웹 서버는 **RFC 2616 (HTTP/1.1)** 표준을 기반으로 하며 다음과 같은 핵심 기능을 제공합니다.

- ✅ **HTTP 1.1 메서드 지원**: `GET`, `POST`, `DELETE` 요청 처리를 완벽하게 지원합니다.
- 🚀 **고성능 이벤트 루프**: `epoll()` 시스템 콜을 사용하여 수백 개의 동시 연결을 효율적으로 관리합니다.
- 🔄 **Chunked Transfer Encoding**: 대용량 데이터를 청크 단위로 분할 전송하여 메모리 효율성을 극대화합니다.
- 📂 **정적 파일 서빙 & Auto Index**: HTML, 이미지 등 정적 리소스를 제공하며, 디렉토리 접근 시 파일 목록을 자동 생성합니다.
- 🐍 **CGI (Common Gateway Interface)**: Python, PHP 등 외부 스크립트를 실행하여 동적 웹 페이지를 생성합니다. (ex: 계산기 기능)
- ⚙️ **강력한 설정 파일 파싱**: 포트, 호스트, 에러 페이지, 클라이언트 바디 제한, 라우트별 규칙 등을 Nginx 스타일의 설정 파일로 제어합니다.

---

## � Getting Started

이 프로젝트를 로컬 환경에서 실행하려면 다음 단계를 따르세요.

### Prerequisites

*   **Linux Environment** (Virtual Machine, WSL, or Native Linux)
    *   *Note: `epoll` API를 사용하므로 Linux 환경이 필수입니다.*
*   **Compiler**: `g++` or `clang++`
*   **Build Tool**: `make`

### Installation

1.  Repository Clone
    ```sh
    git clone https://github.com/your-username/Web_Study.git
    cd Web_Study/Webserv
    ```

2.  Build Project
    ```sh
    make
    ```
    빌드가 완료되면 `program` 실행 파일이 생성됩니다.

---

## 💻 Usage

서버를 실행하려면 설정 파일(`.conf`)이 필요합니다. 기본 제공되는 설정 파일을 사용하거나 직접 작성할 수 있습니다.

```bash
# 기본 설정 파일로 서버 실행
./program configFile/defaultServerConfig.conf
```

### Configuration Example
```nginx
server {
    listen 8080;
    server_name localhost;
    root ./www;
    index index.html;

    location / {
        allow_methods GET POST;
        autoindex on;
    }
}
```

---

## 📂 Project Structure

프로젝트의 디렉토리 구조는 모듈화와 유지보수성을 고려하여 설계되었습니다.

```bash
Webserv/
├── 📂 srcs/
│   ├── 📜 main.cpp         # 프로그램 진입점 (Entry Point)
│   ├── 📂 configParsing/   # 설정 파일 파싱 및 유효성 검사 로직
│   ├── 📂 socket/          # 소켓 생명주기 관리 및 Epoll 이벤트 루프
│   └── 📂 httpMsg/         # HTTP 요청/응답 파서 및 메시지 객체
├── 📂 configFile/          # 서버 설정 파일 예제 (.conf)
├── 📂 cgi-bin/             # CGI 테스트 스크립트 (Python 등)
├── 📂 www/                 # 웹 루트 디렉토리 (HTML, CSS 등)
└── 📜 Makefile             # 빌드 스크립트
```

---

## 📝 Learning Outcomes

이 프로젝트를 진행하며 다음과 같은 기술적 성장을 이루었습니다.

1.  **Network Programming Deep Dive**:
    *   TCP/IP 3-way handshake부터 소켓의 `bind`, `listen`, `accept` 과정을 직접 제어하며 네트워크 통신의 저수준 동작 원리를 체득했습니다.
2.  **Asynchronous & Non-blocking I/O**:
    *   블로킹 I/O의 한계를 극복하기 위해 `fcntl`로 소켓을 논블로킹 모드로 전환하고, `epoll`을 통해 이벤트 기반 비동기 처리를 구현했습니다.
3.  **HTTP Core Spec Implementation**:
    *   상태 코드(200, 404, 500 등), 헤더 파싱, 바디 처리 등 HTTP 프로토콜의 표준 규격을 준수하는 파서를 직접 작성했습니다.
4.  **Legacy C++ (C++98) Constraint**:
    *   스마트 포인터 등 모던 C++ 기능을 사용할 없는 제약 환경에서 RAII 패턴을 활용하여 메모리 누수를 방지하고 리소스를 안전하게 관리했습니다.

---

## 📧 Contact

**Name** - [JunKwak]  
**Email** - [ruito@naver.com]  
**Profile** - [Yohttps://github.com/composerboyV]

