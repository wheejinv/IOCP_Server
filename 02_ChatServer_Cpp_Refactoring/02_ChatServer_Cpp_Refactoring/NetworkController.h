#pragma once

class NetworkController
{
public:
	// socket
	void InitWinSock();
	void CreateListenSocket();
	void Bind(const u_short port);
	void StartListening();
	void WaitForAccept();
	void CloseServer();
};