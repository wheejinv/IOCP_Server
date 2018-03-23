#pragma once

#include <set>
#include "Constants.h"
#include "IocpController.h"
#include "Struct.h"

class NetworkController
{
public:
	NetworkController();
	~NetworkController();

	void InitIocpController();

	// socket
	void InitWinSock();
	void CreateListenSocket();
	void Bind(const u_short port = DEFAULT_PORT);
	void StartListening();
	void WaitForAccept();
	void CloseServer();

	// send & receive
	void PostReceive(const SocketContextPointer clientContext);
	void ProceedReceive(const SocketContextPointer clientContext, const DWORD receiveBytes);
	void PostSend(const SocketContextPointer clientContext, const char* msg);
	void ProceesSend(const SocketContextPointer clientContext);
	void SendAllClient(const char* msg);

	void OnDisconnectSocket(const SocketContextPointer clientContext);

	SocketContextPointer CreateSocketContext(const SOCKET clientSocket);
	void DeleteSocketContext(const SocketContextPointer contextPointer);

	// client context map
	void AddClientContextToSet(const SocketContextPointer clientContext);
	int DeleteClientContextInSet(const SocketContextPointer key);
	void DeleteAllClientContext();

private:
	static void __stdcall StartThread(AcceptThreadParam* const params);

	SOCKET mListenSocket;
	IocpController mIocpController;

	set<SocketContextPointer> mSetSocketContext;
	HANDLE mAcceptThread;

	HANDLE hMutex;
};