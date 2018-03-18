#pragma once

#include <set>
#include "Constants.h"
#include "IocpController.h"
#include "Struct.h"

class NetworkController
{
public:
	void InitIocpController();

	// socket
	void InitWinSock();
	void CreateListenSocket();
	void Bind(const u_short port);
	void StartListening();
	void WaitForAccept();
	void CloseServer();

	// send & receive
	void PostReceive(const SocketContextPointer clientContext);
	void PostSend(const SocketContextPointer clientContext, const char* msg);
	//void ProceedReceive(const Socket)

	SocketContextPointer CreateSocketContext(const SOCKET clientSocket);


	// client context map
	void AddClientContextToSet(const SocketContextPointer clientContext);
	void DeleteClientContextInSet(const SocketContextPointer key);
	void DeleteAllClientContext();

private:
	static void __stdcall StartThread(AcceptThreadParam* const params);

	SOCKET mListenSocket;
	IocpController mIocpController;

	set<SocketContextPointer> mSetSocketContext;
	HANDLE mAcceptThread;
};