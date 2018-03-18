#pragma once

#include "Enum.h"

class NetworkController;

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