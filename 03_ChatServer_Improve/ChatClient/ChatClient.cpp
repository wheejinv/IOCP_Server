#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <process.h>
#include "Struct.h"

#define BUF_SIZE 100
#define NAME_SIZE 20

unsigned WINAPI SendMsg(void *arg);
unsigned WINAPI RecvMsg(void *arg);
void            ErrorHandling(const char *msg);

char name[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE];

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET hSock;
	SOCKADDR_IN servAdr;
	HANDLE hSndThread, hRcvThread;

	if (argc != 4) {
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	}

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		ErrorHandling("WSAStartup() error!");
	}


	sprintf(name, "%s", argv[3]);
	hSock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = inet_addr(argv[1]);
	servAdr.sin_port = htons(atoi(argv[2]));

	if (connect(hSock, (SOCKADDR *)&servAdr, sizeof(servAdr)) == SOCKET_ERROR) {
		ErrorHandling("connect() error");
	}

	hSndThread = (HANDLE)_beginthreadex(NULL, 0, SendMsg, (void *)&hSock, 0, NULL);
	hRcvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void *)&hSock, 0, NULL);

	WaitForSingleObject(hSndThread, INFINITE);
	WaitForSingleObject(hRcvThread, INFINITE);

	printf("close client socket\n");
	closesocket(hSock);
	WSACleanup();
	Sleep(500);
	return 0;
}

unsigned WINAPI SendMsg(void *arg) // send thread main
{
	SOCKET hSock = *((SOCKET *)arg);
	ChatPacket chatPacket;
	chatPacket.userID = 0;
	sprintf_s(chatPacket.userName, "%s", name);
	

	while (1)
	{
		fflush(stdout);
		fgets(msg, BUF_SIZE, stdin);

		if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {
			closesocket(hSock);
			WSACleanup();
			return 0;
		}

		if (strlen(msg) == 0 || !strcmp(msg, "\n")) {
			continue;
		}

		ZeroMemory(&(chatPacket.chatMsg), sizeof(chatPacket.chatMsg));
		sprintf_s(chatPacket.chatMsg, "%s", msg);

		send(hSock, (const char *)&chatPacket, sizeof(chatPacket), 0);
	}
	return 0;
}

unsigned WINAPI RecvMsg(void *arg) // read thread main
{
	int  hSock = *((SOCKET *)arg);
	int  strLen;

	char chatMsg[sizeof(ChatPacket)];

	ChatPacketPointer chatPacketPointer;/* = (ChatPacketPointer)clientContext->ReceiveContext->wsaBuf.buf;*/
	int userID = 0;
	char* userName = nullptr;
	char* msg = nullptr;

	while (1)
	{
		strLen = recv(hSock, chatMsg, sizeof(ChatPacket), 0);

		if (strLen == -1) {
			return -1;
		}

		chatPacketPointer = (ChatPacketPointer)chatMsg;

		userID = chatPacketPointer->userID;
		userName = chatPacketPointer->userName;
		msg = chatPacketPointer->chatMsg;
		
		printf("[%s] : %s", userName, msg);
		//fputs(nameMsg, stdout);
		fputc('\n', stdout);
	}
	return 0;
}

void ErrorHandling(const char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
