#include "pch.h"
#include "ProtoBufServerPacketHandler.h"
#include "BufferReader.h"
#include "BufferWriter.h"

void ProtoBufServerPacketHandler::HandlePacket(BYTE* buffer, int32 len)
{
	BufferReader br(buffer, len);

	PacketHeader header;
	br.Peek(&header);

	switch (header.id)
	{
	default:
		break;
	}
}

SendBufferRef ProtoBufServerPacketHandler::MakeSendBuffer(Protocol::S_TEST& pkt)
{
	return _MakeSendBuffer(pkt, S_TEST);
}