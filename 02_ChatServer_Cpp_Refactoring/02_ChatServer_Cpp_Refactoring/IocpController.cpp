#include "stdafx.h"
#include "IocpController.h"
#include "Logger.h"

IocpController::IocpController()
{
	mIocpHandle = NULL;
	mThreadList = NULL;
	mNumThread = 0;

}

IocpController::~IocpController()
{
	if (mThreadList != NULL) {
		for (int i = 0; i < mNumThread; i++) {
			CloseHandle(mThreadList[i]);
		}

		delete mThreadList;
	}

	mThreadList = NULL;
}

void IocpController::CreateIocpHandle()
{

}

void IocpController::CreateThreadPool(NetworkController* const networkController)
{

}

void IocpController::Associate(const HANDLE deviceHandle, const ULONG_PTR completionKey)
{
	assert(deviceHandle != NULL && "deviceHandle is NULL.");
	assert(completionKey != NULL && "completionKey is NULL.");

	HANDLE returnedHandle = CreateIoCompletionPort(deviceHandle, mIocpHandle, completionKey, 0);

	if (returnedHandle != mIocpHandle) {
		PrintLog(eLogLevel::Error, "IocpController::Associate() - %d", GetLastError());
	}	
}
