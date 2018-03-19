#include "stdafx.h"
#include "IocpController.h"
#include "Logger.h"
#include "NetworkController.h"
#include "Constants.h"

#define REQUEST_QUIT_THREAD (1)

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
	mIocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (mIocpHandle == NULL) {
		PrintLog(eLogLevel::Error, "IocpController::CreateIocpHandle - %d", GetLastError());
	}
	else {
		PrintLog(eLogLevel::Info, "IocpController::CreateIocpHandle()");
	}
}

void IocpController::CreateThreadPool(NetworkController* const networkController)
{
	mNumThread = THREAD_NUM;
	mThreadList = new HANDLE[mNumThread];

	PrintLog(eLogLevel::Info, "Start CreateThreadPool()");

	for (int i = 0; i < mNumThread; i++) {
		IocpThreadParam* params = new IocpThreadParam;
		params->IocpHandle = mIocpHandle;
		params->NetworkController = networkController;
		mThreadList[i] = (HANDLE)_beginthreadex(NULL, 0, (_beginthreadex_proc_type)IocpThreadMain, params, 0, 0);
	}

	PrintLog(eLogLevel::Info, "Finished CreateThreadPool()");
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

void IocpController::TerminateThreads()
{
	for (int i = 0; i < mNumThread; i++) {
		BOOL result = PostQueuedCompletionStatus(mIocpHandle, 0, REQUEST_QUIT_THREAD, NULL);

		if (result == FALSE) {
			PrintLog(eLogLevel::Error, "PostQueuedCompletionStatus - %d", GetLastError());
		}
	}

	PrintLog(eLogLevel::Info, "IocpController::TerminateThreads()");
}

void __stdcall IocpController::IocpThreadMain(IocpThreadParam* const params)
{
	assert(params != NULL && "parmas is NULL.");
	assert(params->IocpHandle != NULL && "params->IocpHandle is NULL.");

	DWORD transferredBytes = 0;
	SocketContextPointer completionKey = NULL;
	IoContextPointer ioType = NULL;
	NetworkController* networkController = params->NetworkController;
	const DWORD threadRealId = GetCurrentThreadId();


	PrintLog(eLogLevel::Info, "IocpController::IocpThreadMain() - Started, thread(%d)", threadRealId);

	while (true)
	{
		BOOL result = GetQueuedCompletionStatus(params->IocpHandle, &transferredBytes,
			reinterpret_cast<PULONG_PTR>(&completionKey),
			reinterpret_cast<LPOVERLAPPED*>(&ioType), INFINITE);

		if (result == FALSE ) {
			continue;
		}

		if ((int)completionKey == REQUEST_QUIT_THREAD) {
			break;
		}

		SocketContextPointer clientContext = completionKey;

		// client 종료 시
		if (transferredBytes == 0) {
			networkController->DeleteClientContextInSet(clientContext);
			networkController->DeleteSocketContext(clientContext);
			continue;
		}

		// io type에 따른 처리동작 분기
		if (ioType == clientContext->ReceiveContext) {
			networkController->ProceedReceive(clientContext, transferredBytes);
		}
		else if (ioType == clientContext->SendContext) {
			networkController->ProceesSend(clientContext);
		}
		else {
			assert(false && "invalid ioType");
		}
	}

	delete params;

	PrintLog(eLogLevel::Info, "IocpController::IocpThreadMain - Terminated, thread(%d)", threadRealId);
}
