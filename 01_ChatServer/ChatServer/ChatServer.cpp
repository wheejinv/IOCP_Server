#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS


#include <iostream>
#include <WinSock2.h>
#include <process.h>
#include <Windows.h>

#define BUF_SIZE 100
#define READ 3
#define WRITE 5
#define MAX_CLNT 256

HANDLE hMutex;
int    clntCnt;
SOCKET clntSocks[MAX_CLNT];


using namespace std;

typedef struct // socket info
{
  SOCKET   hClntSock;
  SOCKADDR clntAdr;
} PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

typedef struct // buffer info
{
  OVERLAPPED overlapped;
  WSABUF     wsaBuf;
  char       buffer[BUF_SIZE];
  int        rwMode; // READ or WRITE
} PER_IO_DATA, *LPPER_IO_DATA;

void            CompressSocketArray(SOCKET sock);
unsigned WINAPI ChatThread(LPVOID CompletionPortIO);
void            ErrorHandling(char *message);

int             main(int argc, char *argv[])
{
  WSADATA wsaData;
  HANDLE  hComPort;
  SYSTEM_INFO   sysInfo;
  LPPER_IO_DATA ioInfo;
  LPPER_HANDLE_DATA handleInfo;

  SOCKET hServSock;
  SOCKADDR_IN servAdr;
  DWORD recvBytes, flags = 0;
  int   i;

  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    ErrorHandling("WSAStartup() error!");
  }

  hMutex = CreateMutex(NULL, FALSE, NULL);

  hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

  GetSystemInfo(&sysInfo);

  for (i = 0; i < sysInfo.dwNumberOfProcessors; i++) {
    _beginthreadex(NULL, 0, ChatThread, (LPVOID)hComPort, 0, NULL);
  }

  hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
  memset(&servAdr, 0, sizeof(servAdr));
  servAdr.sin_family      = AF_INET;
  servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAdr.sin_port        = htons(atoi(argv[1]));

  bind(hServSock, (SOCKADDR *)&servAdr, sizeof(servAdr));
  listen(hServSock, 5);

  while (1)
  {
    SOCKET hClntSock;
    SOCKADDR_IN clntAdr;
    int addrLen = sizeof(clntAdr);

    hClntSock = accept(hServSock, (SOCKADDR *)&clntAdr, &addrLen);

    WaitForSingleObject(hMutex, INFINITE);
    clntSocks[clntCnt++] = hClntSock;
    ReleaseMutex(hMutex);

    handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
    memset(handleInfo, 0, sizeof(PER_HANDLE_DATA));
    handleInfo->hClntSock = hClntSock;
    memcpy(&(handleInfo->clntAdr), &clntAdr, addrLen);

    CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (ULONG_PTR)handleInfo, 0);

    ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
    memset(ioInfo, 0, sizeof(PER_IO_DATA));
    ioInfo->wsaBuf.len = BUF_SIZE;
    ioInfo->wsaBuf.buf = ioInfo->buffer;

    ioInfo->rwMode = READ;

    WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, &recvBytes, &flags,
            &(ioInfo->overlapped), NULL);
  }

  return 0;
}

unsigned WINAPI ChatThread(LPVOID pComPort)
{
  HANDLE hComPort = (HANDLE)pComPort;
  SOCKET sock;
  DWORD  bytesTrans;
  LPPER_HANDLE_DATA handleInfo;
  LPPER_IO_DATA     ioInfo;
  DWORD flags = 0;

  int i;

  LPPER_IO_DATA sendIoData;

  while (1)
  {
    GetQueuedCompletionStatus(hComPort, &bytesTrans, (PULONG_PTR)&handleInfo,
                              (LPOVERLAPPED *)&ioInfo, INFINITE);

    sock = handleInfo->hClntSock;

    if (ioInfo->rwMode == READ) {
      puts("message received!");

      if (bytesTrans == 0) {
        CompressSocketArray(sock);
        closesocket(sock);
        free(handleInfo);
        free(ioInfo);
        continue;
      }

      WaitForSingleObject(hMutex, INFINITE);

      for (i = 0; i < clntCnt; i++) {
        sendIoData = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
        memset(sendIoData, 0, sizeof(PER_IO_DATA));
        sendIoData->wsaBuf.len = BUF_SIZE;
        sendIoData->wsaBuf.buf = sendIoData->buffer;

        sendIoData->rwMode = WRITE;

        strcpy_s(sendIoData->buffer, ioInfo->buffer);

        WSASend(clntSocks[i], &(sendIoData->wsaBuf), 1, NULL, 0,
                &(sendIoData->overlapped), NULL);
      }
      ReleaseMutex(hMutex);


      free(ioInfo);

      // ���� �غ�
      ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
      memset(ioInfo, 0, sizeof(PER_IO_DATA));
      ioInfo->wsaBuf.len = BUF_SIZE;
      ioInfo->wsaBuf.buf = ioInfo->buffer;
      ioInfo->rwMode     = READ;
      WSARecv(sock,
              &(ioInfo->wsaBuf),
              1,
              NULL,
              &flags,
              &(ioInfo->overlapped),
              NULL);
    }
    else {
      puts("message send!");
      free(ioInfo);
    }
  }

  return 0;
}

void CompressSocketArray(SOCKET sock)
{
  int i;

  WaitForSingleObject(hMutex, INFINITE);

  for (i = 0; i < clntCnt; i++) {
    if (sock == clntSocks[i]) {
      while (i++ < clntCnt - 1) clntSocks[i] = clntSocks[i + 1];
      break;
    }
  }
  clntCnt--;
  ReleaseMutex(hMutex);
}

void ErrorHandling(char *message)
{
  cout << message << endl;
  exit(1);
}
