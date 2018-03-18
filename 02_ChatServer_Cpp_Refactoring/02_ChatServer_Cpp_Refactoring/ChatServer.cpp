#include "stdafx.h"
#include "NetworkController.h"

int main()
{

	NetworkController nc;
	nc.InitWinSock();

	nc.CreateListenSocket();


	return 0;
}