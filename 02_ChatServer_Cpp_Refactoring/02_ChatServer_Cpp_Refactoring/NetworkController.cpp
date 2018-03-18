#include "stdafx.h"
#include "NetworkController.h"
#include "Logger.h"

void NetworkController::InitIocpController()
{

}

void NetworkController::InitWinSock()
{
	WSADATA wsaData;
	int resultCode = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (resultCode != 0) {
		PrintLog(eLogLevel::Error, "NetworkController::InitWinSock()-> %d", resultCode);
	}
	else {
		PrintLog(eLogLevel::Info, "NetworkController::InitWinSock()");
	}
}

void NetworkController::CreateListenSocket()
{
	mListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (mListenSocket == INVALID_SOCKET) {
		PrintLog(eLogLevel::Error, "NetworkController::CreateListenSocket() -> %d", WSAGetLastError() );
		WSACleanup();
	}
	else {
		PrintLog(eLogLevel::Info, "NetworkController::CreateListenSocket()");
	}
}

void NetworkController::Bind(const u_short port)
{
	assert(port > -1 && "port number is negative");

	SOCKADDR_IN servAddr;
	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(port);

	int resultCode = bind(mListenSocket, (SOCKADDR *)&servAddr, sizeof(servAddr));

	if (resultCode == SOCKET_ERROR) {
		PrintLog(eLogLevel::Info, "NetworkController::Bind() -> %d", WSAGetLastError());
		closesocket(mListenSocket);
		WSACleanup();
	}
	else {
		PrintLog(eLogLevel::Info, "NetworkController::Bind()");
	}
}

void NetworkController::StartListening()
{
	if (listen(mListenSocket, 5) == SOCKET_ERROR) {
		PrintLog(eLogLevel::Error, "NetworkController::StartListening() -> %d", WSAGetLastError());
		closesocket(mListenSocket);
		WSACleanup();
	}
	else {
		PrintLog(eLogLevel::Info, "NetworkController::StartListening()");
	}
}

void NetworkController::WaitForAccept()
{
	AcceptThreadParam* params = new AcceptThreadParam;
	params->ListenSocket = mListenSocket;
	params->IocpController = &mIocpController;
	params->NetworkController = this;

	mAcceptThread = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)StartThread, params, 0, 0);
}

void NetworkController::CloseServer()
{
	cout << "s" << endl;
}

SocketContextPointer NetworkController::CreateSocketContext(const SOCKET clientSocket)
{
	assert(clientSocket != NULL && "clientSocket is NULL");

	SocketContextPointer contextPotiner = new SocketContext;
	IoContextPointer receivePointer = new IoContext;
	IoContextPointer sendPointer = new IoContext;

	contextPotiner->Socket = clientSocket;
	contextPotiner->ReceiveContext = receivePointer;
	contextPotiner->SendContext = sendPointer;

	contextPotiner->ReceiveContext->wsaBuf.buf = new char[SOCKET_BUF_SIZE];
	contextPotiner->ReceiveContext->wsaBuf.len = SOCKET_BUF_SIZE;

	contextPotiner->SendContext->wsaBuf.buf = new char[SOCKET_BUF_SIZE];
	contextPotiner->SendContext->wsaBuf.len = SOCKET_BUF_SIZE;

	ZeroMemory(contextPotiner->ReceiveContext->wsaBuf.buf, sizeof(char) * SOCKET_BUF_SIZE);
	ZeroMemory(contextPotiner->SendContext->wsaBuf.buf, sizeof(char) * SOCKET_BUF_SIZE);

	return contextPotiner;
}


void __stdcall NetworkController::StartThread(AcceptThreadParam* const params)
{
	SOCKET listenSocket = params->ListenSocket;
	NetworkController* networkController = params->NetworkController;
	IocpController* iocpController = params->IocpController;

	SOCKADDR_IN clientSockAddr;
	int clientSocketLen = sizeof(clientSockAddr);

	PrintLog(eLogLevel::Info, "Starting accept()");

	while (true)
	{
		SOCKET clientSocket = accept(listenSocket, (SOCKADDR *)&clientSockAddr, &clientSocketLen);

		if (clientSocket == INVALID_SOCKET)
		{
			int errorCode = WSAGetLastError();

			// listen socket closed
			if (errorCode == WSAEINTR) {
				PrintLog(eLogLevel::Error, "closed listen socket. (WSAEINTR)");
				break;
			}

			if (errorCode == WSAENOTSOCK) {
				PrintLog(eLogLevel::Error, "closed listen socket. (WSAENOTSOCK)");
				break;
			}

			PrintLog(eLogLevel::Error, "Accepting Client Error - %d", errorCode);
		}

		char ipBuf[64];
		inet_ntop(AF_INET, &(clientSockAddr.sin_addr), ipBuf, sizeof(ipBuf));

		PrintLog(eLogLevel::Info, "accepted! - client(%d) / IP : %s / Port : %d", clientSocket, ipBuf, ntohs(clientSockAddr.sin_port));

		SocketContextPointer clientContext = networkController->CreateSocketContext(clientSocket);
		networkController->AddClientContextToSet(clientContext);
		iocpController->Associate((HANDLE)clientSocket, reinterpret_cast<ULONG_PTR>(clientContext));

		networkController->PostReceive(clientContext);
	}

}


void NetworkController::AddClientContextToSet(const SocketContextPointer clientContext)
{
	assert(clientContext != NULL && "clientContext is NULL.");
	mSetSocketContext.insert(clientContext);

}

void NetworkController::DeleteClientContextInSet(const SocketContextPointer key)
{

}

void NetworkController::DeleteAllClientContext()
{

}