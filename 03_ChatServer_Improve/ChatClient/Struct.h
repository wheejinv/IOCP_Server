#pragma once

#pragma pack(push,1)
typedef struct
{
	int userID;
	char userName[20];
	char chatMsg[124];
} ChatPacket, *ChatPacketPointer;
#pragma pack(pop)