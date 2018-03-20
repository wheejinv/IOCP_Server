#include "stdafx.h"
#include "NetworkController.h"

#define KEY_EXIT (27)

int main()
{

	NetworkController nc;
	nc.InitWinSock();
	nc.CreateListenSocket();
	nc.Bind();
	nc.StartListening();
	nc.InitIocpController();
	nc.WaitForAccept();

	while (true) {
		int ch = _getch();

		if (ch == KEY_EXIT) {
			nc.CloseServer();
			break;
		}
	}

	cout << "END" << endl;

	return 0;
}