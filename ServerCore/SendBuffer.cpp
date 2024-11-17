#include "pch.h"
#include "SendBuffer.h"

/*-----------------
	SendBuffer
-----------------*/
SendBuffer::SendBuffer(int32 bufferSize)
{
	_buffer.resize(bufferSize);
}

SendBuffer::~SendBuffer()
{
}

void SendBuffer::CopyData(void* data, int32 len)
{
	ASSERT_CRASH(Capacity() >= len);
	::memcpy(_buffer.data(), data, len); // 처음 한 번은 복사해줌
	_writeSize = len;
}
