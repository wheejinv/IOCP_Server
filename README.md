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

## 03_ChatServer_Improve....ing

02_ChatServer 에서 아래의 점이 개선되었습니다.
- 통신할 때 구조체 패킷을 사용하기 위해 pragma pack 을 사용.
- 유저들이 나가고 들어와도 관리가 되게끔 만듭니다.
	-  끊김 이벤트가 오면 다른 유저에게 나갔다고 표시
- 유저 정보가 저장된 mysql 연동
