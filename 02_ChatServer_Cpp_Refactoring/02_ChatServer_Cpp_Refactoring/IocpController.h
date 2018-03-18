// IocpController.h : IOCP 모듈 클래스 및 관련 구조체
#pragma once



class NetworkController;

class IocpController
{
public:
	IocpController();
	~IocpController();

	void CreateIocpHandle();
	void CreateThreadPool(NetworkController* const networkController);
	void Associate(const HANDLE deviceHandle, const ULONG_PTR completionKey);

private:
	HANDLE mIocpHandle;
	HANDLE* mThreadList;
	int mNumThread;
};
