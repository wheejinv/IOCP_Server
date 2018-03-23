// IocpController.h : IOCP 모듈 클래스 및 관련 구조체
#pragma once
#include "Struct.h"

class NetworkController;

class IocpController
{
public:
	IocpController();
	~IocpController();

	void CreateIocpHandle();
	void CreateThreadPool(NetworkController* const networkController);
	void Associate(const HANDLE deviceHandle, const ULONG_PTR completionKey);
	void TerminateThreads();

private:

	static void __stdcall IocpThreadMain(IocpThreadParam* const params);

	HANDLE mIocpHandle;
	HANDLE* mThreadList;
	int mNumThread;
};
