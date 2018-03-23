#pragma once

#include "Enum.h"

class NetworkController;
class IocpController;

typedef struct {
	OVERLAPPED overlapped;
	WSABUF wsaBuf;
} IoContext, *IoContextPointer;

typedef struct {
	SOCKET Socket;
	IoContextPointer ReceiveContext;
	IoContextPointer SendContext;
} SocketContext, *SocketContextPointer;

typedef struct {
	SOCKET ListenSocket;
	NetworkController* NetworkController;
	IocpController* IocpController;
} AcceptThreadParam;

typedef struct {
	NetworkController* NetworkController;
	HANDLE IocpHandle;
} IocpThreadParam;

#pragma pack(push,1)
typedef struct
{
	int userID;
	char userName[20];
	char chatMsg[124];
} ChatPacket, *ChatPacketPointer;
#pragma pack(pop)