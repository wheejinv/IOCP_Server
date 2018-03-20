#include "stdafx.h"
#include "Logger.h"

void PrintLog(const eLogLevel level, const char * const str, ...)
{
#ifndef PRINT_LOG
	return;
#endif

	assert(str != NULL && "str is NULL.");

	va_list argList;
	va_start(argList, str);

	char strBuf[256] = { 0 };
	char buf[512] = { 0 };
	char logStr[16] = { 0 };

	switch (level)
	{
	case Info:
		sprintf_s(logStr, "[Info]");
		break;
	case Error:
		sprintf_s(logStr, "[Error]");
		break;

	case None:
		sprintf_s(logStr, "[Error]");
		break;
	default:
		assert(0 && "invalid log level.");
		break;
	}

	vsprintf_s(strBuf, str, argList);

	va_end(argList);

	sprintf_s(buf, "%s %s", logStr, strBuf);

	PrintConsole(buf);
}

void PrintConsole(const char * const str)
{
	cout << str << endl;
}