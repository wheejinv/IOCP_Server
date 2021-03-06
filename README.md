# IOCP 관련 Project 모음입니다.

## 01_ChatServer
윤성우 TCP/IP 책의 에코 IOCP 서버 예제를 바탕으로 채팅 서버를 구현하였습니다.

## 02_ChatServer_Cpp_Refactoring

[iocp 파라미터 설명 페이지 링크](./md/02.md)<br>

01_ChatServer와 비교해서 아래의 점이 개선되었습니다.
- 기존 통신 로직을 클래스화 시킴. ( IocpController, NetworkController 클래스 )
- Logger 프로토 타입 구현
- accept를 수행하는 쓰레드를 생성하고 메인 로직에서는 esc 키를 눌렀을 때 종료되게 변경
- Enum.h, Constant.h 파일에 상수 관리.

## 03_ChatServer_Improve

02_ChatServer 에서 아래의 점이 개선되었습니다.
- 통신할 때 구조체 패킷을 사용하기 위해 pragma pack 을 사용.
- 유저들이 나가고 들어와도 관리가 되게끔 만듭니다.
	-  끊김 이벤트가 오면 다른 유저에게 나갔다고 표시

## 04_MySql Connector

[cpp mysql 셋팅 페이지 링크](./md/04.md)<br>

practice 프로젝트,
linux 서버에 mysql 설치 후 데이터 베이스 생성, table 생성
해당 데이터 베이스 권한만 가진 mysql user 생성 및 접속.

## 05_IOCP server 와 unity 클라이언트 간의 통신.

login 화면이 존재하고,
json 형식으로 통신한다.
