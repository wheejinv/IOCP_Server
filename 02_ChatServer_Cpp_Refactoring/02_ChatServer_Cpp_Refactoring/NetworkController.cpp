#include "stdafx.h"
#include "NetworkController.h"
#include "Logger.h"

NetworkController::NetworkController()
{
	hMutex = CreateMutex(NULL, FALSE, NULL);
}
NetworkController::~NetworkController()
{
	CloseHandle(hMutex);
}

void NetworkController::InitIocpController()
{
	mIocpController.CreateIocpHandle();
	mIocpController.CreateThreadPool(this);
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
	assert(mListenSocket != INVALID_SOCKET && "mListenSocket is INVALID_SOCKET.");

	mIocpController.TerminateThreads();
	DeleteAllClientContext();
	closesocket(mListenSocket);
	CloseHandle(mAcceptThread);
	WSACleanup();

	PrintLog(eLogLevel::Info, "NetworkController::CloseServer()");
}

void NetworkController::PostReceive(const SocketContextPointer clientContext)
{
	assert(clientContext != NULL && "clientContext is NULL.");

	DWORD sizeReceived = 0;
	DWORD flags = 0;

	ZeroMemory(&(clientContext->ReceiveContext->overlapped), sizeof(clientContext->ReceiveContext->overlapped));
	ZeroMemory(clientContext->ReceiveContext->wsaBuf.buf, SOCKET_BUF_SIZE);

	int resultCode = WSARecv(clientContext->Socket, &(clientContext->ReceiveContext->wsaBuf), 1,
		&sizeReceived, &flags, &(clientContext->ReceiveContext->overlapped), NULL);

	if (resultCode == SOCKET_ERROR) {
		int errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING) {
			PrintLog(eLogLevel::Error, "WSARecv() - client(%d), Error code(%d)", clientContext->Socket, errorCode);
		}
	}
}
void NetworkController::ProceedReceive(const SocketContextPointer clientContext, const DWORD receiveBytes)
{
	assert(clientContext != NULL && "clientContext is NULL.");

	PrintLog(eLogLevel::Info, "packet received from client(%d) / thread id : %d / msg : %s",
		clientContext->Socket, GetCurrentThreadId(), clientContext->ReceiveContext->wsaBuf.buf);

	// 에코 버전
	//PostSend(clientContext, clientContext->ReceiveContext->wsaBuf.buf);

	// 모든 클라이언트에게 동작하도록 작업.
	SendAllClient(clientContext->ReceiveContext->wsaBuf.buf);
}
void NetworkController::PostSend(const SocketContextPointer clientContext, const char* msg)
{
	assert(clientContext != NULL && "clientContext is NULL");

	DWORD byteSent = 0;

	ZeroMemory(&(clientContext->SendContext->overlapped), sizeof(clientContext->SendContext->overlapped));
	ZeroMemory(clientContext->SendContext->wsaBuf.buf, SOCKET_BUF_SIZE);
	CopyMemory(clientContext->SendContext->wsaBuf.buf, msg, strlen(msg) + 1);
	clientContext->SendContext->wsaBuf.len = (ULONG)(strlen(msg) + 1);

	int resultCode = WSASend(clientContext->Socket, &(clientContext->SendContext->wsaBuf), 1,
		&byteSent, 0, &(clientContext->SendContext->overlapped), NULL);

	if (resultCode == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			PrintLog(eLogLevel::Error, "WSASend() - %d", errorCode);
		}
	}
}
void NetworkController::ProceesSend(const SocketContextPointer clientContext)
{
	assert(clientContext != NULL && "clientContext is NULL.");

	PrintLog(eLogLevel::Info, "packet sent to client(%d) / thread id : %d / msg : %s",
		clientContext->Socket, GetCurrentThreadId(), clientContext->SendContext->wsaBuf.buf);

	PostReceive(clientContext);
}

void NetworkController::SendAllClient(const char* msg)
{

	WaitForSingleObject(hMutex, INFINITE);

	set<SocketContextPointer>::iterator socketIter = mSetSocketContext.begin();
	const set<SocketContextPointer>::iterator endIter = mSetSocketContext.end();

	while (socketIter != endIter) {
		SocketContextPointer clientContext = *socketIter;

		PostSend(clientContext, msg);

		socketIter++;
	}

	ReleaseMutex(hMutex);
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

void NetworkController::DeleteSocketContext(const SocketContextPointer contextPointer)
{

	assert(contextPointer != NULL && "contextPointer is NULL.");
	assert(contextPointer->Socket != INVALID_SOCKET && "contextPointer->Socket is INVALID_SOCKET");
	assert(contextPointer->ReceiveContext != NULL && "contextPointer->ReceiveContext is NULL.");
	assert(contextPointer->SendContext != NULL && "contextPointer->SendContext is NULL.");

	closesocket(contextPointer->Socket);

	delete contextPointer->ReceiveContext->wsaBuf.buf;
	delete contextPointer->ReceiveContext;
	delete contextPointer->SendContext->wsaBuf.buf;
	delete contextPointer->SendContext;
	delete contextPointer;
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

	WaitForSingleObject(hMutex, INFINITE);

	mSetSocketContext.insert(clientContext);

	ReleaseMutex(hMutex);
}

void NetworkController::DeleteClientContextInSet(const SocketContextPointer key)
{
	assert(key != NULL && "key is NULL.");

	WaitForSingleObject(hMutex, INFINITE);

	mSetSocketContext.erase(key);

	ReleaseMutex(hMutex);
}

void NetworkController::DeleteAllClientContext()
{
	set<SocketContextPointer>::iterator socketIter = mSetSocketContext.begin();
	const set<SocketContextPointer>::iterator endIter = mSetSocketContext.end();

	WaitForSingleObject(hMutex, INFINITE);

	while (socketIter != endIter) {
		SocketContextPointer clientContext = *socketIter;
		DeleteSocketContext(clientContext);
		socketIter++;
	}

	mSetSocketContext.clear();

	ReleaseMutex(hMutex);
}
