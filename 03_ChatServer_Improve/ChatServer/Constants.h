#pragma once
#include "Struct.h"

// network config
constexpr unsigned int DEFAULT_PORT = 2022;
constexpr unsigned int SOCKET_BUF_SIZE = sizeof(ChatPacket);

// iocp config
constexpr unsigned int THREAD_NUM = 6;